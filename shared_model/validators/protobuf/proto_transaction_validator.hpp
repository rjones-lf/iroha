/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_TRANSACTION_VALIDATOR_HPP
#define IROHA_PROTO_TRANSACTION_VALIDATOR_HPP

#include "validators/abstract_validator.hpp"

#include "transaction.pb.h"

namespace shared_model {
  namespace validation {

    class ProtoTransactionValidator
        : public AbstractValidator<iroha::protocol::Transaction> {
     private:
      Answer validateProtoTx(
          const iroha::protocol::Transaction &transaction) const {
        Answer answer;
        std::string tx_reason_name = "Protobuf Transaction";
        ReasonsGroupType reason(tx_reason_name, GroupedReasons());
        for (const auto &command :
             transaction.payload().reduced_payload().commands()) {
          switch (command.command_case()) {
            case iroha::protocol::Command::COMMAND_NOT_SET: {
              reason.second.emplace_back("Undefined command is found");
              answer.addReason(std::move(reason));
              return answer;
            }
            case iroha::protocol::Command::kCreateRole: {
              const auto &cr = command.create_role();
              bool all_permissions_valid = std::all_of(
                  cr.permissions().begin(),
                  cr.permissions().end(),
                  [](const auto &perm) {
                    return iroha::protocol::RolePermission_IsValid(perm);
                  });
              if (not all_permissions_valid) {
                reason.second.emplace_back("Invalid role permission");
                answer.addReason(std::move(reason));
                return answer;
              }
              break;
            }
            case iroha::protocol::Command::kGrantPermission: {
              if (not iroha::protocol::GrantablePermission_IsValid(
                      command.grant_permission().permission())) {
                reason.second.emplace_back("Invalid grantable permission");
                answer.addReason(std::move(reason));
                return answer;
              }
              break;
            }
            case iroha::protocol::Command::kRevokePermission: {
              if (not iroha::protocol::GrantablePermission_IsValid(
                      command.revoke_permission().permission())) {
                reason.second.emplace_back("Invalid grantable permission");
                answer.addReason(std::move(reason));
                return answer;
              }
              break;
            }
            default: { break; }
          }
        }
        return answer;
      }

     public:
      Answer validate(const iroha::protocol::Transaction &tx) const override {
        return validateProtoTx(tx);
      };
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_PROTO_TRANSACTION_VALIDATOR_HPP
