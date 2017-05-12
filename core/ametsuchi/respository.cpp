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

#include <string>
#include <asset_generated.h>
#include <endpoint_generated.h>
#include <infra/ametsuchi/include/ametsuchi/ametsuchi.h>
#include <main_generated.h>
#include <crypto/hash.hpp>
#include <service/flatbuffer_service.h>
#include <service/connection.hpp>
#include <string>
#include <memory>
#include <sys/stat.h>

namespace repository {

static std::unique_ptr<ametsuchi::Ametsuchi> db;
const std::string folder = "/tmp/ametsuchi/";

void init() {
  struct stat st;
  int ret = stat((folder + "data.mdb").c_str(), &st);
  if (!ret) {
    std::cout << folder + "data.mdb already exists.\n";
    exit(0);
  }

  ret = stat((folder + "lock.mdb").c_str(), &st);
  if (!ret) {
    std::cout << folder + "lock.mdb already exists.\n";
    exit(0);
  }
  db = std::make_unique<ametsuchi::Ametsuchi>("/tmp/ametsuchi/");
}

void append(const iroha::Transaction &tx) {
  auto buf = flatbuffer_service::transaction::GetTxPointer(tx);
  db->append(&buf.value());
}

const ::iroha::Transaction *getTransaction(size_t index) {
  return db->getTransaction(index, false);
}

std::vector<const iroha::Asset *> findAssetByPublicKey(
    const flatbuffers::String &key) {
  flatbuffers::FlatBufferBuilder fbb;
  return db->accountGetAllAssets(&key);
}

bool existAccountOf(const flatbuffers::String &key) {
  return false;
}

bool checkUserCanPermission(const flatbuffers::String &key) {
  return false;
}

const std::string getMerkleRoot() { return db->getMerkleRoot(); }

namespace permission {

std::vector<const iroha::AccountPermissionLedger *> getPermissionLedgerOf(
    const flatbuffers::String &key) {
  return db->assetGetPermissionLedger(&key);
}

std::vector<const iroha::AccountPermissionDomain *> getPermissionDomainOf(
    const flatbuffers::String &key) {
  return db->assetGetPermissionDomain(&key);
}

std::vector<const iroha::AccountPermissionAsset *> getPermissionAssetOf(
    const flatbuffers::String &key) {
  return db->assetGetPermissionAsset(&key);
}
}
namespace front_repository {
void initialize_repository() {
  connection::iroha::AssetRepositoryImpl::AccountGetAsset::receive(
      [=](const std::string & /* from */, flatbuffers::unique_ptr_t &&query_ptr)
          -> std::vector<const ::iroha::Asset *> {
        const iroha::AssetQuery &query =
            *flatbuffers::GetRoot<iroha::AssetQuery>(query_ptr.get());
        auto ln = query.ledger_name();
        auto dn = query.domain_name();
        auto an = query.asset_name();
        if (ln == nullptr || dn == nullptr || an == nullptr) {
          return db->accountGetAllAssets(query.pubKey(), query.uncommitted());
        } else {
          std::vector<const ::iroha::Asset *> res{db->accountGetAsset(
              query.pubKey(), ln, dn, an, query.uncommitted())};
          return res;
        }
    });
  }

    bool existAccountOf(const flatbuffers::String &key) {
        if (db == nullptr) init();
        return false;
    }

    bool checkUserCanPermission(const flatbuffers::String &key) {
        if (db == nullptr) init();

        return false;
    }

    const std::string getMerkleRoot() {
        return "TemporaryString";

        if (db == nullptr) init();
        auto buf = db->getMerkleRoot();
        auto mr = flatbuffers::GetRoot<flatbuffers::String>(buf.data());
        return mr->str();
    }

    namespace permission {

        std::vector<const iroha::AccountPermissionLedger*> getPermissionLedgerOf(const flatbuffers::String &key) {
            if (db == nullptr) init();
            return db->assetGetPermissionLedger(&key);
        }

        std::vector<const iroha::AccountPermissionDomain*> getPermissionDomainOf(const flatbuffers::String &key) {
            if (db == nullptr) init();
            return db->assetGetPermissionDomain(&key);
        }

        std::vector<const iroha::AccountPermissionAsset*> getPermissionAssetOf(const flatbuffers::String &key){
            if (db == nullptr) init();
            return db->assetGetPermissionAsset(&key);
        }

    }

    namespace front_repository{
        void initialize_repository(){
            connection::iroha::AssetRepositoryImpl::AccountGetAsset::receive([=](
                const std::string & /* from */, flatbuffers::unique_ptr_t &&query_ptr) -> std::vector<const ::iroha::Asset *>{
                if(db == nullptr) init();
                const iroha::AssetQuery& query = *flatbuffers::GetRoot<iroha::AssetQuery>(query_ptr.get());
                auto ln = query.ledger_name();
                auto dn = query.domain_name();
                auto an = query.asset_name();
                if(ln == nullptr || dn == nullptr || an == nullptr) {
                    return db->accountGetAllAssets(query.pubKey(), query.uncommitted());
                }else{
                    std::vector<const ::iroha::Asset *> res{db->accountGetAsset(query.pubKey(), ln, dn, an, query.uncommitted())};
                    return res;
                }
            });

            connection::memberShipService::SyncImpl::getTransactions::receive([=](
                    const std::string & /* from */, flatbuffers::unique_ptr_t &&query_ptr) -> std::vector<const ::iroha::Transaction*>{
                if(db == nullptr) init();
                const iroha::Ping& ping = *flatbuffers::GetRoot<iroha::Ping>(query_ptr.get());
                std::vector<const ::iroha::Transaction*> ret;
                size_t index = stoi(ping.message()->str());
                ret.emplace_back( db->getTransaction(index) );
                return ret;
            });
        }
    }
}
};