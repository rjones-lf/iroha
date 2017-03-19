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

#include "../asset_repository.hpp"
#include "common_repository.hpp"
#include <repository/world_state_repository.hpp>
#include <util/logger.hpp>

namespace common = ::repository::common;

const std::string NameSpaceID = "asset repository";
const auto ValuePrefix = common::Prefix("Asset::");

namespace repository {
  namespace asset {

    bool add(
        const std::string &publicKey,
        const std::string &assetName,
        const Api::Asset &asset
    ) {
      return world_state_repository::add("asset_" + publicKey + '_' + assetName, asset.SerializeAsString());
    }

    bool update(
        const std::string &publicKey,
        const std::string &assetName,
        const Api::Asset &asset
    ) {
      if (world_state_repository::exists("asset_" + publicKey + '_' + assetName)) {
        return world_state_repository::update("asset_" + publicKey + '_' + assetName, asset.SerializeAsString());
      }
      return false;
    }

    bool remove(
        const std::string &publicKey,
        const std::string &assetName
    ) {
      if (world_state_repository::exists("asset_" + publicKey + '_' + assetName)) {
        return world_state_repository::remove("asset_" + publicKey + '_' + assetName);
      }
      return false;
    }

    Api::Asset find(
        const std::string &publicKey,
        const std::string &assetName
    ) {
      Api::Asset res;
      logger::info("AssetRepository") << "Find:" << "asset_" + publicKey + '_' + assetName;
      logger::info("AssetRepository") << "Find:" << "pub:" + publicKey;
      logger::info("AssetRepository") << "Find:" << "name:" + assetName;
      if (world_state_repository::exists("asset_" + publicKey + '_' + assetName)) {
        logger::info("AssetRepository") << "Ok exists";

        res.ParseFromString(world_state_repository::find("asset_" + publicKey + '_' + assetName));
      }
      return res;
    }

    bool exists(
        const std::string &publicKey,
        const std::string &assetName
    ) {
      return world_state_repository::exists("asset_" + publicKey + '_' + assetName);
    }

  };
};
