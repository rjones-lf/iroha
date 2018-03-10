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

#ifndef IROHA_ADDACCOUNTDETAIL_HPP
#define IROHA_ADDACCOUNTDETAIL_HPP

#include "model/command.hpp"

namespace iroha {
  namespace model {

    struct SetAccountDetail : public Command {
      std::string account_id;
      std::string key;
      std::string value;

      bool operator==(const Command &command) const override;

      SetAccountDetail() {}

      SetAccountDetail(const std::string &account_id,
                       const std::string &key,
                       const std::string &value)
          : account_id(account_id), key(key), value(value) {}
    };

  }  // namespace model
}  // namespace iroha

#endif  // IROHA_ADDACCOUNTDETAIL_HPP
