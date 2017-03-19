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

#include <infra/config/peer_service_with_json.hpp>
#include <infra/protobuf/api.pb.h>
#include <service/peer_service.hpp>

#include <repository/domain/account_repository.hpp>
#include <repository/domain/asset_repository.hpp>

namespace executor {

  using Api::Transaction;

  void add(const Transaction &tx) {
    logger::info("executor") << "tx has peer?" << (tx.has_peer() ? "yes" : "no");
    if (tx.has_asset()) {
      // Add<Asset>
      const auto asset = tx.asset();
      repository::asset::add(tx.senderpubkey(), asset.name(), asset);
    } else if (tx.has_domain()) {
      // Add<Domain>
      // Domain will be supported by v1.0
      const auto domain = tx.domain();
    } else if (tx.has_account()) {
      // Add<Account>
      const auto account = tx.account();
      repository::account::add(account.publickey(), account);

      for (auto asset_name : account.assets()) {
        // Add default asset
        auto asset = Api::Asset();
        auto base = Api::BaseObject();
        base.set_valueint(0);
        asset.set_name(asset_name);
        asset.set_domain("default");
        (*asset.mutable_value())["value"] = base;
        logger::info("executor") << "add asset: " << asset.DebugString();
        repository::asset::add(tx.senderpubkey(), asset.name(), asset);
      }
      logger::info("executor") << "add account";
    } else if (tx.has_peer()) {
      logger::info("executor") << "add peer";
      // Temporary - to operate peer service
      peer::Node query_peer(tx.peer().address(), tx.peer().publickey(),
                            tx.peer().trust().value(), tx.peer().trust().isok());
      ::peer::transaction::executor::add(query_peer);
      if (::peer::myself::getIp() != query_peer.ip && ::peer::myself::isActive())
        ::peer::transaction::izanami::start(query_peer);
    }
  }

  void valueValidation(Api::Asset asset, std::vector<std::string> names) {}

  void transfer(const Transaction &tx) {
    if (tx.has_asset()) {
      // Transfer<Asset>
      auto sender = tx.senderpubkey();
      auto receiver = tx.receivepubkey();
      const auto assetName = tx.asset().name();

      // **********************************************************************************
      // * This is Transfer<Asset>'s logic. Tax send logic
      // **********************************************************************************
      if (tx.asset().value().find("author") != tx.asset().value().end()) {
        const auto author = tx.asset().value().at("author").valuestring();

        const auto value = tx.asset().value().at("value").valueint();

        auto senderAsset = repository::asset::find(sender, assetName);
        auto authorAsset = repository::asset::find(author, assetName);

        auto receiverAsset = repository::asset::find(receiver, assetName);
        if (senderAsset.value().find("value") != senderAsset.value().end() &&
            receiverAsset.value().find("value") != receiverAsset.value().end()) {
          auto senderValue = senderAsset.value().at("value").valueint();
          auto receiverValue = receiverAsset.value().at("value").valueint();
          if (senderValue > value) {
            (*senderAsset.mutable_value())["value"].set_valueint(senderValue -
                                                                 value);
            (*receiverAsset.mutable_value())["value"].set_valueint(receiverValue +
                                                                   value);
            (*authorAsset.mutable_value())["value"].set_valueint(value * 0.8);
          }
        }
        repository::asset::update(sender, assetName, senderAsset);
        repository::asset::update(receiver, assetName, receiverAsset);

        // **********************************************************************************
        // * This is Transfer<Asset>'s logic. multi message chat
        // **********************************************************************************
      } else if (tx.asset().value().find("targetName") !=
                 tx.asset().value().end()) {
        const auto targetName = tx.asset().value().at("targetName").valuestring();
        auto senderAsset = repository::asset::find(sender, assetName);
        auto receiverAsset = repository::asset::find(receiver, assetName);

        auto senderHasNum = (*senderAsset.mutable_value())[targetName].valueint();
        auto receiverHasNum =
            (*receiverAsset.mutable_value())[targetName].valueint();

        (*senderAsset.mutable_value())[targetName].set_valueint(senderHasNum - 1);
        (*receiverAsset.mutable_value())[targetName].set_valueint(receiverHasNum +
                                                                  1);

        repository::asset::update(sender, assetName, senderAsset);
        repository::asset::update(receiver, assetName, receiverAsset);

        // **********************************************************************************
        // * This is Transfer<Asset>'s logic. virtual currency
        // **********************************************************************************
      } else if (tx.asset().value().find("value") != tx.asset().value().end()) {
        const auto value = tx.asset().value().at("value").valueint();
        auto senderAsset = repository::asset::find(sender, assetName);
        auto receiverAsset = repository::asset::find(receiver, assetName);
        if (senderAsset.value().find("value") != senderAsset.value().end() &&
            receiverAsset.value().find("value") != receiverAsset.value().end()) {
          auto senderValue = senderAsset.value().at("value").valueint();
          auto receiverValue = receiverAsset.value().at("value").valueint();
          if (senderValue > value) {
            (*senderAsset.mutable_value())["value"].set_valueint(senderValue -
                                                                 value);
            (*receiverAsset.mutable_value())["value"].set_valueint(receiverValue +
                                                                   value);
          }
        }
        repository::asset::update(sender, assetName, senderAsset);
        repository::asset::update(receiver, assetName, receiverAsset);
      }
    } else if (tx.has_domain()) {
      // Domain will be supported by v1.0
      // Transfer<Domain>
    } else if (tx.has_peer()) {
      // Transfer<Peer>
      // nothing this transaction
    }
  }

  void update(const Transaction &tx) {
    if (tx.has_asset()) {

      // **********************************************************************************
      // * This is Transfer<Asset>'s logic. virtual currency
      // **********************************************************************************
      // Update<Asset>
      logger::info("executor") << "Update";
      const auto asset = tx.asset();
      const auto publicKey = tx.senderpubkey();
      const auto assetName = asset.name();
      if (asset.value().find("value") != asset.value().end()) {
        repository::asset::update(publicKey, assetName, asset);
      }
    } else if (tx.has_domain()) {
      // Domain will be supported by v1.0
    } else if (tx.has_account()) {
      // Update<Account>
      logger::info("executor") << "Update";
      const auto account = tx.account();
      repository::account::update(account.publickey(), account);
    } else if (tx.has_peer()) {
      // Temporary - to operate peer service
      peer::Node query_peer(tx.peer().address(), tx.peer().publickey(),
                            tx.peer().trust().value(), tx.peer().trust().isok());
      ::peer::transaction::executor::update(query_peer.publicKey, query_peer);
      // Update<Peer>
    }
  }

  void remove(const Transaction &tx) {
    if (tx.has_asset()) {
      // Remove<Asset>
      const auto name = tx.account().name();
      repository::asset::remove(tx.senderpubkey(), name);
    } else if (tx.has_domain()) {
      // Remove<Domain>
      // Domain will be supported by v1.0
    } else if (tx.has_account()) {
      // Remove<Account>
      const auto account = tx.account();
      repository::account::remove(account.publickey());
    } else if (tx.has_peer()) {
      // Temporary - to operate peer service
      peer::Node query_peer(tx.peer().address(), tx.peer().publickey(),
                            tx.peer().trust().value(), tx.peer().trust().isok());
      ::peer::transaction::executor::remove(query_peer.publicKey);
    }
  }

  void contract(const Transaction &tx) {
    if (tx.has_asset()) {
      // Contract<Asset>
    } else if (tx.has_domain()) {
      // Contract<Domain>
    } else if (tx.has_account()) {
      // Contract<Account>
    } else if (tx.has_peer()) {
      // Contract<Peer>
      // nothing this transaction
    }
  }

  void execute(const Transaction &tx) {
    logger::info("executor") << "Executor";
    logger::info("executor") << "DebugString:" << tx.DebugString();
    logger::info("executor") << "tx type(): " << tx.type();
    std::string type = tx.type();
    std::transform(cbegin(type), cend(type), begin(type), ::tolower);

    if (type == "add") {
      add(tx);
    } else if (type == "transfer") {
      transfer(tx);
    } else if (type == "update") {
      update(tx);
    } else if (type == "remove") {
      remove(tx);
    } else if (type == "contract") {
      contract(tx);
    }
  }
};
