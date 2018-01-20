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

#ifndef IROHA_POSTGRES_WSV_COMMAND_HPP
#define IROHA_POSTGRES_WSV_COMMAND_HPP

#include "ametsuchi/wsv_command.hpp"

#include <pqxx/nontransaction>

#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {
    class PostgresWsvCommand : public WsvCommand {
     public:
      explicit PostgresWsvCommand(pqxx::nontransaction &transaction);
      bool insertRole(const std::string &role_name) override;

      bool insertAccountRole(const std::string &account_id,
                             const std::string &role_name) override;
      bool deleteAccountRole(const std::string &account_id,
                             const std::string &role_name) override;

      bool insertRolePermissions(
          const std::string &role_id,
          const std::set<std::string> &permissions) override;

      bool insertAccount(const model::Account &account) override;
      bool updateAccount(const model::Account &account) override;
      bool setAccountKV(const std::string &account_id,
                        const std::string &creator_account_id,
                        const std::string &key,
                        const std::string &val) override;
      bool insertAsset(const model::Asset &asset) override;
      bool upsertAccountAsset(const model::AccountAsset &asset) override;
      bool insertSignatory(const pubkey_t &signatory) override;
      bool insertAccountSignatory(const std::string &account_id,
                                  const pubkey_t &signatory) override;
      bool deleteAccountSignatory(const std::string &account_id,
                                  const pubkey_t &signatory) override;
      bool deleteSignatory(const pubkey_t &signatory) override;
      bool insertPeer(const model::Peer &peer) override;
      bool deletePeer(const model::Peer &peer) override;
      bool insertDomain(const model::Domain &domain) override;
      bool insertAccountGrantablePermission(
          const std::string &permittee_account_id,
          const std::string &account_id,
          const std::string &permission_id) override;

      bool deleteAccountGrantablePermission(
          const std::string &permittee_account_id,
          const std::string &account_id,
          const std::string &permission_id) override;

     private:
      template <typename T>
      friend std::string try_quote(PostgresWsvCommand *,
                                   std::initializer_list<T>) noexcept;
      template <typename T>
      friend std::string try_quote(PostgresWsvCommand *, const T &) noexcept;
      friend std::string try_concat(
          PostgresWsvCommand *, std::initializer_list<const char *>) noexcept;
      friend bool try_exec(PostgresWsvCommand *, const std::string &) noexcept;

      const size_t default_tx_counter = 0;

      pqxx::nontransaction &transaction_;

      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_WSV_COMMAND_HPP
