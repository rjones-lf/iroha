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

#include <ametsuchi/wsv/backend/postgresql.hpp>
#include <iostream>

namespace iroha {

  namespace ametsuchi {

    namespace wsv {

      PostgreSQL::PostgreSQL(const std::string &host, std::size_t port,
                             const std::string &user,
                             const std::string &password) {
        std::stringstream str;
        str << "host=" << host << " port=" << port << " user=" << user
            << " password=" << password;
        connection_ = std::make_unique<pqxx::connection>(str.str());
        read_connection_ = std::make_unique<pqxx::connection>(str.str());

        pqxx::work txn(*connection_);
        txn.exec(init_);
        txn.commit();

        write_ = std::make_unique<pqxx::nontransaction>(*connection_);
        read_ = std::make_unique<pqxx::nontransaction>(*read_connection_);

        auto res = read_->exec(
            "SET SESSION CHARACTERISTICS AS TRANSACTION READ ONLY;");

        start_block();
        start_transaction();
      }

      PostgreSQL::~PostgreSQL() {
        connection_->disconnect();
        read_connection_->disconnect();
      }

      bool PostgreSQL::add_account(std::string account_id, uint8_t quorum,
                                   uint32_t status) {
        try {
          write_->exec(
              "INSERT INTO public.account(\n"
              "            account_id, quorum, status)\n"
              "    VALUES (" +
              write_->quote(account_id) + ", " +
              write_->quote((uint32_t)quorum) +  // TODO fix pqxx
              ", " + write_->quote(status) + ");");
        } catch (std::exception e) {
          std::cerr << e.what() << std::endl;
          return false;
        }
        return true;
      }

      bool PostgreSQL::add_peer(const std::string &account_id,
                                const std::string &address, uint32_t state) {
        try {
          write_->exec(
              "INSERT INTO public.peer(\n"
              "            account_id, address, state)\n"
              "    VALUES (" +
              write_->quote(account_id) + ", " + write_->quote(address) + ", " +
              write_->quote(state) + ");");
        } catch (std::exception e) {
          std::cerr << e.what() << std::endl;
          return false;
        }
        return true;
      }

      bool PostgreSQL::add_signatory(const std::string &account_id,
                                     const std::string &public_key) {
        try {
          write_->exec(
              "INSERT INTO public.signatory(\n"
              "            account_id, public_key)\n"
              "    VALUES (" +
              write_->quote(account_id) + ", " + write_->quote(public_key) +
              ");");
        } catch (std::exception e) {
          std::cerr << e.what() << std::endl;
          return false;
        }
        return true;
      }

      std::vector<std::string> PostgreSQL::get_peers(bool committed) {
        std::unique_ptr<pqxx::nontransaction> &tx = committed ? read_ : write_;
        pqxx::result result;
        try {
          result = tx->exec(
              "SELECT \n"
              "  peer.address\n"
              "FROM \n"
              "  public.peer\n"
              "ORDER BY\n"
              "  peer.peer_id ASC;");
        } catch (std::exception e) {
          std::cerr << e.what() << std::endl;
        }
        std::vector<std::string> peers;
        for (const auto &i : result) {
          peers.push_back(i["address"].as<std::string>());
        }
        return peers;
      }

      void PostgreSQL::commit_transaction() {
        write_->exec("RELEASE SAVEPOINT savepoint_;");
        start_transaction();
      }

      void PostgreSQL::commit_block() {
        write_->exec("COMMIT;");
        start_block();
      }

      void PostgreSQL::rollback_transaction() {
        write_->exec("ROLLBACK TO SAVEPOINT savepoint_;");
        start_transaction();
      }

      void PostgreSQL::rollback_block() {
        write_->exec("ROLLBACK;");
        start_block();
      }

      void PostgreSQL::start_block() { write_->exec("BEGIN;"); }

      void PostgreSQL::start_transaction() {
        write_->exec("SAVEPOINT savepoint_;");
      }
    }  // namespace wsv

  }  // namespace ametsuchi
}  // namespace iroha