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

#include "../domain_repository.hpp"
#include "common_repository.hpp"
#include <crypto/hash.hpp>
#include <repository/world_state_repository.hpp>
#include <util/logger.hpp>

namespace common = ::repository::common;

const std::string NameSpaceID = "domain repository";
const auto ValuePrefix = common::Prefix("Domain::");

namespace repository {
  namespace domain {

    namespace detail {
      std::string createDomainUuid(const std::string &ownerPublicKey) {
        return hash::sha3_256_hex(ownerPublicKey);
      }
    }

/********************************************************************************************
 * Add<Domain>
 ********************************************************************************************/
    std::string add(const std::string &ownerPublicKey, const std::string &name) {

      logger::explore(NameSpaceID)
          << "Add<Domain> ownerPublicKey: " << ownerPublicKey << " name: " << name;

      const auto uuid = detail::createDomainUuid(ownerPublicKey);

      if (not exists(uuid)) {
        const auto strDomain =
            common::stringify<Api::Domain>(txbuilder::createDomain(ownerPublicKey, name), ValuePrefix);
        if (world_state_repository::add(uuid, strDomain)) {
          return uuid;
        }
      }

      return "";
    }

/********************************************************************************************
 * Update<Domain>
 ********************************************************************************************/
    bool update(const std::string &uuid, const std::string &name) {
      if (exists(uuid)) {
        const auto rval = world_state_repository::find(uuid);
        logger::explore(NameSpaceID) << "Update<Domain> uuid: " << uuid << ", name:" << name;
        auto domain = common::parse<Api::Domain>(rval, ValuePrefix);
        *domain.mutable_name() = name;
        const auto strDomain = common::stringify<Api::Domain>(domain, ValuePrefix);
        return world_state_repository::update(uuid, strDomain);
      }
      return false;
    }

/********************************************************************************************
 * Remove<Domain>
 ********************************************************************************************/
    bool remove(const std::string &uuid) {
      if (exists(uuid)) {
        logger::explore(NameSpaceID) << "Remove<Domain> uuid: " << uuid;
        return world_state_repository::remove(uuid);
      }
      return false;
    }

/********************************************************************************************
 * find
 ********************************************************************************************/
    Api::Domain findByUuid(const std::string &uuid) {

      logger::explore(NameSpaceID + "::findByUuid") << "";
      auto strDomain = world_state_repository::find(uuid);
      if (not strDomain.empty()) {
        return common::parse<Api::Domain>(strDomain, ValuePrefix);
      }

      return Api::Domain();
    }

    bool exists(const std::string &uuid) {
      const auto result = world_state_repository::exists(uuid);
      logger::explore(NameSpaceID + "::exists") << (result ? "true" : "false");
      return result;
    }
  }
}
