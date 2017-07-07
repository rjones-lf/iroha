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

#include <validation/chain/validator_stub.hpp>
#include <validation/chain/block_validator_stub.hpp>

namespace iroha {
  namespace validation {

    bool ChainValidatorStub::validate(rxcpp::observable<model::Block>& blocks,
                                      ametsuchi::MutableStorage& storage) {
      auto block_validator = BlockValidatorStub(storage);
      auto apply_block = [](const auto& block, auto& executor, auto& query) {
        for (const auto& tx : block.transactions) {
          for (const auto& command : tx.commands) {
            if (!executor.execute(*command)) {
              return false;
            }
          }
        }
        return true;
      };
      auto result = false;
      blocks.take_while(
          [&result, this, &storage, apply_block, block_validator](auto block) {
            return (result = block_validator.validate(block) &&
                             storage.apply(block, apply_block));
          }).subscribe([](auto block){});
      return result;
    }

    ChainValidatorStub::ChainValidatorStub() {}
  }  // namespace validation
}  // namespace iroha
