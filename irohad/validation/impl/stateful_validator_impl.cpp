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
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <string>

#include "builders/protobuf/proposal.hpp"
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

    StatefulValidatorImpl::StatefulValidatorImpl() {
      log_ = logger::log("SFV");
    }

    validation::VerifiedProposalAndErrors StatefulValidatorImpl::validate(
        const shared_model::interface::Proposal &proposal,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      log_->info("transactions in proposal: {}",
                 proposal.transactions().size());
      auto checking_transaction = [](const auto &tx, auto &queries) {
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
                auto signatories =
                    queries.getSignatories(tx.creatorAccountId());
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
      };

      // Filter only valid transactions and accumulate errors
      auto transactions_errors_log = validation::TransactionsErrors{};
      auto is_valid_tx = [&temporaryWsv,
                          checking_transaction,
                          &transactions_errors_log](auto &tx) {
        return temporaryWsv.apply(tx, checking_transaction)
            .match([](expected::Value<void> &) { return true; },
                   [&tx, &transactions_errors_log](
                       expected::Error<validation::CommandError> &error) {
                     transactions_errors_log.push_back(
                         std::make_pair(error.error, tx.hash()));
                     return false;
                   });
      };

      // TODO: kamilsa IR-1010 20.02.2018 rework validation logic, so that
      // casts to proto are not needed and stateful validator does not know
      // about the transport
      std::vector<shared_model::proto::Transaction> valid_proto_txs{};
      auto txs_end = std::end(proposal.transactions());
      for (size_t i = 0; i < proposal.transactions().size(); ++i) {
        auto current_tx_it = std::begin(proposal.transactions()) + i;
        auto current_tx = *current_tx_it;
        if (not current_tx_it->batch_meta()
            or current_tx_it->batch_meta()->get()->type()
                != shared_model::interface::types::BatchType::ATOMIC) {
          if (is_valid_tx(*current_tx_it)) {
            // add tx to list of valid_txs
            valid_proto_txs.push_back(
                static_cast<const shared_model::proto::Transaction &>(
                    *current_tx_it));
          }
        } else {
          auto batch_end_hash =
              current_tx_it->batch_meta()->get()->transactionHashes().back();
          auto batch_end_it =
              std::find_if(current_tx_it, txs_end, [&batch_end_hash](auto &tx) {
                return tx.hash() == batch_end_hash;
              });
          if (batch_end_it == txs_end) {
            // for peer review: adequate exception variants?
            throw std::runtime_error("Batch is formed incorrectly");
          }
          if (std::all_of(
                  current_tx_it, batch_end_it, [&is_valid_tx](auto &tx) {
                    return is_valid_tx(tx);
                  })) {
            // batch is successful; add it to the list of valid_txs
            std::transform(
                current_tx_it,
                batch_end_it,
                std::back_inserter(valid_proto_txs),
                [](const auto &tx) {
                  return static_cast<const shared_model::proto::Transaction &>(
                      tx);
                });
          }
          i += std::distance(current_tx_it, batch_end_it);
        }
      }

      auto validated_proposal = shared_model::proto::ProposalBuilder()
                                    .createdTime(proposal.createdTime())
                                    .height(proposal.height())
                                    .transactions(valid_proto_txs)
                                    .createdTime(proposal.createdTime())
                                    .build();

      log_->info("transactions in verified proposal: {}",
                 validated_proposal.transactions().size());
      return std::make_pair(std::make_shared<decltype(validated_proposal)>(
                                validated_proposal.getTransport()),
                            transactions_errors_log);
    }

  }  // namespace validation
}  // namespace iroha
