/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_PROTO_BUILDER_HELPER_HPP
#define IROHA_PROTO_BUILDER_HELPER_HPP

#include "amount/amount.hpp"
#include "primitive.pb.h"

namespace shared_model {
  namespace proto {
    /**
     * Initialize protobuf Amout by text number representation
     * @param proto is Amount that is being set
     * @param name is string representation of Amount
     */
    static inline void addAmount(iroha::protocol::Amount *proto,
                                 const std::string &name) {
      iroha::Amount::createFromString(name) | [proto](auto &&amount) {
        auto proto_value = proto->mutable_value();
        auto uint64s = amount.to_uint64s();
        proto_value->set_first(uint64s.at(0));
        proto_value->set_second(uint64s.at(1));
        proto_value->set_third(uint64s.at(2));
        proto_value->set_fourth(uint64s.at(3));
        proto->set_precision(amount.getPrecision());
      };
    }
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_BUILDER_HELPER_HPP
