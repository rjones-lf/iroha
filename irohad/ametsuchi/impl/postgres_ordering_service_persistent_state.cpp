/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_ordering_service_persistent_state.hpp"

#include <soci/postgresql/soci-postgresql.h>
#include <boost/format.hpp>
#include <boost/optional.hpp>

namespace iroha {
  namespace ametsuchi {

    bool PostgresOrderingServicePersistentState::execute_(std::string query) {
      try {
        *sql_ << query;
      } catch (std::exception &e) {
        log_->error("Failed to execute query: " + query
                    + ". Reason: " + e.what());
        return false;
      }
      return true;
    }

    expected::Result<std::shared_ptr<PostgresOrderingServicePersistentState>,
                     std::string>
    PostgresOrderingServicePersistentState::create(
        const std::string &postgres_options) {
      std::unique_ptr<soci::session> sql;
      try {
        sql =
            std::make_unique<soci::session>(*soci::factory_postgresql(), postgres_options);

      } catch (std::exception &e) {
        return expected::makeError(
            (boost::format("Connection to PostgreSQL broken: %s") % e.what())
                .str());
      }
      expected::Result<std::shared_ptr<PostgresOrderingServicePersistentState>,
                       std::string>
          storage;
      storage = expected::makeValue(
          std::make_shared<PostgresOrderingServicePersistentState>(
              std::move(sql)));
      return storage;
    }

    PostgresOrderingServicePersistentState::
        PostgresOrderingServicePersistentState(
            std::unique_ptr<soci::session> sql)
        : sql_(std::move(sql)),
          log_(logger::log("PostgresOrderingServicePersistentState")) {}

    bool PostgresOrderingServicePersistentState::initStorage() {
      return execute_(
                 "CREATE TABLE IF NOT EXISTS ordering_service_state "
                 "(proposal_height bigserial)")
          && execute_("INSERT INTO ordering_service_state VALUES (2)");
    }

    bool PostgresOrderingServicePersistentState::dropStorgage() {
      log_->info("Drop storage");
      return execute_("DROP TABLE IF EXISTS ordering_service_state");
    }

    bool PostgresOrderingServicePersistentState::saveProposalHeight(
        size_t height) {
      log_->info("Save proposal_height in ordering_service_state "
                 + std::to_string(height));
      return execute_("DELETE FROM ordering_service_state")
          && execute_("INSERT INTO ordering_service_state VALUES ("
                      + std::to_string(height) + ")");
    }

    boost::optional<size_t>
    PostgresOrderingServicePersistentState::loadProposalHeight() const {
      boost::optional<size_t> height;
      *sql_ << "SELECT * FROM ordering_service_state LIMIT 1",
          soci::into(height);

      if (not height) {
        log_->error(
            "There is no proposal_height in ordering_service_state. "
            "Use default value 2.");
        height = 2;
      }
      return height;
    }

    bool PostgresOrderingServicePersistentState::resetState() {
      return dropStorgage() & initStorage();
    }

  }  // namespace ametsuchi
}  // namespace iroha
