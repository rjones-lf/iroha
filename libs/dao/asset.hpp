/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef IROHA_ASSET_HPP
#define IROHA_ASSET_HPP

#include <common/types.hpp>
#include <string>

namespace iroha {
  namespace dao {

    /**
     * Asset Data Access Object
     */
    struct Asset {
      /**
       * Asset visibility.
       * PUB - everyone from any domain can use this asset
       * PROTECT - account in this domain and it's subdomain can use this asset
       * PRIV - accounts only in this domain can use this asset
       */
      enum Visibility { PUB, PRIV, PROTECT };

      const Visibility visibility;
      /*
       * Asset name
       */
      const std::string name;

      /*
       * Full domain name like sberkek.ru
       */
      const std::string domain_name;

      /*
       * Optional JSON
       */
      const std::string optional_data;

      /*
       * If Asset is open every account according to it's domain (see
       * Visibility)
       * can create a Wallet using this asset.
       */
      const bool is_open;
      /*
       * This filed is used for the number representation.
       */
      const uint32_t precision;
    };
  }
}

#endif  // IROHA_ASSET_HPP
