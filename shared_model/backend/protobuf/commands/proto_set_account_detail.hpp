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

#ifndef IROHA_PROTO_SET_ACCOUNT_DETAIL_HPP
#define IROHA_PROTO_SET_ACCOUNT_DETAIL_HPP

#include "interfaces/commands/set_account_detail.hpp"

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "commands.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class SetAccountDetail final
        : public CopyableProto<interface::SetAccountDetail,
                               iroha::protocol::Command,
                               SetAccountDetail> {
     public:
      template <typename CommandType>
      explicit SetAccountDetail(CommandType &&command)
          : CopyableProto(std::forward<CommandType>(command)) {}

      SetAccountDetail(const SetAccountDetail &o)
          : SetAccountDetail(o.proto_) {}

      SetAccountDetail(SetAccountDetail &&o) noexcept
          : SetAccountDetail(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return set_account_detail_.account_id();
      }

      const interface::types::AccountDetailKeyType &key() const override {
        return set_account_detail_.key();
      }

      const interface::types::AccountDetailValueType &value() const override {
        return set_account_detail_.value();
      }

     private:
      const iroha::protocol::SetAccountDetail &set_account_detail_{
          proto_->set_account_detail()};
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_SET_ACCOUNT_DETAIL_HPP
