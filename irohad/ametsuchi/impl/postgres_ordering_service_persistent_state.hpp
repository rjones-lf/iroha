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

#ifndef IROHA_POSTGRES_ORDERING_SERVICE_PERSISTENT_STATE_HPP
#define IROHA_POSTGRES_ORDERING_SERVICE_PERSISTENT_STATE_HPP

#include <pqxx/pqxx>
#include "ametsuchi/impl/postgres_wsv_common.hpp"
#include "ametsuchi/ordering_service_persistent_state.hpp"
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
       * Constructor
       * @param postgres_connection postgres connection object
       * @param postgres_transaction postgres transaction object
       */
      explicit PostgresOrderingServicePersistentState(
          std::unique_ptr<pqxx::lazyconnection> postgres_connection,
          std::unique_ptr<pqxx::nontransaction> postgres_transaction);

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
       */
      virtual bool saveProposalHeight(size_t height);

      /**
       * Load proposal height
       */
      virtual boost::optional<size_t> loadProposalHeight() const;

      /**
       * Reset storage state to default
       */
      virtual bool resetState();

     private:
      /**
       * Pg connection with direct transaction management
       */
      std::unique_ptr<pqxx::lazyconnection> postgres_connection_;

      /**
       * Pg transaction
       */
      std::unique_ptr<pqxx::nontransaction> postgres_transaction_;

      logger::Logger log_;

      using ExecuteType = decltype(makeExecuteResult(*postgres_transaction_));
      ExecuteType execute_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_ORDERING_SERVICE_PERSISTENT_STATE_HPP
