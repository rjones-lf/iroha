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
#include <boost/range/adaptor/transformed.hpp>
#include <string>
#include "builders/protobuf/proposal.hpp"
#include "common/result.hpp"
#include "validation/utils.hpp"

namespace iroha {
  namespace validation {

    StatefulValidatorImpl::StatefulValidatorImpl() {
      log_ = logger::log("SFV");
    }

    shared_model::interface::types::VerifiedProposalAndErrors
    StatefulValidatorImpl::validate(
        const shared_model::interface::Proposal &proposal,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      using namespace std::string_literals;

      log_->info("transactions in proposal: {}",
                 proposal.transactions().size());
      auto checking_transaction = [this](const auto &tx, auto &queries) {
        return expected::Result<void, std::string>(
            [&]() -> expected::Result<
                         std::shared_ptr<shared_model::interface::Account>,
                         std::string> {
              // Check if tx creator has account
              auto account = queries.getAccount(tx.creatorAccountId());
              if (account) {
                return expected::makeValue(*account);
              }
              return expected::makeError(
                  (boost::format("stateful validator error: could not fetch "
                                 "account with id %s")
                   % tx.creatorAccountId())
                      .str());
            }() |
                [&](const auto &account)
                      -> expected::Result<
                             std::vector<
                                 shared_model::interface::types::PubkeyType>,
                             std::string> {
              // Check if account has signatories and quorum to execute
              // transaction
              if (boost::size(tx.signatures()) >= account->quorum()) {
                auto signatories =
                    queries.getSignatories(tx.creatorAccountId());
                if (signatories) {
                  return expected::makeValue(*signatories);
                }
                return expected::makeError(
                    (boost::format("stateful validator error: could not fetch "
                                   "signatories of "
                                   "account %s")
                     % tx.creatorAccountId())
                        .str());
              }
              return expected::makeError(
                  (boost::format(
                       "stateful validator error: not enough "
                       "signatures on account %s; account's quorum %d, "
                       "transaction's "
                       "signatures amount %d")
                   % tx.creatorAccountId() % account->quorum()
                   % boost::size(tx.signatures()))
                      .str());
            } | [this, &tx](const auto &signatories)
                          -> expected::Result<void, std::string> {
              // Check if signatures in transaction are in account
              // signatory
              if (signaturesSubset(tx.signatures(), signatories)) {
                return {};
              }
              return expected::makeError(
                  formSignaturesErrorMsg(tx.signatures(), signatories));
            });
      };

      // Filter only valid transactions and accumulate errors
      auto transactions_errors_log = std::vector<std::vector<std::string>>{};
      auto filter = [&temporaryWsv,
                     checking_transaction,
                     &transactions_errors_log](auto &tx) {
        return temporaryWsv.apply(tx, checking_transaction)
            .match([](expected::Value<void> &) { return true; },
                   [&transactions_errors_log](
                       expected::Error<std::vector<std::string>> &error) {
                     transactions_errors_log.push_back(error.error);
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
    }  // namespace validation

    std::string StatefulValidatorImpl::formSignaturesErrorMsg(
        const shared_model::interface::types::SignatureRangeType &signatures,
        const std::vector<shared_model::interface::types::PubkeyType>
            &signatories) {
      using namespace std::string_literals;

      auto signatures_string = ""s;
      auto signatories_string = ""s;
      for (const auto &signature : signatures) {
        signatures_string += signature.publicKey().toString() + "\n"s;
      }
      for (const auto &signatory : signatories) {
        signatories_string += signatory.toString() + "\n"s;
      }
      return (boost::format(
                  "stateful validator error: signatures in transaction are not "
                  "account signatories:\n"
                  "signatures' public keys: %s\n"
                  "signatories: %s")
              % signatures_string % signatories_string)
          .str();
    }
  }  // namespace validation
}  // namespace iroha
