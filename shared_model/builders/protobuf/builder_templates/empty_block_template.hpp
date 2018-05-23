/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_EMPTY_BLOCK_TEMPLATE_HPP
#define IROHA_EMPTY_BLOCK_TEMPLATE_HPP

#include "backend/protobuf/empty_block.hpp"
#include "block.pb.h"
#include "builders/protobuf/unsigned_proto.hpp"
#include "interfaces/base/signable.hpp"
#include "interfaces/common_objects/types.hpp"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace proto {

    /**
     * Template empty block builder for creating new types of empty block
     * builders by means of replacing template parameters
     * @tparam S -- field counter for checking that all required fields are set
     * @tparam SV -- stateless validator called when build method is invoked
     * @tparam BT -- build type of built object returned by build method
     */
    template <int S = 0,
              typename SV = validation::DefaultEmptyBlockValidator,
              typename BT = UnsignedWrapper<proto::EmptyBlock>>
    class TemplateEmptyBlockBuilder {
     private:
      template <int, typename, typename>
      friend class TemplateEmptyBlockBuilder;

      enum RequiredFields { Height, PrevHash, CreatedTime, TOTAL };

      template <int s>
      using NextBuilder = TemplateEmptyBlockBuilder<S | (1 << s), SV, BT>;

      iroha::protocol::Block block_;
      SV stateless_validator_;

      template <int Sp, typename SVp, typename BTp>
      TemplateEmptyBlockBuilder(
          const TemplateEmptyBlockBuilder<Sp, SVp, BTp> &o)
          : block_(o.block_), stateless_validator_(o.stateless_validator_) {}

      /**
       * Make transformation on copied content
       * @tparam Transformation - callable type for changing the copy
       * @param t - transform function for proto object
       * @return new builder with updated state
       */
      template <int Fields, typename Transformation>
      auto transform(Transformation t) const {
        NextBuilder<Fields> copy = *this;
        t(copy.block_);
        return copy;
      }

     public:
      TemplateEmptyBlockBuilder(const SV &validator = SV())
          : stateless_validator_(validator){};

      auto height(interface::types::HeightType height) const {
        return transform<Height>(
            [&](auto &block) { block.mutable_payload()->set_height(height); });
      }

      auto prevHash(crypto::Hash hash) const {
        return transform<PrevHash>([&](auto &block) {
          block.mutable_payload()->set_prev_block_hash(
              crypto::toBinaryString(hash));
        });
      }

      auto createdTime(interface::types::TimestampType time) const {
        return transform<CreatedTime>([&](auto &block) {
          block.mutable_payload()->set_created_time(time);
        });
      }

      BT build() {
        static_assert(S == (1 << TOTAL) - 1, "Required fields are not set");

        auto result = EmptyBlock(iroha::protocol::Block(block_));
        auto answer = stateless_validator_.validate(result);

        if (answer.hasErrors()) {
          throw std::invalid_argument(answer.reason());
        }
        return BT(std::move(result));
      }

      static const int total = RequiredFields::TOTAL;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_EMPTY_BLOCK_TEMPLATE_HPP
