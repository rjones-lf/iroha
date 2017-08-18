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

#ifndef IROHA_CLI_INTERACTIVE_QUERY_CLI_HPP
#define IROHA_CLI_INTERACTIVE_QUERY_CLI_HPP

#include <unordered_map>
#include "interactive/interactive_common_cli.hpp"
#include "model/generators/query_generator.hpp"
#include "model/query.hpp"

namespace iroha_cli {
  namespace interactive {
    class InteractiveQueryCli {
     public:
      /**
       *
       * @param account_name - account_id of query creator
       */
      explicit InteractiveQueryCli(std::string account_name);
      /**
       * Run interactive query command line
       * @return False if no query was formed.
       */
      void run();

     private:
      // Creator account id
      std::string creator_;
      // Query menu points
      std::vector<std::string> menu_points_;
      // Query result points
      std::vector<std::string> result_points_;
      // Current context for query forming
      MenuContext current_context_;
      // Processed query
      std::shared_ptr<iroha::model::Query> query_;

      // ------  Query parsers ---------
      bool parseQuery(std::string line);
      using QueryHandler =
          std::shared_ptr<iroha::model::Query> (InteractiveQueryCli::*)(std::string);
      std::unordered_map<std::string, QueryHandler> query_handlers_;
      // Query handlers
      std::shared_ptr<iroha::model::Query> parseGetAccount(std::string line);
      std::shared_ptr<iroha::model::Query> parseGetAccountAssets(
          std::string line);
      std::shared_ptr<iroha::model::Query> parseGetAccountTransactions(
          std::string line);
      std::shared_ptr<iroha::model::Query> parseGetSignatories(
          std::string line);
      // ------ Result parsers --------
      bool parseResult(std::string line);
      using ResultHandler =
      bool (InteractiveQueryCli::*)(std::string);
      std::unordered_map<std::string, ResultHandler> result_handlers_;
      // Result handlers
      bool parseSendToIroha(std::string line);
      bool parseSaveFile(std::string line);

    };
  }  // namespace interactive
}  // namespace iroha_cli
#endif  // IROHA_CLI_INTERACTIVE_QUERY_CLI_HPP
