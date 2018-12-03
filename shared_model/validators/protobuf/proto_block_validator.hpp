/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_BLOCK_VALIDATOR_HPP
#define IROHA_PROTO_BLOCK_VALIDATOR_HPP

#include "validators/abstract_validator.hpp"

#include "block.pb.h"

namespace shared_model {
  namespace validation {
    class ProtoBlockValidator
        : public AbstractValidator<iroha::protocol::Block> {
     private:
      Answer validateProtoBlock(const iroha::protocol::Block &block) const {
        Answer answer;
        std::string tx_reason_name = "Protobuf Block";
        ReasonsGroupType reason{tx_reason_name, GroupedReasons()};

        // make sure version one_of field of the Block is set
        if (block.block_version_case()
            == iroha::protocol::Block::BLOCK_VERSION_NOT_SET) {
          reason.second.emplace_back("Block version is not set");
          answer.addReason(std::move(reason));
          return answer;
        }

        return answer;
      }

     public:
      Answer validate(const iroha::protocol::Block &block) const override {
        return validateProtoBlock(block);
      }
    };
  };  // namespace validation
}  // namespace shared_model

#endif  // IROHA_PROTO_BLOCK_VALIDATOR_HPP
