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
#include <gtest/gtest.h>
#include <memory>
#include <unordered_map>
#include <service/executor.hpp>
#include <repository/domain/asset_repository.hpp>
#include <repository/domain/account_repository.hpp>
#include <infra/protobuf/api.grpc.pb.h>

Api::Account makeAccount(
        const std::string& publicKey,
        const std::string& name,
        const std::initializer_list<std::string> assets
){
    Api::Account account;
    account.set_name(name);
    account.set_publickey(publicKey);
    for(const auto ac: assets){
        account.add_assets(ac);
    }
    return account;
}

Api::Asset makeAsset(
        const std::string& name,
        const std::unordered_map<std::string,Api::BaseObject> prop
){
    Api::Asset asset;
    asset.set_name(name);
    for(const auto p: prop){
        (*asset.mutable_value())[p.first] = p.second;
    }
    return asset;
}

void removeData(
        const std::string& publicKey,
        const std::string& assetName1 = "naocoin",
        const std::string& assetName2 = "kayanocoin"
){
    repository::account::remove(publicKey);
    Api::Account checkAccount = repository::account::find(publicKey);
    IROHA_ASSERT_TRUE(checkAccount.name().empty());
    repository::asset::remove(publicKey, assetName1);
    repository::asset::remove(publicKey, assetName2);
}

TEST(ScenarioTest, TransferWithTax) {
    const auto name1 = "mizuki";
    const auto name2 = "iori";
    const auto name3 = "maruuchi";

    const auto assetName1 = "naocoin";
    const auto assetName2 = "kayanocoin";

    //const auto privateKey = "iOHEkcoNgABDt9lz+ndjlLl5dSBkutleGerCSOGoP0nF3vArKBO7oyIxvoDSH+RGkcHRgQYgpRhRq3WSy8MYYA==";
    const auto publicKey1 = "mXVNEpTcUiMUDZlAlMhmubbJXNqvwNlpSVXcFdu3qt4=";
    const auto publicKey2 = "p8v1bQ76tHmD3fZrYDWBvzn+aJspfaBw8OwNg+Cx8AI=";
    const auto publicKey3 = "q3W9e3fc65WyQXQDSUxRd1dXL1x8/4G93IKDBmXV54w=";

    {
        const auto type = "add";
        Api::Transaction tx;
        tx.set_senderpubkey(publicKey1);
        tx.set_type(type);
        tx.mutable_account()->CopyFrom(makeAccount(
                publicKey1, name1, {assetName1}
        ));
        executor::execute(tx);
        Api::Account account = repository::account::find(publicKey1);

        Api::BaseObject bobj;
        bobj.set_valueint(100);
        std::unordered_map<std::string,Api::BaseObject> prop;
        prop["value"] = bobj;

        tx.Clear();
        tx.set_senderpubkey(publicKey1);
        tx.set_type(type);
        tx.mutable_asset()->CopyFrom(makeAsset(
                assetName1, prop
        ));
        executor::execute(tx);
    }
    {
        const auto type = "add";
        Api::Transaction tx;
        tx.set_senderpubkey(publicKey2);
        tx.set_type(type);
        tx.mutable_account()->CopyFrom(makeAccount(
                publicKey2, name2, {assetName1}
        ));
        executor::execute(tx);

        Api::BaseObject bobj;
        bobj.set_valueint(100);
        std::unordered_map<std::string,Api::BaseObject> prop;
        prop["value"] = bobj;

        tx.Clear();
        tx.set_senderpubkey(publicKey2);
        tx.set_type(type);
        tx.mutable_asset()->CopyFrom(makeAsset(
            assetName1, prop
        ));
        executor::execute(tx);
    }
    {
        const auto type = "add";
        Api::Transaction tx;
        tx.set_senderpubkey(publicKey3);
        tx.set_type(type);
        tx.mutable_account()->CopyFrom(makeAccount(
            publicKey3, name3, {assetName1}
        ));
        executor::execute(tx);

        Api::BaseObject bobj;
        bobj.set_valueint(5);
        std::unordered_map<std::string,Api::BaseObject> prop;
        prop["value"] = bobj;

        tx.Clear();
        tx.set_senderpubkey(publicKey3);
        tx.set_type(type);
        tx.mutable_asset()->CopyFrom(makeAsset(
                assetName1, prop
        ));
        executor::execute(tx);
    }

    {
        const auto type = "transfer";
        Api::Transaction tx;
        Api::BaseObject value;
        value.set_valueint(50);

        Api::BaseObject author;
        author.set_valuestring(publicKey3);

        Api::BaseObject percent;
        percent.set_valuedouble(0.1);

        Api::BaseObject assetType;
        assetType.set_valuestring("tax");

        std::unordered_map<std::string,Api::BaseObject> prop;
        prop["value"] = value;
        prop["author"] = author;
        prop["percent"] = percent;
        prop["type"] = assetType;

        tx.set_senderpubkey(publicKey1);
        tx.set_receivepubkey(publicKey2);
        tx.set_type(type);
        tx.mutable_asset()->CopyFrom(makeAsset(
            assetName1, prop
        ));
        executor::execute(tx);
    }
    {
        Api::Asset asset1 = repository::asset::find(publicKey3, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        ASSERT_TRUE(asset1.value().at("value").valueint() == 10);
    }
    {
        Api::Asset asset1 = repository::asset::find(publicKey2, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        ASSERT_TRUE(asset1.value().at("value").valueint() == 145);
    }
    {
        Api::Asset asset1 = repository::asset::find(publicKey1, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        ASSERT_TRUE(asset1.value().at("value").valueint() == 50);
    }

    {
        const auto type = "transfer";
        Api::Transaction tx;
        Api::BaseObject value;
        value.set_valueint(50);

        Api::BaseObject author;
        author.set_valuestring(publicKey3);

        Api::BaseObject percent;
        percent.set_valuedouble(0.06);

        Api::BaseObject assetType;
        assetType.set_valuestring("tax");

        std::unordered_map<std::string,Api::BaseObject> prop;
        prop["value"] = value;
        prop["author"] = author;
        prop["percent"] = percent;
        prop["type"] = assetType;

        tx.set_senderpubkey(publicKey1);
        tx.set_receivepubkey(publicKey2);
        tx.set_type(type);
        tx.mutable_asset()->CopyFrom(makeAsset(
            assetName1, prop
        ));
        executor::execute(tx);
    }
    {
        Api::Asset asset1 = repository::asset::find(publicKey3, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        std::cout << asset1.value().at("value").valueint() ;
        ASSERT_TRUE(asset1.value().at("value").valueint() == 13);
    }
    {
        Api::Asset asset1 = repository::asset::find(publicKey2, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        ASSERT_TRUE(asset1.value().at("value").valueint() == 192);
    }
    {
        Api::Asset asset1 = repository::asset::find(publicKey1, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        ASSERT_TRUE(asset1.value().at("value").valueint() == 0);
    }

    removeData(publicKey1);
    removeData(publicKey2);
    removeData(publicKey3);
}
