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

#include <algorithm>

#include "model/execution/command_executor_factory.hpp"
#include "model/registration/command_registration.hpp"

using namespace iroha::model;

nonstd::optional<std::shared_ptr<CommandExecutorFactory>>
CommandExecutorFactory::create() {
  CommandRegistry registry_;
  decltype(std::declval<CommandExecutorFactory>().executors_) executors;

  executors[typeid(AddAssetQuantity)] =
      std::make_shared<AddAssetQuantityExecutor>();
  executors[typeid(SubtractAssetQuantity)] =
      std::make_shared<SubtractAssetQuantityExecutor>();
  executors[typeid(AddPeer)] = std::make_shared<AddPeerExecutor>();
  executors[typeid(AddSignatory)] = std::make_shared<AddSignatoryExecutor>();
  executors[typeid(CreateAccount)] = std::make_shared<CreateAccountExecutor>();
  executors[typeid(CreateAsset)] = std::make_shared<CreateAssetExecutor>();
  executors[typeid(CreateDomain)] = std::make_shared<CreateDomainExecutor>();
  executors[typeid(RemoveSignatory)] =
      std::make_shared<RemoveSignatoryExecutor>();
  executors[typeid(SetQuorum)] = std::make_shared<SetQuorumExecutor>();
  executors[typeid(TransferAsset)] = std::make_shared<TransferAssetExecutor>();
  executors[typeid(AppendRole)] = std::make_shared<AppendRoleExecutor>();
  executors[typeid(CreateRole)] = std::make_shared<CreateRoleExecutor>();
  executors[typeid(GrantPermission)] = std::make_shared<GrantPermissionExecutor>();
  executors[typeid(RevokePermission)] = std::make_shared<RevokePermissionExecutor>();
  executors[typeid(SetAccountDetail)] = std::make_shared<SetAccountDetailExecutor>();


  auto result =
      std::all_of(registry_.command_handler.types().begin(),
                  registry_.command_handler.types().end(),
                  [executors](auto type) { return executors.count(type) > 0; });
  if (not result) {
    return nonstd::nullopt;
  }

  return std::shared_ptr<CommandExecutorFactory>(
      new CommandExecutorFactory(std::move(executors)));
}

CommandExecutorFactory::CommandExecutorFactory(
    std::unordered_map<std::type_index, std::shared_ptr<CommandExecutor>>
        executors)
    : executors_(std::move(executors)) {}

std::shared_ptr<CommandExecutor> CommandExecutorFactory::getCommandExecutor(
    std::shared_ptr<Command> command) {
  return executors_.at(typeid(*command));
}
