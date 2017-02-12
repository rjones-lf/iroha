/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

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

#ifndef IROHA_ACCOUNT_HPP
#define IROHA_ACCOUNT_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <tuple>
#include "domain.hpp"

namespace object {

    class Account {

    public:
        std::string ownerPublicKey;
        std::string name;

        std::vector<object::Domain> assets;
/*
        std::vector<
            std::tuple<std::string, std::int64_t>
        > assets;
*/
        explicit Account():
            ownerPublicKey(""),
            name("")
        {}
/*
        explicit Account(
            std::string ownerPublicKey,
            std::string name,
            std::vector<
                std::tuple<std::string, std::int64_t>
            > assets
        ):
            ownerPublicKey(std::move(ownerPublicKey)),
            name(std::move(name)),
            assets(std::move(assets))
        {}

        explicit Account(
            std::string ownerPublicKey,
            std::string name,
            std::tuple<std::string, std::int64_t> asset
        ):
            ownerPublicKey(std::move(ownerPublicKey)),
            name(std::move(name))
        {
            assets.push_back(std::move(asset));
        }
*/
        explicit Account(
            std::string ownerPublicKey,
            std::string name
        ):
            ownerPublicKey(std::move(ownerPublicKey)),
            name(std::move(name))
        {}

        explicit Account(
            std::string ownerPublicKey,
            std::string name,
            std::vector<object::Domain> assets
        ):
            ownerPublicKey(std::move(ownerPublicKey)),
            name(std::move(name)),
            assets(std::move(assets))
        {}


    };

};  // namespace asset

#endif //IROHA_ACCOUNT_HPP
