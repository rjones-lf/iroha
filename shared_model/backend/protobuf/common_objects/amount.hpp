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

#ifndef IROHA_PROTO_AMOUNT_HPP
#define IROHA_PROTO_AMOUNT_HPP

#include "interfaces/common_objects/amount.hpp"

#include <numeric>

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/util.hpp"
#include "primitive.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class Amount final : public CopyableProto<interface::Amount,
                                              iroha::protocol::Amount,
                                              Amount> {
     public:
      template <typename AmountType>
      explicit Amount(AmountType &&amount)
          : CopyableProto(std::forward<AmountType>(amount)),
            multiprecision_repr_([this] {
              const auto offset = 64u;
              auto times = 3u;
              const auto &value = proto_->value();
              boost::multiprecision::uint256_t result;
              result |= value.first() << offset * times--;
              result |= value.second() << offset * times--;
              result |= value.third() << offset * times--;
              result |= value.fourth() << offset * times--;
              return result;
            }),
            blob_([this] { return makeBlob(*proto_); }) {}

      Amount(const Amount &o) : Amount(o.proto_) {}

      Amount(Amount &&o) noexcept : Amount(std::move(o.proto_)) {}

      const boost::multiprecision::uint256_t &intValue() const override {
        return *multiprecision_repr_;
      }

      interface::types::PrecisionType precision() const override {
        return proto_->precision();
      }

      const BlobType &blob() const override { return *blob_; }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<boost::multiprecision::uint256_t> multiprecision_repr_;

      const Lazy<BlobType> blob_;
    };

  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_AMOUNT_HPP
