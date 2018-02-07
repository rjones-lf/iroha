/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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
#include <nudb/nudb.hpp>

auto const dat_path = "/tmp/tezt_db.dat";
auto const key_path = "/tmp/tezt_db.key";
auto const log_path = "/tmp/tezt_db.log";

class NuDB : public ::testing::Test {
 public:
  void TearDown() override {
    nudb::erase_file(dat_path);
    nudb::erase_file(key_path);
    nudb::erase_file(log_path);
  }
};

TEST_F(NuDB, CRUD) {
  using key_type = std::uint32_t;
  std::size_t constexpr N = 10;
  nudb::error_code ec;

  // NuDB is a hashmap, and it looks like the same coefficient as "load_factor"
  // for hashmaps.
  float load_factor = 0.5f;
  size_t key_size = sizeof(key_type);  // bytes
  auto salt = nudb::make_salt();
  size_t appnum = 1;
  auto block_size = nudb::block_size(".");

  nudb::create<nudb::xxhasher>(dat_path,
                               key_path,
                               log_path,
                               appnum,
                               salt,
                               key_size,
                               block_size,
                               load_factor,
                               ec);
  nudb::store db;

  db.open(dat_path, key_path, log_path, ec);
  if (ec) {
    FAIL() << "db can not be opened: " << ec.message();
  }

  std::string data = "secret";
  {
    // Insert
    for (key_type i = 0; i < N; ++i) {
      db.insert(&i, data.data(), data.size(), ec);
      if (ec) {
        FAIL() << "key i=" << i << " &i=" << &i
               << " can not be inserted: " << ec.message();
      }
    }
  }
  {
    // Fetch
    for (key_type i = 0; i < N; ++i) {
      db.fetch(&i,
               [&](void const *buffer, std::size_t size) {
                 ASSERT_EQ(size, data.size())
                     << "write and read buffers sizes are different: "
                     << ec.message();

                 for (key_type j = 0; j < size; j++) {
                   ASSERT_EQ(((char *)buffer)[j], data[j]);
                 }
               },
               ec);
      if (ec) {
        FAIL() << "key i=" << i << " &i=" << &i
               << " can not be fetched: " << ec.message();
      }
    }
  }

  {
    key_type a = 1337;
    // try to read non-existent key
    db.fetch(static_cast<const void *>(&a),
             [](const void *p, size_t size) {
               ASSERT_EQ(size, 0) << "size is " << size;
             },
             ec);
    if (ec == nudb::error::key_not_found) {
      SUCCEED();
    } else if (ec) {
      FAIL() << "failed during reading of non-existent key: " << ec.message();
    }
  }

  db.close(ec);
  if (ec) {
    FAIL() << "failed during closing of database: " << ec.message();
  }
}
