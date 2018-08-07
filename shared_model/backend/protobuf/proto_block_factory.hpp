/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_BLOCK_FACTORY_HPP
#define IROHA_PROTO_BLOCK_FACTORY_HPP

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/empty_block.hpp"
#include "backend/protobuf/transaction.hpp"
#include "block.pb.h"
#include "common/result.hpp"
#include "interfaces/iroha_internal/unsafe_block_factory.hpp"
#include "validators/abstract_validator.hpp"

namespace shared_model {
  namespace proto {
    /**
     * ProtoBlockFactory is used to create proto::Block objects
     */
    class ProtoBlockFactory : public interface::UnsafeBlockFactory {
     public:
      explicit ProtoBlockFactory(
          std::unique_ptr<shared_model::validation::AbstractValidator<
              shared_model::interface::BlockVariant>> validator);

      interface::BlockVariant unsafeCreateBlock(
          interface::types::HeightType height,
          const interface::types::HashType &prev_hash,
          interface::types::TimestampType created_time,
          const interface::types::TransactionsCollectionType &txs) override;

      /**
       * Create block variant with nonempty block
       * @return Error if block is empty, or if it is invalid
       */
      iroha::expected::Result<interface::BlockVariant, std::string> createBlock(
          iroha::protocol::Block block);

     private:
      std::unique_ptr<shared_model::validation::AbstractValidator<
          shared_model::interface::BlockVariant>>
          validator_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_BLOCK_FACTORY_HPP
