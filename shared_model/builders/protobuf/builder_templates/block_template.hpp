/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_PROTO_TEMPLATE_BLOCK_BUILDER_HPP
#define IROHA_PROTO_TEMPLATE_BLOCK_BUILDER_HPP

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/transaction.hpp"
#include "block.pb.h"

#include "builders/protobuf/unsigned_proto.hpp"
#include "interfaces/base/hashable.hpp"
#include "interfaces/base/signable.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/transaction.hpp"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace proto {

    /**
     * Template block builder for creating new types of block builders by
     * means of replacing template parameters
     * @tparam S -- field counter for checking that all required fields are set
     * @tparam SV -- stateless validator called when build method is invoked
     * @tparam BT -- build type of built object returned by build method
     */
    template <int S = 0,
              typename SV = validation::DefaultBlockValidator,
              typename BT = UnsignedWrapper<Block>>
    class TemplateBlockBuilder {
     private:
      template <class T>
      using w = detail::PolymorphicWrapper<T>;

      template <int, typename, typename>
      friend class TemplateBlockBuilder;

      enum RequiredFields {
        Transactions,
        Height,
        PrevHash,
        CreatedTime,
        TOTAL
      };

      template <int s>
      using NextBuilder = TemplateBlockBuilder<S | (1 << s), SV, BT>;

      iroha::protocol::Block block_;
      SV stateless_validator_;

      template <int Sp, typename SVp, typename BTp>
      TemplateBlockBuilder(const TemplateBlockBuilder<Sp, SVp, BTp> &o)
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
      TemplateBlockBuilder(const SV &validator = SV())
          : stateless_validator_(validator){};

      template <class T>
      auto transactions(const T &transactions) const {
        return transform<Transactions>([&](auto &block) {
          for (const auto &tx : transactions) {
            new (block.mutable_payload()->add_transactions())
                iroha::protocol::Transaction(tx.getTransport());
          }
        });
      }

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

        auto tx_number = block_.payload().transactions().size();
        block_.mutable_payload()->set_tx_number(tx_number);

        auto result = Block(iroha::protocol::Block(block_));
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

#endif  // IROHA_PROTO_TEMPLATE_BLOCK_BUILDER_HPP
