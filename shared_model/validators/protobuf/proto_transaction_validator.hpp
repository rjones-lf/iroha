/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_TRANSACTION_VALIDATOR_HPP
#define IROHA_PROTO_TRANSACTION_VALIDATOR_HPP

#include "cryptography/default_hash_provider.hpp"
#include "transaction.pb.h"
#include "validators/abstract_validator.hpp"

namespace shared_model {
  namespace validation {

    class ProtoTransactionValidator
        : public AbstractValidator<iroha::protocol::Transaction> {
     public:
      Answer validate(
          const iroha::protocol::Transaction &transaction) const override {
        Answer answer;
        auto hash = shared_model::crypto::DefaultHashProvider::makeHash(
            proto::makeBlob(tx.payload()));
        std::string tx_reason_name = "Transaction " + hash.toString();
        ReasonsGroupType tx_reason(tx_reason_name, GroupedReasons());

        for (const auto &command :
             transaction.payload().reduced_payload().commands()) {
          switch (command.command_case()) {
            case iroha::protocol::Command::COMMAND_NOT_SET:
              tx_reason.second.emplace_back("Command is not set");
              break;
            default:
              break;
          }
        }

        return answer;
      }
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_PROTO_TRANSACTION_VALIDATOR_HPP
