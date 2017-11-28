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

#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    /**
     * Simple generic class for handling proto objects
     * @tparam Iface is interface to inherit from
     * @tparam Proto is protobuf containter
     */
    template <typename Iface, typename Proto>
    class CommonProto final : public Iface {
     public:
      template <typename TxResponse>
      explicit CommonProto(TxResponse &&ref)
          : proto_(std::forward<TxResponse>(ref)) {}

      typename Iface::ModelType *copy() const override {
        return new CommonProto(*proto_);
      }

     private:
      detail::ReferenceHolder<Proto> proto_;
    };
  }  // namespace proto
}  // namespace shared_model
