//
// Created by SonokoMizuki on 2016/10/23.
//


#include "../../../core/repository/world_state_repository.hpp"

#include <leveldb/db.h>

#include <gtest/gtest.h>
#include <string>

const std::string key = "name";
std::string value = "mizuki";

/*
static std::unique_ptr<leveldb::DB> db = nullptr;

void load_Db() {
    leveldb::DB *tmpDb;
    leveldb::Options options;
    options.error_if_exists = false;
    options.create_if_missing = true;
    leveldb::DB::Open(options, "iroha_test_DB", &tmpDb);
    db.reset(tmpDb);
}
*/
TEST(World_sate_repository_with_leveldb, add){

    repository::world_state_repository::add(key, value);

    std::string res;
    ASSERT_STREQ(res.c_str(), value.c_str());
}


