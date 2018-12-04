/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validators/protobuf/proto_block_validator.hpp"

namespace shared_model {
  namespace validation {
    Answer ProtoBlockValidator::validate(
        const iroha::protocol::Block &block) const {
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
  }  // namespace validation
}  // namespace shared_model
