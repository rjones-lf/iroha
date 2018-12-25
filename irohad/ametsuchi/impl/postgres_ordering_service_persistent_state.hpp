/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_POSTGRES_ORDERING_SERVICE_PERSISTENT_STATE_HPP
#define IROHA_POSTGRES_ORDERING_SERVICE_PERSISTENT_STATE_HPP

#include "ametsuchi/ordering_service_persistent_state.hpp"

#include <soci/soci.h>
#include "common/result.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Class implements OrderingServicePersistentState for persistent storage of
     * Ordering Service with PostgreSQL.
     */
    class PostgresOrderingServicePersistentState
        : public OrderingServicePersistentState {
     public:
      /**
       * Create the instance of PostgresOrderingServicePersistentState
       * @param postgres_options postgres connection string
       * @return new instace of PostgresOrderingServicePersistentState
       */
      static expected::Result<
          std::shared_ptr<PostgresOrderingServicePersistentState>,
          std::string>
      create(const std::string &postgres_options);

      /**
       * @param sql - pointer to soci session
       * @param log to print progress
       */
      PostgresOrderingServicePersistentState(
          std::unique_ptr<soci::session> sql,
          logger::Logger log =
              logger::log("PostgresOrderingServicePersistentState"));

      /**
       * Initialize storage.
       * Create tables and fill with initial value.
       */
      bool initStorage();

      /**
       * Drop storage tables.
       */
      bool dropStorgage();

      /**
       * Save proposal height that it can be restored
       * after launch
       * @param height is height of last proposal
       * @return if height has been saved
       */
      bool saveProposalHeight(size_t height) override;

      /**
       * Load proposal height
       * @return proposal height if it was saved, otherwise boost::none
       */
      boost::optional<size_t> loadProposalHeight() const override;

      /**
       * Reset storage state to default
       * @return whether state was reset successfully
       */
      bool resetState() override;

     private:
      std::unique_ptr<soci::session> sql_;

      logger::Logger log_;

      bool execute_(std::string query);
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_ORDERING_SERVICE_PERSISTENT_STATE_HPP
