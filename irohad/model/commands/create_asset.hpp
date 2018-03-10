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

#ifndef IROHA_CREATE_ASSET_HPP
#define IROHA_CREATE_ASSET_HPP

#include "model/command.hpp"
#include <string>

namespace iroha {
  namespace model {

    /**
     * Create new asset in the system
     */
    struct CreateAsset : public Command {
      /**
       * Asset to create in the system
       */
      std::string asset_name;

      /**
       * Domain id (full name)
       */
      std::string domain_id;

      /**
       * Asset precision
       */
      uint8_t precision;

      bool operator==(const Command &command) const override;

      CreateAsset() {}

      CreateAsset(const std::string &asset_name,
                  const std::string &domain_id,
                  uint8_t precision)
          : asset_name(asset_name),
            domain_id(domain_id),
            precision(precision) {}
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_CREATE_ASSET_HPP
