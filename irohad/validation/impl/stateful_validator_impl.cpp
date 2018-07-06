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

    /**
     * Check, if this tx belongs to any atomic batch
     * @param tx we want to check
     * @return if tx belongs to atomic batch
     */
    static bool txIsFromAtomicBatch(
        const shared_model::interface::Transaction &tx) {
      return tx.batch_meta()
          and tx.batch_meta()->get()->type()
          == shared_model::interface::types::BatchType::ATOMIC;
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
      auto filter = [this,
                     &temporaryWsv,
                     checking_transaction,
                     &transactions_errors_log](auto &tx) {
        if (not batchlyProcessTx(tx)) {
          // if tx is from failed batch, just skip it
          return false;
        }
        return temporaryWsv.apply(tx, checking_transaction)
            .match(
                [](expected::Value<void> &) {
                  processSuccessTx(tx);
                  return true;
                },
                [&transactions_errors_log,
                 &tx](expected::Error<validation::CommandError> &error) {
                  processFailedTx(tx);
                  transactions_errors_log.push_back(
                      std::make_pair(error.error, tx.hash()));
                  return false;
                });
      };

      // TODO: kamilsa IR-1010 20.02.2018 rework validation logic, so that this
      // cast is not needed and stateful validator does not know about the
      // transport
      auto valid_proto_txs =
          proposal.transactions() | boost::adaptors::filtered(filter)
          | boost::adaptors::transformed([](auto &tx) {
              return static_cast<const shared_model::proto::Transaction &>(tx);
            });

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

    bool StatefulValidatorImpl::batchlyProcessTx(
        const shared_model::interface::Transaction &tx,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      if (not txIsFromAtomicBatch(tx)) {
        // it tx is not from atomic batch, we're not interested
        return true;
      }

      if (not current_atomic_batch_) {
        // if this tx is from atomic batch, which is new for us, remember it and
        // create a savepoint in case of future failure
        current_atomic_batch_ = std::make_shared<BatchData>(BatchData{tx});
        temporaryWsv.createSavepoint(current_atomic_batch_->savepoint_name);
        return true;
      }

      if (current_atomic_batch_ and current_atomic_batch_->batch_failed) {
        if (current_atomic_batch_->isLastTxInBatch(tx)) {
          // if this is last tx from failed batch, nullify the batch
          current_atomic_batch_ = nullptr;
        }
        return false;
      }

      return true;
    }

    void StatefulValidatorImpl::processSuccessTx(
        const shared_model::interface::Transaction &tx,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      if (not txIsFromAtomicBatch(tx)) {
        // it tx is not from atomic batch, we're not interested
        return;
      }

      if (current_atomic_batch_->isLastTxInBatch(tx)) {
        // if this is last tx in the batch, it finished successfully; nullify
        // the batch and release a savepoint
        temporaryWsv.releaseSavepoint(current_atomic_batch_->savepoint_name);
        current_atomic_batch_ = nullptr;
      }
    }

    void StatefulValidatorImpl::processFailedTx(
        const shared_model::interface::Transaction &tx,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      if (not txIsFromAtomicBatch(tx)) {
        // it tx is not from atomic batch, we're not interested
        return;
      }

      if (current_atomic_batch_) {
        // this is the first failed tx in this batch; cancel all txs before it
        temporaryWsv.rollbackToSavepoint(current_atomic_batch_->savepoint_name);

        // if this is also a last tx, dismiss current batch; otherwise, just
        // fail it
        if (current_atomic_batch_->isLastTxInBatch(tx)) {
          current_atomic_batch_ = nullptr;
        } else {
          current_atomic_batch_->batch_failed = true;
        }
        return;
      }
    }

  }  // namespace validation
}  // namespace iroha
