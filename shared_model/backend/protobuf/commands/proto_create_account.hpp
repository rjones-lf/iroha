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

#ifndef IROHA_PROTO_CREATE_ACCOUNT_HPP
#define IROHA_PROTO_CREATE_ACCOUNT_HPP

#include "interfaces/commands/create_account.hpp"

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "commands.pb.h"
#include "cryptography/public_key.hpp"

namespace shared_model {
  namespace proto {

    class CreateAccount final : public CopyableProto<interface::CreateAccount,
                                                     iroha::protocol::Command,
                                                     CreateAccount> {
     public:
      template <typename CommandType>
      explicit CreateAccount(CommandType &&command);

      CreateAccount(const CreateAccount &o);

      CreateAccount(CreateAccount &&o) noexcept;

      const interface::types::PubkeyType &pubkey() const override;

      const interface::types::AccountNameType &accountName() const override;

      const interface::types::DomainIdType &domainId() const override;

     private:
      // lazy
      template <typename Value>
      using Lazy = detail::LazyInitializer<Value>;

      const iroha::protocol::CreateAccount &create_account_;

      const Lazy<interface::types::PubkeyType> pubkey_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_CREATE_ACCOUNT_HPP
