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

TEST(ScenarioTest, AddNewAccountNoAsset) {
    const auto name = "mizuki";

    const auto publicKey  = "mXVNEpTcUiMUDZlAlMhmubbJXNqvwNlpSVXcFdu3qt4=";
    const auto type = "add";

    Api::Transaction tx;
    tx.set_senderpubkey(publicKey);
    tx.set_type(type);
    tx.mutable_account()->CopyFrom(makeAccount(
            publicKey, name, {}
    ));
    executor::execute(tx);

    Api::Account account = repository::account::find(publicKey);

    ASSERT_STREQ(account.publickey().c_str(), publicKey);
    ASSERT_STREQ(account.name().c_str(), name);
    IROHA_ASSERT_TRUE(account.assets_size() == 0);

    removeData(publicKey);
}


TEST(ScenarioTest, AddNewAccountHasSingleAsset) {
    const auto name = "mizuki";
    const auto assetName1 = "naocoin";

    const auto publicKey  = "mXVNEpTcUiMUDZlAlMhmubbJXNqvwNlpSVXcFdu3qt4=";
    const auto type = "add";

    Api::Transaction tx;
    tx.set_senderpubkey(publicKey);
    tx.set_type(type);
    tx.mutable_account()->CopyFrom(makeAccount(
        publicKey, name, {assetName1}
    ));
    executor::execute(tx);

    Api::Account account = repository::account::find(publicKey);

    ASSERT_STREQ(account.publickey().c_str(), publicKey);
    ASSERT_STREQ(account.name().c_str(), name);
    IROHA_ASSERT_TRUE(account.assets_size() == 1);
    ASSERT_STREQ(account.assets(0).c_str(), assetName1);

    {
        Api::Asset asset1 = repository::asset::find(publicKey, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        IROHA_ASSERT_TRUE(asset1.value().at("value").valueint() == 0);
    }

    removeData(publicKey);
}

TEST(ScenarioTest, AddNewAccountHasMultiAsset) {
    const auto name = "mizuki";
    const auto assetName1 = "naocoin";
    const auto assetName2 = "kayanocoin";

    const auto publicKey  = "mXVNEpTcUiMUDZlAlMhmubbJXNqvwNlpSVXcFdu3qt4=";
    const auto type = "add";

    Api::Transaction tx;
    tx.set_senderpubkey(publicKey);
    tx.set_type(type);
    tx.mutable_account()->CopyFrom(makeAccount(
        publicKey, name, {assetName1, assetName2}
    ));
    executor::execute(tx);

    Api::Account account = repository::account::find(publicKey);

    ASSERT_STREQ(account.publickey().c_str(), publicKey);
    ASSERT_STREQ(account.name().c_str(), name);
    ASSERT_TRUE(account.assets_size() == 2);
    ASSERT_STREQ(account.assets(0).c_str(), assetName1);
    ASSERT_STREQ(account.assets(1).c_str(), assetName2);

    {
        Api::Asset asset1 = repository::asset::find(publicKey, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        IROHA_ASSERT_TRUE(asset1.value().at("value").valueint() == 0);
    }

    {
        Api::Asset asset2 = repository::asset::find(publicKey, assetName2);
        ASSERT_STREQ(asset2.name().c_str(), assetName2);
        IROHA_ASSERT_TRUE(asset2.value().at("value").valueint() == 0);
    }

    removeData(publicKey);
}

TEST(ScenarioTest, AddNewAsset) {
    const auto name = "mizuki";
    const auto assetName1 = "naocoin";
    const auto assetName2 = "kayanocoin";

    //const auto privateKey = "iOHEkcoNgABDt9lz+ndjlLl5dSBkutleGerCSOGoP0nF3vArKBO7oyIxvoDSH+RGkcHRgQYgpRhRq3WSy8MYYA==";
    const auto publicKey  = "mXVNEpTcUiMUDZlAlMhmubbJXNqvwNlpSVXcFdu3qt4=";
    {
        Api::Transaction tx;
        tx.set_senderpubkey(publicKey);
        tx.set_type("add");
        tx.mutable_account()->CopyFrom(makeAccount(
                publicKey, name, {assetName1}
        ));
        executor::execute(tx);
        Api::Account account = repository::account::find(publicKey);
        ASSERT_STREQ(account.publickey().c_str(), publicKey);
        ASSERT_STREQ(account.name().c_str(), name);
        IROHA_ASSERT_TRUE(account.assets_size() == 1);
        ASSERT_STREQ(account.assets(0).c_str(), assetName1);
    }

    Api::Transaction tx;
    Api::BaseObject bobj;
    bobj.set_valueint(100);
    std::unordered_map<std::string,Api::BaseObject> prop;
    prop["value"] = bobj;

    tx.set_senderpubkey(publicKey);
    tx.set_type("add");
    tx.mutable_asset()->CopyFrom(makeAsset(
        assetName2, prop
    ));
    executor::execute(tx);

    Api::Account account = repository::account::find(publicKey);
    ASSERT_STREQ(account.publickey().c_str(), publicKey);
    ASSERT_STREQ(account.name().c_str(), name);
    IROHA_ASSERT_TRUE(account.assets().size() == 2);
    ASSERT_STREQ(account.assets(0).c_str(), assetName1);
    ASSERT_STREQ(account.assets(1).c_str(), assetName2);

    {
        Api::Asset asset1 = repository::asset::find(publicKey, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        IROHA_ASSERT_TRUE(asset1.value().at("value").valueint() == 0);
    }
    {
        Api::Asset asset2 = repository::asset::find(publicKey, assetName2);
        ASSERT_STREQ(asset2.name().c_str(), assetName2);
        IROHA_ASSERT_TRUE(asset2.value().at("value").valueint() == 100);
    }
    removeData(publicKey);

}


TEST(ScenarioTest, TaxTransfer) {

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
        ASSERT_STREQ(account.publickey().c_str(), publicKey1);
        ASSERT_STREQ(account.name().c_str(), name1);
        IROHA_ASSERT_TRUE(account.assets_size() == 1);
        ASSERT_STREQ(account.assets(0).c_str(), assetName1);

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
        Api::Account account = repository::account::find(publicKey2);
        ASSERT_STREQ(account.publickey().c_str(), publicKey2);
        ASSERT_STREQ(account.name().c_str(), name2);
        IROHA_ASSERT_TRUE(account.assets_size() == 1);
        ASSERT_STREQ(account.assets(0).c_str(), assetName1);

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
        Api::Account account = repository::account::find(publicKey3);
        ASSERT_STREQ(account.publickey().c_str(), publicKey3);
        ASSERT_STREQ(account.name().c_str(), name3);
        IROHA_ASSERT_TRUE(account.assets_size() == 1);
        ASSERT_STREQ(account.assets(0).c_str(), assetName1);
    }

    const auto type = "transfer";

    Api::Transaction tx;
    Api::BaseObject value;
    value.set_valueint(100);

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

    {
        Api::Asset asset1 = repository::asset::find(publicKey2, assetName1);
        ASSERT_STREQ(asset1.name().c_str(), assetName1);
        std::cout << asset1.value().at("value").valueint() << std::endl;
        ASSERT_TRUE(asset1.value().at("value").valueint() == 90);
    }
    removeData(publicKey1);
    removeData(publicKey2);
    removeData(publicKey3);
}