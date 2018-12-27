/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validators/protobuf/proto_block_validator.hpp"

#include <boost/range/adaptors.hpp>

#include "validators/validators_common.hpp"

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
      }

      const auto &rejected_hashes =
          block.block_v1().payload().rejected_transactions_hashes();
      std::for_each(rejected_hashes.begin(),
                    rejected_hashes.end(),
                    [&reason](const auto &hash) {
                      if (not validateHexString(hash)) {
                        reason.second.emplace_back("Rejected hash " + hash
                                                   + " is not in hash format");
                      }
                    });
      if (not validateHexString(block.block_v1().payload().prev_block_hash())) {
        reason.second.emplace_back("Prev block hash has incorrect format");
      }
      if (not reason.second.empty()) {
        answer.addReason(std::move(reason));
      }

      return answer;
    }
  }  // namespace validation
}  // namespace shared_model
