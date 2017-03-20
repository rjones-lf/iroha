/**
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
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
#ifndef IROHA_TEST_UTILS_HPP
#define IROHA_TEST_UTILS_HPP

#include <unordered_map>
#include <string>

#include <infra/protobuf/api.grpc.pb.h>

Api::Account makeAccount(const std::string& publicKey, const std::string& name,
                         const std::initializer_list<std::string> assets) {
    Api::Account account;
    account.set_name(name);
    account.set_publickey(publicKey);
    for (const auto ac : assets) {
        account.add_assets(ac);
    }
    return account;
}

Api::Asset makeAsset(
        const std::string& name,
        const std::unordered_map<std::string, Api::BaseObject> prop) {
    Api::Asset asset;
    asset.set_name(name);
    for (const auto p : prop) {
        (*asset.mutable_value())[p.first] = p.second;
    }
    return asset;
}

void removeData(const std::string& publicKey,
                const std::string& assetName1 = "naocoin",
                const std::string& assetName2 = "kayanocoin") {
    repository::account::remove(publicKey);
    Api::Account checkAccount = repository::account::find(publicKey);
    ASSERT_TRUE(checkAccount.name().empty());
    repository::asset::remove(publicKey, assetName1);
    repository::asset::remove(publicKey, assetName2);
}

#endif //IROHA_TEST_UTILS_HPP
