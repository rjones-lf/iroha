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

#include <gtest/gtest.h>

#include <cmath>
#include <../smart_contract/repository/jni_constants.hpp>
#include <infra/protobuf/api.pb.h>
#include <repository/domain/asset_repository.hpp>
#include <repository/world_state_repository.hpp>
#include <virtual_machine/virtual_machine.hpp>
#include <infra/virtual_machine/jvm/java_data_structure.hpp>

const std::string PackageName = "test";
const std::string ContractName = "TestAsset";

namespace tag = jni_constants;

std::map<std::string, std::string> assetInfo;
std::map<std::string, std::map<std::string, std::string>> assetValue;

void ensureIntegrityOfAsset(const std::string& assetUuid) {
  const std::string received_asset_value =
      repository::world_state_repository::find(assetUuid);

  // Restore asset.
  Api::Asset asset;
  asset.ParseFromString(received_asset_value);

  ASSERT_STREQ(assetInfo[tag::DomainId].c_str(), asset.domain().c_str());
  ASSERT_STREQ(assetInfo[tag::AssetName].c_str(), asset.name().c_str());
  ASSERT_STREQ(assetInfo[tag::SmartContractName].c_str(), asset.smartcontractname().c_str());

  using virtual_machine::jvm::convertBaseObjectToMapString;

  for (auto &&e: assetValue) {
    std::map<std::string, std::string> lhs = e.second;
    Api::BaseObject rhsObj = asset.value().find(e.first)->second;
    std::map<std::string, std::string> rhs = convertBaseObjectToMapString(rhsObj);

    std::cout << "(" << lhs["type"] << ", " << lhs["value"] << ") vs "
              << "(" << rhs["type"] << ", " << rhs["value"] << ")\n";

    if (lhs["type"] != rhs["type"]) {
      throw "Mismatch type";
    }

    if (lhs["type"] == "double") {
      const auto lhsValue = std::stod(lhs["value"]);
      const auto rhsValue = std::stod(rhs["value"]);
      ASSERT_TRUE(std::abs(lhsValue - rhsValue) < 1e-5);
    } else {
      ASSERT_TRUE(lhs["value"] == rhs["value"]);
    }
  }

  for (auto &&e: asset.value()) {
    std::map<std::string, std::string> lhs = assetValue.find(e.first)->second;
    Api::BaseObject rhsObj = e.second;
    std::map<std::string, std::string> rhs = convertBaseObjectToMapString(rhsObj);
    
    std::cout << "(" << lhs["type"] << ", " << lhs["value"] << ") vs "
              << "(" << rhs["type"] << ", " << rhs["value"] << ")\n";

    if (lhs["type"] != rhs["type"]) {
      throw "Mismatch type";
    }

    if (lhs["type"] == "double") {
      const auto lhsValue = std::stod(lhs["value"]);
      const auto rhsValue = std::stod(rhs["value"]);
      ASSERT_TRUE(std::abs(lhsValue - rhsValue) < 1e-5);
    } else {
      ASSERT_TRUE(lhs["value"] == rhs["value"]);
    }
  }
}

/*********************************************************************************************************
 * Test Asset
 *********************************************************************************************************/
TEST(JavaQueryRepoAsset, InitializeVM) {
  virtual_machine::initializeVM(PackageName, ContractName);
}

TEST(JavaQueryRepoAsset, invokeAddAssetQuery) {

  /***********************************************************************
   * 1. Initial guess
   ***********************************************************************/
  const auto assetUuid = "3f8ba1e5df7f1587defc8fae4789207c8719c7b6d86ce299821b8a83fe08b5a9";

  assetInfo[tag::DomainId] = "A domain id";
  assetInfo[tag::AssetName] = "Currency";
  assetInfo[tag::SmartContractName] = "smartContractFunc";

  assetValue["ownerName"] = {
      {"type", "string"},
      {"value", "karin"}
  };

  assetValue["my_money"] = {
      {"type", "int"},
      {"value", "123456"}
  };

  assetValue["rate"] = {
      {"type", "double"},
      {"value", "0.12345678901234567890123456789"}
  };

  /***********************************************************************
   * 2. Invoke Java
   ***********************************************************************/
  virtual_machine::invokeFunction(PackageName, ContractName, "testAddAsset",
                                  assetInfo, assetValue);

  /***********************************************************************
   * 3. Test
   ***********************************************************************/
  std::cout << "In c++:" << std::endl;
  ensureIntegrityOfAsset(assetUuid);

  // Remove chache.
  ASSERT_TRUE(repository::asset::remove(assetUuid));
}

TEST(JavaQueryRepoAsset, invokeUpdateAssetQuery) {
  /***********************************************************************
   * 1. Initial guess
   ***********************************************************************/
  const auto assetUuid = "3f8ba1e5df7f1587defc8fae4789207c8719c7b6d86ce299821b8a83fe08b5a9";

  assetInfo[tag::DomainId] = "A domain id";
  assetInfo[tag::AssetName] = "Currency";
  assetInfo[tag::SmartContractName] = "smartContractFunc";

  assetValue["ownerName"] = {
      {"type", "string"},
      {"value", "karin"}
  };

  assetValue["my_money"] = {
      {"type", "int"},
      {"value", "123456"}
  };

  assetValue["rate"] = {
      {"type", "double"},
      {"value", "0.12345678901234567890123456789"}
  };

  virtual_machine::invokeFunction(PackageName, ContractName, "testAddAsset",
                                  assetInfo, assetValue);

  std::map<std::string, std::string> params;
  {
    params[tag::Uuid] = assetUuid;
  }

  assetValue["ownerName"] = {
      {"type", "string"},
      {"value", "asuka"}
  };

  assetValue["my_money"] = {
      {"type", "int"},
      {"value", "98765"}
  };

  assetValue["updated_value"] = {
      {"type", "string"},
      {"value", "アセット更新のテスト"}
  };

  /***********************************************************************
   * 2. Invocation Java.
   ***********************************************************************/
  virtual_machine::invokeFunction(PackageName, ContractName, "testUpdateAsset",
                                  params, assetValue);

  /***********************************************************************
   * 3. Test
   ***********************************************************************/
  std::cout << "In c++:" << std::endl;
  ensureIntegrityOfAsset(assetUuid);

  // Remove chache.
  ASSERT_TRUE(repository::asset::remove(assetUuid));
}

TEST(JavaQueryRepoAsset, invokeRemoveAssetQuery) {
  /***********************************************************************
   * 1. Invocation Java.
   ***********************************************************************/
  const auto assetUuid = "3f8ba1e5df7f1587defc8fae4789207c8719c7b6d86ce299821b8a83fe08b5a9";

  assetInfo[tag::DomainId] = "A domain id";
  assetInfo[tag::AssetName] = "Currency";
  assetInfo[tag::SmartContractName] = "smartContractFunc";

  assetValue["ownerName"] = {
      {"type", "string"},
      {"value", "karin"}
  };

  assetValue["my_money"] = {
      {"type", "int"},
      {"value", "123456"}
  };

  assetValue["rate"] = {
      {"type", "double"},
      {"value", "0.12345678901234567890123456789"}
  };

  virtual_machine::invokeFunction(PackageName, ContractName, "testAddAsset",
                                  assetInfo, assetValue);

  std::map<std::string, std::string> params;
  {
    params[tag::Uuid] = assetUuid;
  }

  virtual_machine::invokeFunction(PackageName, ContractName, "testRemoveAsset",
                                  params);
  /***********************************************************************
   * 2. Test
   ***********************************************************************/
  ASSERT_FALSE(repository::asset::exists(assetUuid)); // cache has removed.
}

TEST(JavaQueryRepoAsset, reinvokeAddAssetQuery) {
  /***********************************************************************
   * 1. Invocation Java.
   ***********************************************************************/
  const auto assetUuid = "3f8ba1e5df7f1587defc8fae4789207c8719c7b6d86ce299821b8a83fe08b5a9";

  assetInfo[tag::DomainId] = "A domain id";
  assetInfo[tag::AssetName] = "Currency";
  assetInfo[tag::SmartContractName] = "smartContractFunc";

  assetValue["ownerName"] = {
      {"type", "string"},
      {"value", "karin"}
  };

  assetValue["my_money"] = {
      {"type", "int"},
      {"value", "123456"}
  };

  assetValue["rate"] = {
      {"type", "double"},
      {"value", "0.12345678901234567890123456789"}
  };

  virtual_machine::invokeFunction(PackageName, ContractName, "testAddAsset",
                                  assetInfo, assetValue);

  ASSERT_TRUE(repository::asset::remove(assetUuid));

  assetInfo[tag::DomainId] = "組織";
  assetInfo[tag::AssetName] = "アセット";
  assetInfo[tag::SmartContractName] = "SCFunc";

  assetValue["key1"] = {
      {"type", "int"},
      {"value", "1234567"}
  };

  assetValue["aaaaa"] = {
      {"type", "double"},
      {"value", "0.987654321234567890"}
  };

  virtual_machine::invokeFunction(PackageName, ContractName, "testAddAsset",
                                  assetInfo, assetValue);

  /***********************************************************************
   * 2. Test
   ***********************************************************************/
  const auto newAssetUuid = "7cdd09dff5f8f586a64a8948cac4937ccc1c7e3362de3125861966cfc7d177d7";
  ASSERT_FALSE(repository::asset::exists(assetUuid));
  ASSERT_TRUE(repository::asset::exists(newAssetUuid));

  // Remove chache
  ASSERT_TRUE(repository::asset::remove(newAssetUuid));
}

TEST(JavaQueryRepoAsset, FinishVM) {
  virtual_machine::finishVM(PackageName, ContractName);
}
