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

//
// Created by SonokoMizuki on 2016/10/23.
//


#include <repository/world_state_repository.hpp>

#include <leveldb/db.h>

#include <gtest/gtest.h>
#include <iostream>
#include <string>

const std::string key = "name";
std::string value = "mizuki";


TEST(World_sate_repository_with_leveldb, AddAndfind){

    repository::world_state_repository::add(key, value);

    std::string res;
    res = repository::world_state_repository::find(key);
    ASSERT_STREQ(res.c_str(), value.c_str());
}


TEST(World_sate_repository_with_leveldb, NotFind){
    repository::world_state_repository::find(key+"++");
}


TEST(World_sate_repository_with_leveldb, Update){
    repository::world_state_repository::add(key, value);
    std::string res;
    repository::world_state_repository::update(key, value + "sonoko");
    res = repository::world_state_repository::find(key);
    ASSERT_STREQ(res.c_str(), (value+"sonoko").c_str());
}

TEST(World_sate_repository_with_leveldb, Remove){
    repository::world_state_repository::add(key, value);
    std::string res;
    repository::world_state_repository::remove(key);
    res = repository::world_state_repository::find(key);
    ASSERT_STREQ(res.c_str(), "");
}


TEST(World_sate_repository_with_leveldb, FindOrElse){
    repository::world_state_repository::add(key, value);
    std::string res;
    res = repository::world_state_repository::findOrElse(key, "iori");
    ASSERT_STREQ(res.c_str(), value.c_str());

    res = repository::world_state_repository::findOrElse(key + "++", "iori");
    ASSERT_STREQ(res.c_str(), "iori");
}

