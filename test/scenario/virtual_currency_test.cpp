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
#include <repository/domain/account_repository.hpp>
#include <repository/domain/asset_repository.hpp>
#include <service/executor.hpp>

#include "test_utils.hpp"

TEST(ScenarioTest, CurrencyTransfer) {
  const auto name1 = "mizuki";
  const auto name2 = "iori";
  const auto name3 = "maruuchi";

  const auto assetName1 = "naocoin";
  const auto assetName2 = "kayanocoin";

  const auto publicKey1 = "mXVNEpTcUiMUDZlAlMhmubbJXNqvwNlpSVXcFdu3qt4=";
  const auto publicKey2 = "p8v1bQ76tHmD3fZrYDWBvzn+aJspfaBw8OwNg+Cx8AI=";
  const auto publicKey3 = "q3W9e3fc65WyQXQDSUxRd1dXL1x8/4G93IKDBmXV54w=";

  {
    const auto type = "add";
    Api::Transaction tx;
    tx.set_senderpubkey(publicKey1);
    tx.set_type(type);
    tx.mutable_account()->CopyFrom(
        makeAccount(publicKey1, name1, {assetName1, assetName2}));
    executor::execute(tx);
    {
      Api::BaseObject bobj;
      bobj.set_valueint(100);
      std::unordered_map<std::string, Api::BaseObject> prop;
      prop["value"] = bobj;

      tx.Clear();
      tx.set_senderpubkey(publicKey1);
      tx.set_type(type);
      tx.mutable_asset()->CopyFrom(makeAsset(assetName1, prop));
      executor::execute(tx);
    }
    {
      Api::BaseObject bobj;
      bobj.set_valueint(200);
      std::unordered_map<std::string, Api::BaseObject> prop;
      prop["value"] = bobj;

      tx.Clear();
      tx.set_senderpubkey(publicKey1);
      tx.set_type(type);
      tx.mutable_asset()->CopyFrom(makeAsset(assetName2, prop));
      executor::execute(tx);
    }
  }
  {
    const auto type = "add";
    Api::Transaction tx;
    tx.set_senderpubkey(publicKey2);
    tx.set_type(type);
    tx.mutable_account()->CopyFrom(
        makeAccount(publicKey1, name1, {assetName1, assetName2}));
    executor::execute(tx);
    {
      Api::BaseObject bobj;
      bobj.set_valueint(0);
      std::unordered_map<std::string, Api::BaseObject> prop;
      prop["value"] = bobj;

      tx.Clear();
      tx.set_senderpubkey(publicKey2);
      tx.set_type(type);
      tx.mutable_asset()->CopyFrom(makeAsset(assetName1, prop));
      executor::execute(tx);
    }
    {
      Api::BaseObject bobj;
      bobj.set_valueint(0);
      std::unordered_map<std::string, Api::BaseObject> prop;
      prop["value"] = bobj;

      tx.Clear();
      tx.set_senderpubkey(publicKey2);
      tx.set_type(type);
      tx.mutable_asset()->CopyFrom(makeAsset(assetName2, prop));
      executor::execute(tx);
    }
  }

  {
    const auto type = "transfer";
    Api::Transaction tx;
    Api::BaseObject value;
    value.set_valueint(100);

    std::unordered_map<std::string, Api::BaseObject> prop;
    prop["value"] = value;

    tx.set_senderpubkey(publicKey1);
    tx.set_receivepubkey(publicKey2);
    tx.set_type(type);
    tx.mutable_asset()->CopyFrom(makeAsset(assetName1, prop));
    executor::execute(tx);
  }
  {
    const auto type = "transfer";
    Api::Transaction tx;
    Api::BaseObject value;
    value.set_valueint(200);

    std::unordered_map<std::string, Api::BaseObject> prop;
    prop["value"] = value;

    tx.set_senderpubkey(publicKey1);
    tx.set_receivepubkey(publicKey2);
    tx.set_type(type);
    tx.mutable_asset()->CopyFrom(makeAsset(assetName2, prop));
    executor::execute(tx);
  }
  {
    Api::Asset asset1 = repository::asset::find(publicKey2, assetName1);
    ASSERT_STREQ(asset1.name().c_str(), assetName1);
    ASSERT_TRUE(asset1.value().at("value").valueint() == 100);
  }
  {
    Api::Asset asset1 = repository::asset::find(publicKey2, assetName2);
    ASSERT_STREQ(asset1.name().c_str(), assetName2);
    ASSERT_TRUE(asset1.value().at("value").valueint() == 200);
  }
  {
    Api::Asset asset1 = repository::asset::find(publicKey1, assetName1);
    ASSERT_STREQ(asset1.name().c_str(), assetName1);
    ASSERT_TRUE(asset1.value().at("value").valueint() == 0);
  }
  {
    Api::Asset asset1 = repository::asset::find(publicKey1, assetName2);
    ASSERT_STREQ(asset1.name().c_str(), assetName2);
    ASSERT_TRUE(asset1.value().at("value").valueint() == 0);
  }


  {
    const auto type = "transfer";
    Api::Transaction tx;
    Api::BaseObject value;
    value.set_valueint(150);

    std::unordered_map<std::string, Api::BaseObject> prop;
    prop["value"] = value;

    tx.set_senderpubkey(publicKey2);
    tx.set_receivepubkey(publicKey1);
    tx.set_type(type);
    tx.mutable_asset()->CopyFrom(makeAsset(assetName2, prop));
    executor::execute(tx);
  }
  {
    Api::Asset asset1 = repository::asset::find(publicKey1, assetName2);
    ASSERT_STREQ(asset1.name().c_str(), assetName2);
    ASSERT_TRUE(asset1.value().at("value").valueint() == 150);
  }

  {
    const auto type = "transfer";
    Api::Transaction tx;
    Api::BaseObject value;
    value.set_valueint(150);

    std::unordered_map<std::string, Api::BaseObject> prop;
    prop["value"] = value;

    tx.set_senderpubkey(publicKey2);
    tx.set_receivepubkey(publicKey1);
    tx.set_type(type);
    tx.mutable_asset()->CopyFrom(makeAsset(assetName2, prop));
    executor::execute(tx);
  }
  {
    Api::Asset asset1 = repository::asset::find(publicKey1, assetName2);
    ASSERT_STREQ(asset1.name().c_str(), assetName2);
    ASSERT_TRUE(asset1.value().at("value").valueint() == 150);
  }

  {
    const auto type = "transfer";
    Api::Transaction tx;
    Api::BaseObject value;
    value.set_valueint(50);

    std::unordered_map<std::string, Api::BaseObject> prop;
    prop["value"] = value;

    tx.set_senderpubkey(publicKey2);
    tx.set_receivepubkey(publicKey3);
    tx.set_type(type);
    tx.mutable_asset()->CopyFrom(makeAsset(assetName2, prop));
    executor::execute(tx);
  }
  {
    Api::Asset asset1 = repository::asset::find(publicKey2, assetName2);
    ASSERT_STREQ(asset1.name().c_str(), assetName2);
    ASSERT_TRUE(asset1.value().at("value").valueint() == 50);
  }

  removeData(publicKey1);
  removeData(publicKey2);
}
