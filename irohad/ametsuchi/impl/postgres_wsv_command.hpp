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

#include <set>
#include <string>

#include "ametsuchi/impl/postgres_wsv_common.hpp"
#include "logger/logger.hpp"

namespace iroha {

  namespace model {
    struct Asset;
    struct Account;
    struct Domain;
    struct Peer;
    struct AccountAsset;
  }  // namespace model

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

      bool insertAccount(
          const shared_model::interface::Account &account) override;
      bool updateAccount(
          const shared_model::interface::Account &account) override;
      bool setAccountKV(const std::string &account_id,
                        const std::string &creator_account_id,
                        const std::string &key,
                        const std::string &val) override;
      bool insertAsset(const shared_model::interface::Asset &asset) override;
      bool upsertAccountAsset(
          const shared_model::interface::AccountAsset &asset) override;
      bool insertSignatory(
          const shared_model::crypto::PublicKey &signatory) override;
      bool insertAccountSignatory(
          const std::string &account_id,
          const shared_model::crypto::PublicKey &signatory) override;
      bool deleteAccountSignatory(
          const std::string &account_id,
          const shared_model::crypto::PublicKey &signatory) override;
      bool deleteSignatory(
          const shared_model::crypto::PublicKey &signatory) override;
      bool insertPeer(const shared_model::interface::Peer &peer) override;
      bool deletePeer(const shared_model::interface::Peer &peer) override;
      bool insertDomain(const shared_model::interface::Domain &domain) override;
      bool insertAccountGrantablePermission(
          const std::string &permittee_account_id,
          const std::string &account_id,
          const std::string &permission_id) override;

      bool deleteAccountGrantablePermission(
          const std::string &permittee_account_id,
          const std::string &account_id,
          const std::string &permission_id) override;

     private:
      const size_t default_tx_counter = 0;

      pqxx::nontransaction &transaction_;
      logger::Logger log_;

      using ExecuteType = decltype(makeExecute(transaction_, log_));
      ExecuteType execute_;

      // TODO: refactor to return Result when it is introduced IR-775
      bool execute(const std::string &statement) noexcept {
        return static_cast<bool>(execute_(statement));
      }
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_WSV_COMMAND_HPP
