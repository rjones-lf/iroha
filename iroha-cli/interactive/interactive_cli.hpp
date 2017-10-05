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

#include "crypto/keys_manager_impl.hpp"
#include "interactive/interactive_query_cli.hpp"
#include "interactive_transaction_cli.hpp"

namespace iroha_cli {
  namespace interactive {

    class InteractiveCli {
     public:
      /**
       * @param account_id account id used as transaction or query creator
       */
      InteractiveCli(std::string account_name,
                     uint64_t tx_counter,
                     uint64_t qry_counter,
                     std::string key_path);
      /**
       * Run interactive cli. Print menu and parse command
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
       * Validate user keypair
       */
      bool checkKeys();

      const std::string TX_CODE = "tx";
      const std::string QRY_CODE = "qry";

      /**
       * Account id of creator
       */
      std::string creator_;

      /**
       * To read a keypair
       */
      iroha::KeysManagerImpl keysManager_;

      // -- Query, tx cli --
      InteractiveTransactionCli tx_cli_;
      InteractiveQueryCli query_cli_;

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
