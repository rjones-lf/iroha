/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "validation/impl/stateful_validator_impl.hpp"

#include <boost/format.hpp>
#include <string>

#include "backend/protobuf/transaction.hpp"
#include "common/result.hpp"
#include "validation/utils.hpp"

namespace iroha {
  namespace validation {

    /**
     * Forms a readable error string from transaction signatures and account
     * signatories
     * @param signatures of the transaction
     * @param signatories of the transaction creator
     * @return well-formed error string
     */
    static std::string formSignaturesErrorMsg(
        const shared_model::interface::types::SignatureRangeType &signatures,
        const std::vector<shared_model::interface::types::PubkeyType>
            &signatories) {
      std::string signatures_string, signatories_string;
      for (const auto &signature : signatures) {
        signatures_string.append(signature.publicKey().toString().append("\n"));
      }
      for (const auto &signatory : signatories) {
        signatories_string.append(signatory.toString().append("\n"));
      }
      return (boost::format(
                  "stateful validator error: signatures in transaction are not "
                  "account signatories:\n"
                  "signatures' public keys: %s\n"
                  "signatories: %s")
              % signatures_string % signatories_string)
          .str();
    }

    /**
     * Initially checks the transaction: its creator, signatures etc
     * @param tx to be checked
     * @param queries to get data from
     * @return result with void, if check is succesfull, command error otherwise
     */
    static expected::Result<void, validation::CommandError>
    initiallyCheckTransaction(const shared_model::interface::Transaction &tx,
                              ametsuchi::WsvQuery &queries) {
      return expected::Result<void, validation::CommandError>(
          [&]() -> expected::Result<
                       std::shared_ptr<shared_model::interface::Account>,
                       validation::CommandError> {
            // Check if tx creator has account
            auto account = queries.getAccount(tx.creatorAccountId());
            if (account) {
              return expected::makeValue(*account);
            }
            return expected::makeError(
                validation::CommandError{"looking up tx creator's account",
                                         (boost::format("could not fetch "
                                                        "account with id %s")
                                          % tx.creatorAccountId())
                                             .str(),
                                         false});
          }() |
              [&](const auto &account)
                    -> expected::Result<
                           std::vector<
                               shared_model::interface::types::PubkeyType>,
                           validation::CommandError> {
            // Check if account has signatories and quorum to execute
            // transaction
            if (boost::size(tx.signatures()) >= account->quorum()) {
              auto signatories = queries.getSignatories(tx.creatorAccountId());
              if (signatories) {
                return expected::makeValue(*signatories);
              }
              return expected::makeError(validation::CommandError{
                  "looking up tx creator's signatories",
                  (boost::format("could not fetch "
                                 "signatories of "
                                 "account %s")
                   % tx.creatorAccountId())
                      .str(),
                  false});
            }
            return expected::makeError(validation::CommandError{
                "comparing number of tx signatures to account's quorum",
                (boost::format(
                     "not enough "
                     "signatures in transaction; account's quorum %d, "
                     "transaction's "
                     "signatures amount %d")
                 % account->quorum() % boost::size(tx.signatures()))
                    .str(),
                false});
          } | [&tx](const auto &signatories)
                        -> expected::Result<void, validation::CommandError> {
            // Check if signatures in transaction are in account
            // signatory
            if (signaturesSubset(tx.signatures(), signatories)) {
              return {};
            }
            return expected::makeError(validation::CommandError{
                "signatures are a subset of signatories",
                formSignaturesErrorMsg(tx.signatures(), signatories),
                false});
          });
    }

    /**
     * Complements initial transaction check with command-by-command check
     * @param temporary_wsv to apply commands on
     * @param transactions_errors_log to write errors to
     * @param tx to be checked
     * @return empty result, if check is succesfull, command error otherwise
     */
    static bool checkTransactions(
        ametsuchi::TemporaryWsv &temporary_wsv,
        validation::TransactionsErrors &transactions_errors_log,
        const shared_model::interface::Transaction &tx) {
      return temporary_wsv.apply(tx, initiallyCheckTransaction)
          .match([](expected::Value<void> &) { return true; },
                 [&tx, &transactions_errors_log](
                     expected::Error<validation::CommandError> &error) {
                   transactions_errors_log.push_back(
                       std::make_pair(error.error, tx.hash()));
                   return false;
                 });
    };

    /**
     * Validate all transactions supplied; includes special rules, such as batch
     * validation etc
     * @param txs to be validated
     * @param temporary_wsv to apply transactions on
     * @param transactions_errors_log to write errors to
     * @param log to write errors to console
     * @return vector of proto transactions, which passed stateful validation
     */
    static std::vector<shared_model::proto::Transaction> validateTransactions(
        const shared_model::interface::types::TransactionsCollectionType &txs,
        ametsuchi::TemporaryWsv &temporary_wsv,
        validation::TransactionsErrors &transactions_errors_log,
        const logger::Logger &log) {
      std::vector<shared_model::proto::Transaction> valid_proto_txs{};
      // TODO: kamilsa IR-1010 20.02.2018 rework validation logic, so that
      // casts to proto are not needed and stateful validator does not know
      // about the transport
      auto txs_begin = std::begin(txs);
      auto txs_end = std::end(txs);
      for (size_t i = 0; i < txs.size(); ++i) {
        auto current_tx_it = txs_begin + i;
        if (not current_tx_it->batchMeta()
            or current_tx_it->batchMeta()->get()->type()
                != shared_model::interface::types::BatchType::ATOMIC) {
          // if transaction does not belong to atomic batch
          if (checkTransactions(
                  temporary_wsv, transactions_errors_log, *current_tx_it)) {
            // and it is valid
            valid_proto_txs.push_back(
                static_cast<const shared_model::proto::Transaction &>(
                    *current_tx_it));
          }
        } else {
          // find the batch end in proposal's transactions
          auto batch_end_hash =
              current_tx_it->batchMeta()->get()->transactionHashes().back();
          auto batch_end_it =
              std::find_if(current_tx_it, txs_end, [&batch_end_hash](auto &tx) {
                return tx.reducedHash() == batch_end_hash;
              });
          if (batch_end_it == txs_end) {
            // exceptional case, such batch should not have passed stateless
            // validation, so fail the whole proposal
            auto batch_error_msg =
                (boost::format("batch is formed incorrectly: could not "
                               "find end of batch; "
                               "first transaction is %s, supposed last "
                               "transaction is %s")
                 % current_tx_it->hash().hex() % batch_end_hash.hex())
                    .str();
            transactions_errors_log.emplace_back(std::make_pair(
                validation::CommandError{
                    "batch stateful validation", batch_error_msg, true},
                current_tx_it->hash()));
            log->error(std::move(batch_error_msg));
            return std::vector<shared_model::proto::Transaction>{};
          }

          // check all batch's transactions for validness
          auto savepoint = temporary_wsv.createSavepoint(
              "batch_" + current_tx_it->hash().hex());
          if (std::all_of(current_tx_it,
                          batch_end_it + 1,
                          [&temporary_wsv, &transactions_errors_log](auto &tx) {
                            return checkTransactions(
                                temporary_wsv, transactions_errors_log, tx);
                          })) {
            // batch is successful; add it to the list of valid_txs and
            // release savepoint
            std::transform(
                current_tx_it,
                batch_end_it + 1,
                std::back_inserter(valid_proto_txs),
                [](const auto &tx) {
                  return static_cast<const shared_model::proto::Transaction &>(
                      tx);
                });
            savepoint->release();
          }

          // move directly to transaction after batch
          i += std::distance(current_tx_it, batch_end_it);
        }
      }
      return valid_proto_txs;
    }

    StatefulValidatorImpl::StatefulValidatorImpl(
        std::shared_ptr<shared_model::interface::UnsafeProposalFactory> factory)
        : factory_(std::move(factory)), log_(logger::log("SFV")) {}

    validation::VerifiedProposalAndErrors StatefulValidatorImpl::validate(
        const shared_model::interface::Proposal &proposal,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      log_->info("transactions in proposal: {}",
                 proposal.transactions().size());

      auto transactions_errors_log = validation::TransactionsErrors{};
      auto valid_proto_txs = validateTransactions(
          proposal.transactions(), temporaryWsv, transactions_errors_log, log_);

      // Since proposal came from ordering gate it was already validated.
      // All transactions has been validated aswell
      // This allows for unsafe construction of proposal
      auto validated_proposal = factory_->unsafeCreateProposal(
          proposal.height(), proposal.createdTime(), valid_proto_txs);

      return std::make_pair(std::move(validated_proposal),
                            transactions_errors_log);
    }
  }  // namespace validation
}  // namespace iroha
