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

#include "../account_repository.hpp"
#include "common_repository.hpp"
#include <crypto/hash.hpp>
#include <repository/world_state_repository.hpp>
#include <transaction_builder/transaction_builder.hpp>
#include <util/logger.hpp>

namespace common = ::repository::common;

const std::string NameSpaceID = "account repository";
const auto ValuePrefix = common::Prefix("Account::");

namespace repository {
namespace account {

    /********************************************************************************************
     * Add<Account>
     ********************************************************************************************/
    bool add(
        const std::string &publicKey,
        const Api::Account &account
    ){
      return world_state_repository::add("account_" + publicKey, account.SerializeAsString());
    }

    /********************************************************************************************
     * Update<Account>
     ********************************************************************************************/
    bool update(
        const std::string &publicKey,
        const Api::Account &account
    ){
      if(world_state_repository::exists("account_" + publicKey)){
        return world_state_repository::update("account_" + publicKey, account.SerializeAsString());
      }
      return false;
    }

    /********************************************************************************************
     * Remove<Account>
     ********************************************************************************************/
    bool remove(
        const std::string &publicKey
    ){
      if(world_state_repository::exists("account_" + publicKey)){
        return world_state_repository::remove("account_" + publicKey);
      }
      return false;
    }

    Api::Account find(
        const std::string &publicKey
    ){
      Api::Account res;
      if(world_state_repository::exists("account_" + publicKey)){
        res.ParseFromString(world_state_repository::find("account_" + publicKey));
      }
      return res;
    }

    bool exists(
        const std::string &publicKey
    ){
      return world_state_repository::exists("account_" + publicKey);
    }

};
};