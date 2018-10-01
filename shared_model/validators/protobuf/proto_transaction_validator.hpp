/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_TRANSACTION_VALIDATOR_HPP
#define IROHA_PROTO_TRANSACTION_VALIDATOR_HPP

#include "validators/transaction_validator.hpp"

namespace shared_model {
  namespace validation {

    template <typename FieldValidator, typename CommandValidator>
    class ProtoTransactionValidator
        : public TransactionValidator<FieldValidator, CommandValidator> {
     private:
      void validateProtoTx(const iroha::protocol::Transaction &transaction,
                           ReasonsGroupType &reason) const {
        for (const auto &command :
             transaction.payload().reduced_payload().commands()) {
          if (command.command_case()
              == iroha::protocol::Command::COMMAND_NOT_SET) {
            reason.second.emplace_back("Undefined command is found");
            break;
          } else if (command.command_case()
                     == iroha::protocol::Command::kCreateRole) {
            const auto &cr = command.create_role();
            bool all_permissions_valid = std::all_of(
                cr.permissions().begin(),
                cr.permissions().end(),
                [](const auto &perm) {
                  return interface::permissions::isValid(
                      static_cast<interface::permissions::Role>(perm));
                });
            if (not all_permissions_valid) {
              reason.second.emplace_back("Undefined command is found");
              break;
            }
          }
        }
      }

     public:
      Answer validate(const interface::Transaction &tx) const override {
        Answer answer;
        std::string tx_reason_name = "Transaction ";
        ReasonsGroupType tx_reason(tx_reason_name, GroupedReasons());

        // validate proto-backend of transaction
        validateProtoTx(
            static_cast<const proto::Transaction &>(tx).getTransport(),
            tx_reason);

        if (not tx_reason.second.empty()) {
          answer.addReason(std::move(tx_reason));
          return answer;
        }
        auto interface_validation =
            TransactionValidator<FieldValidator, CommandValidator>::validate(
                tx);

        if (interface_validation.hasErrors()) {
          tx_reason.second.push_back(interface_validation.reason());
        }
        if (not tx_reason.second.empty()) {
          answer.addReason(std::move(tx_reason));
        }
        return answer;
      };
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_PROTO_TRANSACTION_VALIDATOR_HPP
