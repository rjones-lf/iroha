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

#include "interactive/interactive_status_cli.hpp"

#include <boost/assert.hpp>

#include "client.hpp"
#include "common/byteutils.hpp"

namespace iroha_cli {
  namespace interactive {
    static const std::map<iroha::protocol::TxStatus, std::string>
        userMessageMap = {
            {iroha::protocol::TxStatus::STATELESS_VALIDATION_FAILED,
             "Transaction has not passed stateless validation."},
            {iroha::protocol::TxStatus::STATELESS_VALIDATION_SUCCESS,
             "Transaction has successfully passed stateless validation."},
            {iroha::protocol::TxStatus::STATEFUL_VALIDATION_FAILED,
             "Transaction has not passed stateful validation."},
            {iroha::protocol::TxStatus::STATEFUL_VALIDATION_SUCCESS,
             "Transaction has successfully passed stateful validation."},
            {iroha::protocol::TxStatus::COMMITTED,
             "Transaction was successfully committed."},
            {iroha::protocol::TxStatus::IN_PROGRESS,
             "Transaction is being processed at the moment."},
            {iroha::protocol::TxStatus::NOT_RECEIVED,
             "Transaction was not found in the system."}};

    InteractiveStatusCli::InteractiveStatusCli(
        const std::string &default_peer_ip, int default_port)
        : default_peer_ip_(default_peer_ip), default_port_(default_port) {
      createActionsMenu();
      createResultMenu();
    }

    void InteractiveStatusCli::createActionsMenu() {
      descriptionMap_ = {{GET_TX_INFO, "Get status of transaction"}};
      const auto tx_id = "Requested tx hash";

      requestParamsDescriptions_ = {{GET_TX_INFO, {tx_id}}};
      actionHandlers_ = {{GET_TX_INFO, &InteractiveStatusCli::parseGetHash}};

      menuPoints_ = formMenu(
          actionHandlers_, requestParamsDescriptions_, descriptionMap_);
      addBackOption(menuPoints_);
    }

    void InteractiveStatusCli::createResultMenu() {
      resultHandlers_ = {{SEND_CODE, &InteractiveStatusCli::parseSendToIroha},
                         {SAVE_CODE, &InteractiveStatusCli::parseSaveFile}};
      resultParamsDescriptions_ =
          getCommonParamsMap(default_peer_ip_, default_port_);

      resultPoints_ = formMenu(resultHandlers_,
                               resultParamsDescriptions_,
                               getCommonDescriptionMap());
      addBackOption(resultPoints_);
    }

    void InteractiveStatusCli::run() {
      bool isParsing = true;
      currentContext_ = MAIN;
      printMenu("Choose action: ", menuPoints_);
      while (isParsing) {
        auto line = promptString("> ");
        if (not line){
          // line has terminating symbol
          isParsing = false;
          break;
        }
        switch (currentContext_) {
          case MAIN:
            isParsing = parseAction(line.value());
            break;
          case RESULT:
            isParsing = parseResult(line.value());
            break;
          default:
            // shouldn't get here
            BOOST_ASSERT_MSG(false, "not implemented");
            break;
        }
      }
    }

    bool InteractiveStatusCli::parseAction(std::string &line) {
      if (isBackOption(line)) {
        return false;
      }

      auto res = handleParse<std::string>(
          this, line, actionHandlers_, requestParamsDescriptions_);
      if (not res) {
        // Continue parsing
        return true;
      }

      txHash_ = res.value();
      currentContext_ = RESULT;
      printMenu("Tx hash is saved. Choose what to do:", resultPoints_);
      // Continue parsing
      return true;
    }

    bool InteractiveStatusCli::parseResult(std::string &line) {
      if (isBackOption(line)) {
        // Give up the last query and start a new one
        currentContext_ = MAIN;
        printEnd();
        printMenu("Choose action: ", menuPoints_);
        // Continue parsing
        return true;
      }

      auto res = handleParse<bool>(
          this, line, resultHandlers_, resultParamsDescriptions_);

      return res.value_or(true);
    }

    bool InteractiveStatusCli::parseSendToIroha(ActionParams line) {
      auto address =
          parseIrohaPeerParams(line, default_peer_ip_, default_port_);
      if (not address) {
        return true;
      }

      auto status = iroha::protocol::TxStatus::NOT_RECEIVED;
      if (iroha::hexstringToBytestring(txHash_)) {
        status = CliClient(address.value().first, address.value().second)
                     .getTxStatus(*iroha::hexstringToBytestring(txHash_))
                     .answer.tx_status();
      }

      std::string message;
      try {
        message = userMessageMap.at(status);
      } catch (const std::out_of_range &e) {
        message =
            "A problem detected while retrieving transaction status. Please "
            "try again later.";
      }
      std::cout << message << std::endl;

      printEnd();
      return false;
    }

    bool InteractiveStatusCli::parseSaveFile(ActionParams line) {
      std::cout << "Not implemented yet" << std::endl;
      return true;
    }

    std::string InteractiveStatusCli::parseGetHash(ActionParams params) {
      return params[0];
    }
  }  // namespace interactive
}  // namespace iroha_cli
