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
#include "../../../core/model/transaction_builder/transaction_builder.hpp"
#include "../../../core/util/exception.hpp"

using transaction::TransactionBuilder;

/***************************************************************************
  Domain
 ***************************************************************************/
TEST(transaction_builder, create_domain) {
  
  const std::vector<std::string> ownerPublicKey = {"publicKey1", "publicKey2"};
  const std::string name = "My domain";

  auto domain = TransactionBuilder<object::Domain>()
    .setOwnerPublicKey(ownerPublicKey)
    .setName(name)
    .build();

  ASSERT_TRUE(ownerPublicKey.size() == domain.ownerPublicKey.size());

  for (std::size_t i=0; i<ownerPublicKey.size(); i++) {
    ASSERT_STREQ(ownerPublicKey[i].c_str(), domain.ownerPublicKey[i].c_str());
  }

  ASSERT_STREQ(name.c_str(), domain.name.c_str());
}

TEST(transaction_builder, create_domain_unset_members) {
  
  const std::vector<std::string> ownerPublicKey = {"publicKey1", "publicKey2"};
  const std::string name = "My domain";

  ASSERT_THROW({TransactionBuilder<object::Domain>()
      .setOwnerPublicKey(ownerPublicKey)
      .build();
  }, exception::transaction::UnsetBuildArgmentsException);
  
  ASSERT_THROW({TransactionBuilder<object::Domain>()
      .setName(name)
      .build();
  }, exception::transaction::UnsetBuildArgmentsException);
  
  ASSERT_THROW({TransactionBuilder<object::Domain>()
      .build();
  }, exception::transaction::UnsetBuildArgmentsException);
}

/***************************************************************************
  Asset
 ***************************************************************************/
TEST(transaction_builder, create_asset) {

  const std::string domainId = "domainId";
  const std::string name = "karin";

  object::AssetValueT value({{"key1", object::BaseObject("val1")}, {"key2", object::BaseObject(123.456f)}});

  ASSERT_TRUE(static_cast<std::string>(value["key1"]) == static_cast<std::string>("val1"));
  ASSERT_TRUE(static_cast<float>(value["key2"]) == 123.456f);

  auto asset = TransactionBuilder<object::Asset>()
    .setDomain(domainId)
    .setName(name)
    .setValue(value)
    .build();

  ASSERT_STREQ(domainId.c_str(), asset.domain.c_str());
  ASSERT_STREQ(name.c_str(), asset.name.c_str());
  
  for (auto&& e: value) {
    const auto eKey = e.first;
    const auto eVal = e.second;

    ASSERT_TRUE(asset.value.find(eKey) != asset.value.end());
    ASSERT_TRUE(asset.value.find(eKey)->second == eVal);
  }

  for (auto&& e: asset.value) {
    const auto eKey = e.first;
    const auto eVal = e.second;

    ASSERT_TRUE(value.find(eKey) != value.end());
    ASSERT_TRUE(value.find(eKey)->second == eVal);
  }
}

TEST(transaction_builder, create_asset_unset_members) {

  const std::string domainId = "domainId";
  const std::string name = "karin";

  object::AssetValueT value({{"key1", object::BaseObject("val1")}, {"key2", object::BaseObject(123.456f)}});

  ASSERT_THROW({TransactionBuilder<object::Asset>()
      .setName(name).setValue(value)
      .build();
  }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Asset>()
      .setDomain(domainId).setValue(value)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Asset>()
      .setDomain(domainId).setName(name)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Asset>()
      .setDomain(domainId)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Asset>()
      .setName(name)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Asset>()
      .setValue(value)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Asset>()
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);
}


/***************************************************************************
  Account
 ***************************************************************************/
TEST(transaction_builder, create_account) {

  const std::string ownerPublicKey = "ownerPublicKey";
  const std::string name = "karin";

  auto createDomain = [](std::vector<std::string>&& pubkeys, std::string&& name) {
    return TransactionBuilder<object::Domain>()
      .setOwnerPublicKey(std::move(pubkeys))
      .setName(std::move(name))
      .build();
  };

  std::vector<object::Domain> assets = {
    createDomain({"publicKey1", "publicKey2"}, "My domain"),
    createDomain({"abcdefg"}, "domain name"),
  };

  auto account = TransactionBuilder<object::Account>()
    .setOwnerPublicKey(ownerPublicKey)
    .setName(name)
    .setAssets(assets)
    .build();

  ASSERT_STREQ(ownerPublicKey.c_str(), account.ownerPublicKey.c_str());
  ASSERT_STREQ(name.c_str(), account.name.c_str());

  {
    ASSERT_TRUE(account.assets[0].ownerPublicKey.size() == 2);
    ASSERT_STREQ(account.assets[0].ownerPublicKey[0].c_str(), "publicKey1");
    ASSERT_STREQ(account.assets[0].ownerPublicKey[1].c_str(), "publicKey2");
    ASSERT_STREQ(account.assets[0].name.c_str(), "My domain");
  }

  {
    ASSERT_TRUE(account.assets[1].ownerPublicKey.size() == 1);
    ASSERT_STREQ(account.assets[1].ownerPublicKey[0].c_str(), "abcdefg");
    ASSERT_STREQ(account.assets[1].name.c_str(), "domain name");
  }

}


/***************************************************************************
  Message
 ***************************************************************************/
TEST(transaction_builder, create_message) {

  const std::string text = "a message";

  auto message = TransactionBuilder<object::Message>()
    .setMessage(text)
    .build();

  ASSERT_STREQ(text.c_str(), message.text.c_str());
}

TEST(transaction_builder, create_message_unset_members) {

  const std::string domainId = "domainId";
  const std::string name = "karin";
  
  ASSERT_THROW({TransactionBuilder<object::Message>()
      .build();
  }, exception::transaction::UnsetBuildArgmentsException);
}

/***************************************************************************
  SimpleAsset
 ***************************************************************************/
TEST(transaction_builder, create_simpleAsset) {

  const std::string domainId = "domainId";
  const std::string name = "karin";
  const object::BaseObject value(12345);

  ASSERT_TRUE(static_cast<int>(value) == 12345);
  
  auto simpleAsset = TransactionBuilder<object::SimpleAsset>()
    .setDomain(domainId)
    .setName(name)
    .setValue(value)
    .build();

  ASSERT_STREQ(domainId.c_str(), simpleAsset.domain.c_str());
  ASSERT_STREQ(name.c_str(), simpleAsset.name.c_str());
  ASSERT_TRUE(static_cast<int>(value) == static_cast<int>(simpleAsset.value));
}

TEST(transaction_builder, create_simpleAsset_unset_members) {

  const std::string domainId = "domainId";
  const std::string name = "karin";
  const object::BaseObject value(12345);

  ASSERT_THROW({TransactionBuilder<object::SimpleAsset>()
      .setName(name).setValue(value)
      .build();
  }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::SimpleAsset>()
      .setDomain(domainId).setValue(value)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::SimpleAsset>()
      .setDomain(domainId).setName(name)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::SimpleAsset>()
      .setDomain(domainId)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::SimpleAsset>()
      .setName(name)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::SimpleAsset>()
      .setValue(value)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::SimpleAsset>()
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);
}


/***************************************************************************
  Peer
 ***************************************************************************/
TEST(transaction_builder, create_peer) {

  const std::string ip = "111.111.111.111";
  const std::string publicKey = "publicKey";
  const double trustScore = 123.123456;

  auto peer = TransactionBuilder<object::Peer>()
    .setIP(ip)
    .setPublicKey(publicKey)
    .setTrustScore(trustScore)
    .build();

  ASSERT_STREQ(ip.c_str(), peer.getIP().c_str());
  ASSERT_STREQ(publicKey.c_str(), peer.getPublicKey().c_str());
  ASSERT_TRUE(trustScore == peer.getTrustScore());
}

TEST(transaction_builder, create_peer_unset_members) {

  const std::string ip = "111.111.111.111";
  const std::string publicKey = "publicKey";
  const double trustScore = 123.123456;

  auto peer = TransactionBuilder<object::Peer>()
    .setIP(ip)
    .setPublicKey(publicKey)
    .setTrustScore(trustScore)
    .build();

  ASSERT_THROW({TransactionBuilder<object::Peer>()
      .setPublicKey(publicKey).setTrustScore(trustScore)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Peer>()
      .setIP(ip).setTrustScore(trustScore)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Peer>()
      .setIP(ip).setPublicKey(publicKey)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Peer>()
      .setIP(ip)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Peer>()
      .setPublicKey(publicKey)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Peer>()
      .setTrustScore(trustScore)
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);

  ASSERT_THROW({TransactionBuilder<object::Peer>()
      .build();
    }, exception::transaction::UnsetBuildArgmentsException);
}
