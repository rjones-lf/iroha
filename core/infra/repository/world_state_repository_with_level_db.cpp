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

#include <repository/world_state_repository.hpp>
#include <infra/config/iroha_config_with_json.hpp>
#include <util/exception.hpp>
#include <util/logger.hpp>

#include <leveldb/write_batch.h>
#include <leveldb/db.h>
#include <tuple>

// +------------------------------------------------+
// | Repository should save string to any database. |
// +------------------------------------------------+
// |                                                |
// | I know ...                                     |
// |  - leveldb                                     |
// |                                                |
// | I don't know                                   |
// |  - json formats or data model                  |
// |                                                |
// +------------------------------------------------+
namespace repository {

  // Level DB is known only to me.
  namespace world_state_repository {

      namespace detail {

              static leveldb::DB* db = nullptr;

              bool loggerStatus(leveldb::Status const status) {
                  if (!status.ok()) {
                      logger::info("WorldStateRepositoryWithLeveldb") << status.ToString();
                      return false;
                  }
                  return true;
              }

              void loadDb() {
                  leveldb::Options options;
                  options.error_if_exists = false;
                  options.create_if_missing = true;

                  logger::info("WorldStateRepositoryWithLeveldb") << "LoadDB";
                  loggerStatus(leveldb::DB::Open(options,
                        config::IrohaConfigManager::getInstance().getDatabasePath("/tmp/iroha_ledger"),
                        &db
                  ));
              }
      }

      void finish(){
          logger::info("WorldStateRepositoryWithLeveldb") << "finish";
          if (nullptr != detail::db) {
              logger::info("WorldStateRepositoryWithLeveldb") << "delete db pointer";
              delete detail::db;
          }
      }

      bool add(const std::string &key, const std::string &value) {
          if (nullptr == detail::db) {
              detail::loadDb();
          }
          if(nullptr != detail::db) {

              logger::info("WorldStateRepositoryWithLeveldb") << "Add:" << key;
              return detail::loggerStatus(detail::db->Put(leveldb::WriteOptions(), key, value));
          }
          logger::error("WorldStateRepositoryWithLeveldb") << "Error DB already held by process";
          return false;
      }

      template <>
      bool addBatch<std::string>(const std::vector<std::tuple<std::string, std::string>> &tuples){
          if (nullptr == detail::db) {
              detail::loadDb();
          }
          leveldb::WriteBatch batch;

          for (auto&& tuple : tuples) {
              batch.Put(std::get<0>(tuple), std::get<1>(tuple));
          }

          if(nullptr != detail::db) {
              return detail::loggerStatus(detail::db->Write(leveldb::WriteOptions(), &batch));
          }
          logger::error("WorldStateRepositoryWithLeveldb") << "Error DB already held by process";
          return false;
      }

      std::vector<std::string> findAll(){
          logger::info("WorldStateRepositoryWithLeveldb") << "findAll";
          if (nullptr == detail::db) {
              detail::loadDb();
          }
          std::vector<std::string> res;
          leveldb::Iterator* it = detail::db->NewIterator(leveldb::ReadOptions());
          for (it->SeekToFirst(); it->Valid(); it->Next()) {
              res.push_back( it->value().ToString() );
          }
          delete it;
          return res;
      }

      std::vector<std::string> findByPrefix(const std::string& prefix){
          if (nullptr == detail::db) {
              detail::loadDb();
          }
          std::vector<std::string> res;
          leveldb::Iterator* it = detail::db->NewIterator(leveldb::ReadOptions());
          for(it->Seek(prefix);
           it->Valid() && it->key().ToString() < prefix + "~";
           it->Next()
          ){
              res.push_back( it->value().ToString() );
          }
          delete it;
          return res;
      }

      template <typename T>
      bool addBatch(const std::vector<std::tuple<T, T>> &tuples) {
          if (nullptr == detail::db) {
              detail::loadDb();
          }

          leveldb::WriteBatch batch;

          for (auto&& tuple : tuples) {
              batch.Put(std::get<0>(tuple), std::get<1>(tuple));
          }

          return detail::loggerStatus(detail::db->Write(leveldb::WriteOptions(), &batch));
      }

      bool update(const std::string &key, const std::string &value) {
          if (nullptr == detail::db) {
              detail::loadDb();
          }
          if(nullptr != detail::db) {
              std::string dummy;
              if (detail::loggerStatus(detail::db->Get(leveldb::ReadOptions(), key, &dummy))) {
                  leveldb::WriteBatch batch;
                  batch.Delete(key);
                  batch.Put(key, value);
                  return detail::loggerStatus(detail::db->Write(leveldb::WriteOptions(), &batch));
              }
              return false;
          }
          logger::error("WorldStateRepositoryWithLeveldb") << "Error DB already held by process";
          return false;
      }

      bool remove(const std::string &key) {
          if (nullptr == detail::db) {
              detail::loadDb();
          }
          if(nullptr != detail::db) {
              std::string dummy;
              if (detail::loggerStatus(detail::db->Get(leveldb::ReadOptions(), key, &dummy))) {
                  return detail::loggerStatus(detail::db->Delete(leveldb::WriteOptions(), key));
              }
              return false;
          }
          logger::error("WorldStateRepositoryWithLeveldb") << "Error DB already held by process";
          return false;
      }

      std::string find(const std::string &key) {
          if (nullptr == detail::db) {
              detail::loadDb();
          }
          if(nullptr != detail::db) {
              logger::info("WorldStateRepositoryWithLeveldb") << "Find:" << key;
              std::string readData;
              detail::loggerStatus(detail::db->Get(leveldb::ReadOptions(), key, &readData));
              if (!readData.empty()) {
                  return readData;
              } else {
                  return "";
              }
          }
          logger::error("WorldStateRepositoryWithLeveldb") << "Error DB already held by process";
          return "";
      }

      std::string findOrElse(
              const std::string &key,
              const std::string &defaultValue
      ) {
          if (nullptr == detail::db) {
              detail::loadDb();
          }

          std::string result;
          detail::loggerStatus(detail::db->Get(leveldb::ReadOptions(), key, &result));
          if (!result.empty()) {
              return result;
          } else {
              return defaultValue;
          }
      }

      bool exists(const std::string &key) {
          if (nullptr == detail::db) {
              detail::loadDb();
          }

          std::string result;
          detail::loggerStatus(detail::db->Get(leveldb::ReadOptions(), key, &result));
          return !result.empty();
      }
  };
};
