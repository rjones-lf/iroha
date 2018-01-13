/*
Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.

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
#include <boost/filesystem.hpp>
#include <fstream>
#include <string>
#include "crypto/keys_manager_impl.hpp"

#define PUBKEY_SIZE 32
#define PRIVKEY_SIZE 32

using namespace iroha;
using namespace boost::filesystem;
using namespace std::string_literals;

class KeyManager : public ::testing::Test {
 public:
  bool create_file(const path &ph, const std::string &contents) {
    std::ofstream f(ph.c_str());
    if (not f)
      return false;
    if (not contents.empty())
      f.write(contents.c_str(), contents.size());
    return f.good();
  }

  void SetUp() {
    create_directory(test_dir);
  }

  void TearDown() {
    remove_all(test_dir);
  }

  const path test_dir = "/tmp/iroha";
  const std::string filepath = (test_dir / "keymanager_test_file").string();
  const path pub_key_path = filepath + ".pub";
  const path pri_key_path = filepath + ".priv";
  const std::string pubkey =
      "00576e02f23c8c694c322796cb3ef494829fdf484f4b42312fb7d776fbd5123b"s;
  const std::string prikey =
      "36f028580bb02cc8272a9a020f4200e346e276ae664e45ee80745574e2f5ab80"s;
  KeysManagerImpl manager = KeysManagerImpl(filepath);
  const std::string passphrase = "test";
};

TEST_F(KeyManager, LoadNonExistentKeyFile) {
  ASSERT_FALSE(manager.loadKeys());
}

TEST_F(KeyManager, LoadEmptyFiles) {
  create_file(pub_key_path, "");
  create_file(pri_key_path, "");
  ASSERT_FALSE(manager.loadKeys());
}

TEST_F(KeyManager, LoadInvalid) {
  create_file(pub_key_path, std::string(PUBKEY_SIZE, '1'));
  create_file(pri_key_path, std::string(PRIVKEY_SIZE, '1'));
  ASSERT_FALSE(manager.loadKeys());
}

TEST_F(KeyManager, LoadValid) {
  create_file(pub_key_path, pubkey);
  create_file(pri_key_path, prikey);
  ASSERT_TRUE(manager.loadKeys());
}

TEST_F(KeyManager, CreateAndLoad) {
  ASSERT_TRUE(manager.createKeys(passphrase));
  ASSERT_TRUE(manager.loadKeys());
}

TEST_F(KeyManager, LoadInaccessiblePubkey) {
  create_file(pub_key_path, pubkey);
  create_file(pri_key_path, prikey);
  permissions(pub_key_path, no_perms);
  ASSERT_FALSE(manager.loadKeys());
}

TEST_F(KeyManager, LoadInaccessiblePrikey) {
  create_file(pub_key_path, pubkey);
  create_file(pri_key_path, prikey);
  permissions(pri_key_path, no_perms);
  ASSERT_FALSE(manager.loadKeys());
}
