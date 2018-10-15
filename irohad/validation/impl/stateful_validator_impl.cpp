/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validation/impl/stateful_validator_impl.hpp"

#include <string>

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include "common/result.hpp"
#include "interfaces/iroha_internal/batch_meta.hpp"
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
     * @param batch_parser to parse batches from transaction range
     * @return range of transactions, which passed stateful validation
     */
    static auto validateTransactions(
        const shared_model::interface::types::TransactionsCollectionType &txs,
        ametsuchi::TemporaryWsv &temporary_wsv,
        validation::TransactionsErrors &transactions_errors_log,
        const logger::Logger &log,
        const shared_model::interface::TransactionBatchParser &batch_parser) {
      std::vector<bool> validation_results;
      validation_results.reserve(boost::size(txs));

      for (auto batch : batch_parser.parseBatches(txs)) {
        auto validation = [&](auto &tx) {
          return checkTransactions(temporary_wsv, transactions_errors_log, tx);
        };
        if (batch.front().batchMeta()
            and batch.front().batchMeta()->get()->type()
                == shared_model::interface::types::BatchType::ATOMIC) {
          // check all batch's transactions for validness
          auto savepoint = temporary_wsv.createSavepoint(
              "batch_" + batch.front().hash().hex());
          bool validation_result = false;

          if (boost::algorithm::all_of(batch, validation)) {
            // batch is successful; release savepoint
            validation_result = true;
            savepoint->release();
          }

          validation_results.insert(
              validation_results.end(), boost::size(batch), validation_result);
        } else {
          for (const auto &tx : batch) {
            validation_results.push_back(validation(tx));
          }
        }
      }

      return txs | boost::adaptors::indexed()
          | boost::adaptors::filtered(
                 [validation_results =
                      std::move(validation_results)](const auto &el) {
                   return validation_results.at(el.index());
                 })
          | boost::adaptors::transformed(
                 [](const auto &el) -> decltype(auto) { return el.value(); });
    }

    StatefulValidatorImpl::StatefulValidatorImpl(
        std::unique_ptr<shared_model::interface::UnsafeProposalFactory> factory,
        std::shared_ptr<shared_model::interface::TransactionBatchParser>
            batch_parser)
        : factory_(std::move(factory)),
          batch_parser_(std::move(batch_parser)),
          log_(logger::log("SFV")) {}

    validation::VerifiedProposalAndErrors StatefulValidatorImpl::validate(
        const shared_model::interface::Proposal &proposal,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      log_->info("transactions in proposal: {}",
                 proposal.transactions().size());

      auto transactions_errors_log = validation::TransactionsErrors{};
      auto valid_txs = validateTransactions(proposal.transactions(),
                                            temporaryWsv,
                                            transactions_errors_log,
                                            log_,
                                            *batch_parser_);

      // Since proposal came from ordering gate it was already validated.
      // All transactions has been validated as well
      // This allows for unsafe construction of proposal
      auto validated_proposal = factory_->unsafeCreateProposal(
          proposal.height(), proposal.createdTime(), valid_txs);

      log_->info("transactions in verified proposal: {}",
                 validated_proposal->transactions().size());
      return std::make_pair(std::move(validated_proposal),
                            transactions_errors_log);
    }
  }  // namespace validation
}  // namespace iroha
