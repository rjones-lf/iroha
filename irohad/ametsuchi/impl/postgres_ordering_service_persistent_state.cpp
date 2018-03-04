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

#include "ametsuchi/impl/postgres_ordering_service_persistent_state.hpp"
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include "common/types.hpp"

namespace iroha {
  namespace ametsuchi {

    expected::Result<std::shared_ptr<PostgresOrderingServicePersistentState>,
                     std::string>
    PostgresOrderingServicePersistentState::create(
        const std::string &postgres_options) {
      // create connection
      auto postgres_connection =
          std::make_unique<pqxx::lazyconnection>(postgres_options);
      try {
        postgres_connection->activate();
      } catch (const pqxx::broken_connection &e) {
        return expected::makeError(
            (boost::format("Connection to PostgreSQL broken: %s") % e.what())
                .str());
      }

      // create transaction
      auto postgres_transaction = std::make_unique<pqxx::nontransaction>(
          *postgres_connection, "Storage");
      expected::Result<std::shared_ptr<PostgresOrderingServicePersistentState>,
                       std::string>
          storage;
      storage = expected::makeValue(
          std::make_shared<PostgresOrderingServicePersistentState>(
              std::move(postgres_connection), std::move(postgres_transaction)));
      return storage;
    }

    PostgresOrderingServicePersistentState::
        PostgresOrderingServicePersistentState(
            std::unique_ptr<pqxx::lazyconnection> postgres_connection,
            std::unique_ptr<pqxx::nontransaction> postgres_transaction)
        : postgres_connection_(std::move(postgres_connection)),
          postgres_transaction_(std::move(postgres_transaction)),
          log_(logger::log("PostgresOrderingServicePersistentState")),
          execute_{ametsuchi::makeExecuteResult(*postgres_transaction_)} {}

    bool PostgresOrderingServicePersistentState::initStorage() {
      return execute_(
                 "CREATE TABLE IF NOT EXISTS ordering_service_state (\n"
                 "    proposal_height bigserial\n"
                 ");\n"
                 "INSERT INTO ordering_service_state\n"
                 "VALUES (2); -- expected height (1 is genesis)")
          .match([](expected::Value<pqxx::result> v) -> bool { return true; },
                 [&](expected::Error<std::string> e) -> bool {
                   log_->error(e.error);
                   return false;
                 });
    }

    bool PostgresOrderingServicePersistentState::dropStorgage() {
      log_->info("Drop storage");
      return execute_("DROP TABLE IF EXISTS ordering_service_state;")
          .match([](expected::Value<pqxx::result> v) -> bool { return true; },
                 [&](expected::Error<std::string> e) -> bool {
                   log_->error(e.error);
                   return false;
                 });
    }

    bool PostgresOrderingServicePersistentState::saveProposalHeight(
        size_t height) {
      log_->info("Save proposal_height in ordering_service_state "
                 + std::to_string(height));
      return execute_(
                 "DELETE FROM ordering_service_state;\n"
                 "INSERT INTO ordering_service_state "
                 "VALUES ("
                 + postgres_transaction_->quote(height) + ");")
          .match([](expected::Value<pqxx::result> v) -> bool { return true; },
                 [&](expected::Error<std::string> e) -> bool {
                   log_->error(e.error);
                   return false;
                 });
    }

    boost::optional<size_t>
    PostgresOrderingServicePersistentState::loadProposalHeight() const {
      boost::optional<size_t> height;
      execute_("SELECT * FROM ordering_service_state;")
          .match(
              [&](expected::Value<pqxx::result> result) {
                if (result.value.empty()) {
                  log_->error(
                      "There is no proposal_height in ordering_service_state. "
                      "Use default value 2.");
                  height = 2;
                } else {
                  auto row = result.value.at(0);
                  height = row.at("proposal_height").as<size_t>();
                  log_->info("Load proposal_height in ordering_service_state "
                             + std::to_string(height.value()));
                }
              },
              [&](expected::Error<std::string> e) { log_->error(e.error); });
      return height;
    }

    bool PostgresOrderingServicePersistentState::resetState() {
      return dropStorgage() & initStorage();
    }

  }  // namespace ametsuchi
}  // namespace iroha
