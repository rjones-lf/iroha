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

#ifndef IROHA_PROTO_AMOUNT_BUILDER_HPP
#define IROHA_PROTO_AMOUNT_BUILDER_HPP

#include "backend/protobuf/common_objects/amount.hpp"
#include "primitive.pb.h"

namespace shared_model {
  namespace proto {

    /**
     * AmountBuilder is used to construct Amount proto objects with initialized
     * protobuf implementation
     */
    class AmountBuilder {
     public:
      shared_model::proto::Amount build() {
        return shared_model::proto::Amount(iroha::protocol::Amount(amount_));
      }

      AmountBuilder intValue(const boost::multiprecision::uint256_t &value) {
        AmountBuilder copy(*this);
        convertToProtoAmount(*copy.amount_.mutable_value(), value);
        return copy;
      }

      AmountBuilder precision(
          const interface::types::PrecisionType &precision) {
        AmountBuilder copy(*this);
        copy.amount_.set_precision(precision);
        return copy;
      }

     private:
      iroha::protocol::Amount amount_;
    };
  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_AMOUNT_BUILDER_HPP
