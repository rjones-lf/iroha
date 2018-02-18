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

#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/public_key.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"

namespace framework {
  static inline auto createUser(const std::string &user,
                                const shared_model::crypto::PublicKey &key) {
    return shared_model::proto::TransactionBuilder()
        .createAccount(
            user,
            integration_framework::IntegrationTestFramework::kDefaultDomain,
            key)
        .txCounter(1)
        .creatorAccountId(
            integration_framework::IntegrationTestFramework::kAdminId)
        .createdTime(iroha::time::now());
  }

  static inline auto createUserWithPerms(
      const std::string &user,
      const std::string &user_id,
      const shared_model::crypto::PublicKey &key,
      const std::string role_id,
      std::vector<std::string> perms) {
    return createUser(user, key)
        .detachRole(
            user_id,
            integration_framework::IntegrationTestFramework::kDefaultRole)
        .createRole(role_id, perms)
        .appendRole(user_id, role_id);
  }
}  // namespace framework
