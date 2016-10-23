
#include "../../../repository/world_state_repository.hpp"
#include "../../../util/exception.hpp"

#include "../../../util/logger.hpp"

#include <leveldb/write_batch.h>
#include <leveldb/db.h>

// +------------------------------------------------+
// | Repository should save string to any database. |
// +------------------------------------------------+
// |                                                |
// | I know ...                                     |
// |  - leveldb                                     |
// |                                                |
// | I don't know                                   |
// |  - json library                                |
// |                                                |
// +------------------------------------------------+
namespace repository{

  // Level DB is known only to me.
  namespace world_state_repository {

      namespace detail {

              static std::unique_ptr<leveldb::DB> db = nullptr;

              bool loggerStatus(leveldb::Status const status) {
                  if (!status.ok()) {
                      logger::info(__FILE__, status.ToString());
                      return false;
                  }
                  return true;
              }

              void loadDb() {
                  leveldb::DB *tmpDb;
                  leveldb::Options options;
                  options.error_if_exists = false;
                  options.create_if_missing = true;
                  loggerStatus(leveldb::DB::Open(options, "irohaDB", &tmpDb)); //TODO: This path should be configurable
                  db.reset(tmpDb);
              }
          }

      bool add(const std::string &key, const std::string &value) {
          if (detail::db == nullptr) detail::loadDb();

          return detail::loggerStatus(detail::db->Put(leveldb::WriteOptions(), key, value));
      }

      bool update(const std::string &key, const std::string &value) {
          if (detail::db == nullptr) detail::loadDb();

          std::string dummy;
          if (detail::loggerStatus(detail::db->Get(leveldb::ReadOptions(), key, &dummy))) {
              leveldb::WriteBatch batch;
              batch.Delete(key);
              batch.Put(key, value);
              return detail::loggerStatus(detail::db->Write(leveldb::WriteOptions(), &batch));
          }
          return false;
      }

      bool remove(const std::string &key) {
          if (detail::db == nullptr) detail::loadDb();
          return detail::loggerStatus(detail::db->Delete(leveldb::WriteOptions(), key));
      }

      std::string find(const std::string &key) {
          if (detail::db == nullptr) detail::loadDb();

          std::string readData;
          detail::loggerStatus(detail::db->Get(leveldb::ReadOptions(), key, &readData));
          if (readData != "") {
              return readData;
          } else {
              return "";
          }
      }

      std::string findOrElse(
              const std::string &key,
              const std::string &defaultValue
      ) {
          if (detail::db == nullptr) detail::loadDb();

          std::string result = "";
          detail::loggerStatus(detail::db->Get(leveldb::ReadOptions(), key, &result));
          if (result == "") {
              return defaultValue;
          } else {
              return result;
          }
      }

      bool isExist(const std::string &key) {
          std::string result = "";
          detail::loggerStatus(detail::db->Get(leveldb::ReadOptions(), key, &result));
          return result == "";
      }
  };
};
