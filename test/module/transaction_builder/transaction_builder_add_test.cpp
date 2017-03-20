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
#include <infra/protobuf/api.pb.h>
#include <transaction_builder/transaction_builder.hpp>
#include <util/exception.hpp>

using txbuilder::TransactionBuilder;
using type_signatures::Add;
using type_signatures::Domain;
using type_signatures::Account;
using type_signatures::Asset;
using type_signatures::SimpleAsset;
using type_signatures::Peer;

constexpr auto commandType = "Add";
constexpr auto senderPublicKey = "sender public key";

/***************************************************************************
  Add
 ***************************************************************************/
TEST(transaction_builder, create_add_domain) {

  const auto ownerPublicKey = "owner public key";
  const auto name = "name";

  auto txDomain = TransactionBuilder<Add<Domain>>()
    .setSenderPublicKey(senderPublicKey)
    .setDomain(txbuilder::createDomain(ownerPublicKey, name))
    .build();

  ASSERT_STREQ(txDomain.senderpubkey().c_str(), senderPublicKey);
  ASSERT_STREQ(txDomain.type().c_str(), commandType);

  auto obj = txDomain.domain();
  ASSERT_STREQ(obj.ownerpublickey().c_str(), ownerPublicKey);
  ASSERT_STREQ(obj.name().c_str(), name);
}

TEST(transaction_builder, create_add_account) {

  const auto publicKey = "public key";
  const auto name = "name";
  const std::vector<std::string> assets = {
    "asset1",
    "asset2"
  };

  auto txAccount = TransactionBuilder<Add<Account>>()
    .setSenderPublicKey(senderPublicKey)
    .setAccount(txbuilder::createAccount(publicKey, name, assets))
    .build();

  ASSERT_STREQ(txAccount.senderpubkey().c_str(), senderPublicKey);
  ASSERT_STREQ(txAccount.type().c_str(), commandType);

  auto obj = txAccount.account();
  ASSERT_STREQ(obj.publickey().c_str(), publicKey);
  ASSERT_STREQ(obj.name().c_str(), name);
  for (int i = 0; i < obj.assets_size(); i++) {
    ASSERT_STREQ(obj.assets(i).c_str(), assets[i].c_str());
  }
}

TEST(transaction_builder, create_add_asset) {

  const auto domainID = "domainID";
  const auto name = "name";

  txbuilder::Map value;
  {
    value.emplace("key1", txbuilder::createValueString("value1"));
    value.emplace("key2", txbuilder::createValueInt(123456));
  }

  const auto scName = "contract_func";

  auto asset =
      txbuilder::createAsset(domainID, name, value, scName);

  auto txAsset = TransactionBuilder<Add<Asset>>()
                     .setSenderPublicKey(senderPublicKey)
                     .setAsset(std::move(asset))
                     .build();

  // Verify
  ASSERT_STREQ(txAsset.senderpubkey().c_str(), senderPublicKey);
  ASSERT_STREQ(txAsset.type().c_str(), commandType);

  auto obj = txAsset.asset();
  ASSERT_STREQ(obj.domain().c_str(), domainID);
  ASSERT_STREQ(obj.name().c_str(), name);

  ASSERT_TRUE(obj.value().find("key1")->second.value_case() == Api::BaseObject::ValueCase::kValueString);
  ASSERT_TRUE(obj.value().find("key2")->second.value_case() == Api::BaseObject::ValueCase::kValueInt);
  ASSERT_STREQ(obj.value().find("key1")->second.valuestring().c_str(), "value1");
  ASSERT_TRUE(obj.value().find("key2")->second.valueint() == 123456);
  ASSERT_STREQ(obj.smartcontractname().c_str(), scName);
}

TEST(transaction_builder, create_add_peer) {

  auto peer = txbuilder::createPeer(
    "publickey",
    "address",
    txbuilder::createTrust(1.23, true)
  );

  auto txPeer = TransactionBuilder<Add<Peer>>()
    .setSenderPublicKey(senderPublicKey)
    .setPeer(std::move(peer))
    .build();

  // Verify
  ASSERT_STREQ(txPeer.senderpubkey().c_str(), senderPublicKey);
  ASSERT_STREQ(txPeer.type().c_str(), commandType);

  auto obj = txPeer.peer();
  ASSERT_STREQ(obj.publickey().c_str(), "publickey");
  ASSERT_STREQ(obj.address().c_str(), "address");
  ASSERT_TRUE(obj.trust().value() == 1.23);
  ASSERT_TRUE(obj.trust().isok()  == true);
}

TEST(transaction_builder, create_add_simpleasset) {

  auto simpleAsset = txbuilder::createSimpleAsset(
    "domainID",
    "name",
    txbuilder::createValueInt(123456),
    "contract_func"
  );

  auto txSimpleAsset = TransactionBuilder<Add<SimpleAsset>>()
    .setSenderPublicKey(senderPublicKey)
    .setSimpleAsset(std::move(simpleAsset))
    .build();

  // Verify
  ASSERT_STREQ(txSimpleAsset.senderpubkey().c_str(), senderPublicKey);
  ASSERT_STREQ(txSimpleAsset.type().c_str(), commandType);

  auto obj = txSimpleAsset.simpleasset();
  ASSERT_STREQ(obj.domain().c_str(), "domainID");
  ASSERT_STREQ(obj.name().c_str(), "name");

  ASSERT_TRUE(obj.value().value_case() == Api::BaseObject::ValueCase::kValueInt);
  ASSERT_TRUE(obj.value().valueint() == 123456);
  ASSERT_STREQ(obj.smartcontractname().c_str(), "contract_func");
}
