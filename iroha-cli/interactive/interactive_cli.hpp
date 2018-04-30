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

#ifndef IROHA_CLI_INTERACTIVE_CLI_HPP
#define IROHA_CLI_INTERACTIVE_CLI_HPP

#include "interactive/interactive_query_cli.hpp"
#include "interactive/interactive_status_cli.hpp"
#include "interactive/interactive_transaction_cli.hpp"

namespace iroha_cli {
  namespace interactive {

    class InteractiveCli {
     public:
      /**
       * Interactive command line client
       * @param account_name registered in Iroha network
       * @param default_peer_ip default peer ip to send transactions/query
       * @param default_port default port of peer's Iroha Torii
       * @param qry_counter synchronized nonce for sending queries
       * @param provider crypto provider to make signatures
       */
      InteractiveCli(
          const std::string &account_name,
          const std::string &default_peer_ip,
          int default_port,
          uint64_t qry_counter,
          const std::shared_ptr<iroha::model::ModelCryptoProvider> &provider);
      /**
       * Run interactive cli. Print menu and parse commands
       */
      void run();

     private:
      /**
       * Create main menu and assign parser for commands
       */
      void assign_main_handlers();
      /**
       * Parse main menu commands
       * @param line, command to parse
       */
      void parseMain(std::string line);

      /**
       * Start new query
       */
      void startQuery();

      /**
       * Start new transaction
       */
      void startTx();

      /**
       * Start new request about tx status
       */
      void startTxStatusRequest();

      const std::string TX_CODE = "tx";
      const std::string QRY_CODE = "qry";
      const std::string ST_CODE = "st";

      /**
       * Account id of creator
       */
      std::string creator_;

      // -- Query, tx cli --
      InteractiveTransactionCli tx_cli_;
      InteractiveQueryCli query_cli_;
      InteractiveStatusCli statusCli_;

      /**
       * Main menu points
       */
      MenuPoints menu_points_;

      using MainHandler = void (InteractiveCli::*)();
      std::unordered_map<std::string, MainHandler> main_handler_map_;
    };

  }  // namespace interactive
}  // namespace iroha_cli

#endif  // IROHA_CLI_INTERACTIVE_CLI_HPP
