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

#ifndef IROHA_INTERACTIVE_STATUS_CLI_HPP
#define IROHA_INTERACTIVE_STATUS_CLI_HPP

#include <endpoint.pb.h>
#include <string>

#include "interactive/interactive_common_cli.hpp"

namespace iroha_cli {
  namespace interactive {
    /**
     * A special class for retrieving transaction status.
     * It's not a transaction and not a query so it should be
     * processed separately.
     */
    class InteractiveStatusCli {
     public:
      InteractiveStatusCli(const std::string &default_peer_ip,
                           int default_port);
      void run();

     private:
      using ActionName = std::string;
      using ActionParams = std::vector<std::string>;
      using ActionHandler = std::string (InteractiveStatusCli::*)(ActionParams);
      using ResultHandler = bool (InteractiveStatusCli::*)(ActionParams);
      std::unordered_map<ActionName, ActionHandler> actionHandlers_;

      bool parseAction(std::string &line);
      bool parseResult(std::string &line);

      bool parseSendToIroha(ActionParams line);
      bool parseSaveFile(ActionParams line);

      void createActionsMenu();
      void createResultMenu();

      std::string parseGetHash(ActionParams params);

      const std::string GET_TX_INFO = "get_tx_info";

      std::string default_peer_ip_;
      int default_port_;

      std::unordered_map<ActionName, ResultHandler> resultHandlers_;
      ParamsMap resultParamsDescriptions_;

      DescriptionMap descriptionMap_;
      ParamsMap requestParamsDescriptions_;
      MenuPoints menuPoints_;
      MenuPoints resultPoints_;

      MenuContext currentContext_;
      std::string txHash_;
    };
  }  // namespace interactive
}  // namespace iroha_cli

#endif  // IROHA_INTERACTIVE_STATUS_CLI_HPP
