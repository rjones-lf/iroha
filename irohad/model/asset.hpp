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

#ifndef IROHA_ASSET_HPP
#define IROHA_ASSET_HPP

#include <common/types.hpp>
#include <string>

namespace iroha {
  namespace model {

    /**
     * Asset Data Model
     */
    struct Asset {
      Asset() = default;

      Asset(std::string asset_id, std::string domain_id, uint8_t precision)
          : asset_id(asset_id), domain_id(domain_id), precision(precision) {}
      /**
       * Asset unique identifier
       */
      std::string asset_id;

      std::string domain_id;

      /**
       * Precision of asset
       */
      uint8_t precision;
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_ASSET_HPP
