/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_PROPOSAL_FACTORY_HPP
#define IROHA_PROTO_PROPOSAL_FACTORY_HPP

#include "interfaces/iroha_internal/unsafe_block_factory.hpp"
#include "block.pb.h"
#include "backend/protobuf/transaction.hpp"
#include "backend/protobuf/block.hpp"
#include "backend/protobuf/empty_block.hpp"

namespace shared_model {
  namespace proto {
    class ProtoBlockFactory : public interface::UnsafeBlockFactory {
     public:
      interface::BlockVariant unsafeCreateBlock(
          interface::types::HeightType height,
          const interface::types::HashType &prev_hash,
          interface::types::TimestampType created_time,
          const interface::types::TransactionsCollectionType &txs) override {
        iroha::protocol::Block block;
        auto *block_payload = block.mutable_payload();
        block_payload->set_height(height);
        block_payload->set_prev_block_hash(crypto::toBinaryString(prev_hash));
        block_payload->set_created_time(created_time);

        if (not txs.empty()) {
          std::for_each(
              std::begin(txs), std::end(txs), [&block_payload](const auto &tx) {
                auto *transaction = block_payload->add_transactions();
                (*transaction) =
                    static_cast<const Transaction &>(tx).getTransport();
              });
          return std::make_shared<shared_model::proto::Block>(std::move(block));
        }

        return std::make_shared<shared_model::proto::EmptyBlock>(
            std::move(block));
      }
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_PROPOSAL_FACTORY_HPP
