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

#ifndef IROHA_JSON_COMMAND_FACTORY_HPP
#define IROHA_JSON_COMMAND_FACTORY_HPP

#include <memory>
#include <typeindex>
#include <unordered_map>
#include "model/command.hpp"
#include "model/common.hpp"
#include "model/converters/json_common.hpp"

namespace iroha {
  namespace model {
    namespace converters {
      class JsonCommandFactory {
       public:
        JsonCommandFactory();

        // AddAssetQuantity
        rapidjson::Document serializeAddAssetQuantity(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeAddAssetQuantity(
            const rapidjson::Value &document);

        // SubtractAssetQuantity
        rapidjson::Document serializeSubtractAssetQuantity(
          std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeSubtractAssetQuantity(
          const rapidjson::Value &document);

        // AddPeer
        rapidjson::Document serializeAddPeer(std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeAddPeer(
            const rapidjson::Value &document);

        // AddSignatory
        rapidjson::Document serializeAddSignatory(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeAddSignatory(
            const rapidjson::Value &document);

        // CreateAccount
        rapidjson::Document serializeCreateAccount(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeCreateAccount(
            const rapidjson::Value &document);

        // SetAccountAsset
        rapidjson::Document serializeSetAccountDetail(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeSetAccountDetail(
            const rapidjson::Value &document);

        // CreateAsset
        rapidjson::Document serializeCreateAsset(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeCreateAsset(
            const rapidjson::Value &document);

        // CreateDomain
        rapidjson::Document serializeCreateDomain(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeCreateDomain(
            const rapidjson::Value &document);

        // RemoveSignatory
        rapidjson::Document serializeRemoveSignatory(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeRemoveSignatory(
            const rapidjson::Value &document);

        // SetQuorum
        rapidjson::Document serializeSetQuorum(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeSetQuorum(
            const rapidjson::Value &document);

        // TransferAsset
        rapidjson::Document serializeTransferAsset(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeTransferAsset(
            const rapidjson::Value &document);

        // AppendRole
        rapidjson::Document serializeAppendRole(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeAppendRole(
            const rapidjson::Value &document);

        // CreateRole
        rapidjson::Document serializeCreateRole(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeCreateRole(
            const rapidjson::Value &document);

        // GrantPermission
        rapidjson::Document serializeGrantPermission(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeGrantPermission(
            const rapidjson::Value &document);

        // RevokePermission
        rapidjson::Document serializeRevokePermission(
            std::shared_ptr<Command> command);
        optional_ptr<Command> deserializeRevokePermission(
            const rapidjson::Value &document);

        // Abstract
        rapidjson::Document serializeAbstractCommand(
            std::shared_ptr<Command> command);
        optional_ptr<model::Command> deserializeAbstractCommand(
            const rapidjson::Value &document);

       private:
        Convert<std::shared_ptr<Command>> toCommand;

        using Serializer = rapidjson::Document (JsonCommandFactory::*)(
            std::shared_ptr<Command>);
        using Deserializer = optional_ptr<Command> (JsonCommandFactory::*)(
            const rapidjson::Value &);

        std::unordered_map<std::type_index, Serializer> serializers_;
        std::unordered_map<std::string, Deserializer> deserializers_;
      };

    }  // namespace converters
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_JSON_COMMAND_FACTORY_HPP
