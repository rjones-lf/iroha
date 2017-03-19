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

#include "../simple_asset_repository.hpp"
#include <crypto/hash.hpp>
#include <repository/world_state_repository.hpp>
#include <util/logger.hpp>

const std::string NameSpaceID = "simple asset repository";

namespace repository {
  namespace simple_asset {

    namespace detail {
/********************************************************************************************
 * stringify / parse
 ********************************************************************************************/
      std::string stringifySimpleAsset(const Api::SimpleAsset &obj) {
        std::string ret;
        obj.SerializeToString(&ret);
        return ret;
      }

      Api::SimpleAsset parseSimpleAsset(const std::string &str) {
        Api::SimpleAsset ret;
        ret.ParseFromString(str);
        return ret;
      }

      std::string createSimpleAssetUuid(const std::string &domain,
                                        const std::string &name) {
        return hash::sha3_256_hex("simple_asset@" + domain + "@" + name);
      }
    }

/********************************************************************************************
 * Add<SimpleAsset>
 ********************************************************************************************/
    std::string add(const std::string &domain, const std::string &name,
                    const Api::BaseObject &value,
                    const std::string &smartContractName) {

      logger::explore(NameSpaceID) << "Add<SimpleAsset> domainId: " << domain
                                   << " assetName: " << name
                                   << " assetValue: " << txbuilder::stringify(value)
                                   << " smartContractName: " << smartContractName;

      const auto uuid = detail::createSimpleAssetUuid(domain, name);

      if (not exists(uuid)) {
        const auto strAsset = detail::stringifySimpleAsset(
            txbuilder::createSimpleAsset(domain, name, value, smartContractName));
        if (world_state_repository::add(uuid, strAsset)) {
          return uuid;
        }
      }

      return "";
    }

/********************************************************************************************
 * Update<SimpleAsset>
 ********************************************************************************************/
    bool update(const std::string &uuid, const Api::BaseObject &value) {
      if (exists(uuid)) {
        const auto rval = world_state_repository::find(uuid);
        logger::explore(NameSpaceID) << "Update<SimpleAsset> uuid: " << uuid << ", "
                                     << "value: " << txbuilder::stringify(value);
        auto simpleAsset = detail::parseSimpleAsset(rval);
        *simpleAsset.mutable_value() = value;
        const auto strSimpleAsset = detail::stringifySimpleAsset(simpleAsset);
        return world_state_repository::update(uuid, strSimpleAsset);
      }
      return false;
    }

/********************************************************************************************
 * Remove<SimpleAsset>
 ********************************************************************************************/
    bool remove(const std::string &uuid) {
      if (exists(uuid)) {
        logger::explore(NameSpaceID) << "Remove<SimpleAsset> uuid: " << uuid;
        return world_state_repository::remove(uuid);
      }
      return false;
    }

/********************************************************************************************
 * find
 ********************************************************************************************/
    Api::SimpleAsset findByUuid(const std::string &uuid) {

      logger::explore(NameSpaceID + "::findByUuid") << "";
      auto strSimpleAsset = world_state_repository::find(uuid);
      if (not strSimpleAsset.empty()) {
        return detail::parseSimpleAsset(strSimpleAsset);
      }

      return Api::SimpleAsset();
    }

    Api::SimpleAsset findByUuidOrElse(const std::string &uuid,
                                      const Api::SimpleAsset &defaultValue) {
      throw "simple asset repo :: findByUuidOrElse() is not implemented yet.";
    }

    bool exists(const std::string &uuid) {
      const auto result = world_state_repository::exists(uuid);
      logger::explore(NameSpaceID + "::exists")
          << (result ? "true" : "false");
      return result;
    }
  }
}
