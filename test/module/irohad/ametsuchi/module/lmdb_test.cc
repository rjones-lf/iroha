/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include <ametsuchi/ametsuchi.h>
#include <gtest/gtest.h>
#include <lmdb.h>
#include <spdlog/spdlog.h>

static auto console = spdlog::stdout_color_mt("lmdb");

#define HANDLE(res)                                      \
  {                                                      \
    if (res) console->critical("{}", mdb_strerror(res)); \
  }

class LMDB_Test : public ::testing::Test {
 protected:
  virtual void TearDown() {
    remove((lmdb + "data.mdb").c_str());
    remove((lmdb + "lock.mdb").c_str());
    system(("rm -rf " + lmdb).c_str());
  }

  std::string lmdb = "/tmp/ametsuchi";

  LMDB_Test() {
    int res;

    HANDLE(mdb_env_create(&env));
    HANDLE(mdb_env_set_mapsize(env, 1024L * 1024 * 2));
    HANDLE(mdb_env_set_maxdbs(env, 3));  // we have only 3 databases

    // create index directory
    res = mkdir(lmdb.c_str(), 0700);
    if (res == -1) {
      if (errno != EEXIST) {
        console->critical("can not create {} folder", lmdb);
        exit(res);
      }
    }

    HANDLE(mdb_env_open(env, lmdb.c_str(), MDB_FIXEDMAP, 0700));
  }

  void open_tx() {
    int res;
    HANDLE(mdb_txn_begin(env, NULL, 0, &append_tx));
    HANDLE(mdb_dbi_open(append_tx, "TEST1",
                        MDB_DUPSORT | MDB_DUPFIXED | MDB_CREATE, &dbi_index1));
    HANDLE(mdb_cursor_open(append_tx, dbi_index1, &cursor_1));
  }

  void commit_tx() {
    mdb_cursor_close(cursor_1);
    HANDLE(mdb_txn_commit(append_tx));
    HANDLE(mdb_env_stat(env, &mst));
  }

  MDB_env *env;
  MDB_stat mst;
  MDB_txn *append_tx;
  MDB_dbi dbi_index1;
  MDB_cursor *cursor_1;
};


TEST_F(LMDB_Test, LMDB_Dup_update){

  open_tx();

  MDB_val key;
  key.mv_data = (void *)"helloworld";
  key.mv_size = 10;

  MDB_val val1;

  int a = 2;
  val1.mv_data = &a;
  val1.mv_size = sizeof(a);

  HANDLE(mdb_cursor_put(cursor_1, &key, &val1, 0));

  MDB_val val2;
  int b = 3;
  val2.mv_data = &b;
  val2.mv_size = sizeof(b);

  HANDLE(mdb_cursor_put(cursor_1, &key, &val2, 0));
  commit_tx();

  // retrive
  MDB_val val;

  open_tx();

  // We want to replace element that has key and value val1
  HANDLE(mdb_cursor_get(cursor_1, &key, &val1, MDB_GET_BOTH));

  auto a_retrived = *reinterpret_cast<int *>(val1.mv_data);
  ASSERT_EQ(a, a_retrived);

  // Update current data:
  int new_a = 5;
  MDB_val new_val1;
  new_val1.mv_data = &new_a;
  new_val1.mv_size = sizeof(new_a);

  // Remove current data
  HANDLE(mdb_cursor_del(cursor_1, 0));
  // Put a new value, and sort it
  HANDLE(mdb_cursor_put(cursor_1, &key, &new_val1, 0));

  commit_tx();


  // Read only transaction:
  MDB_txn *read_tx;
  MDB_dbi read_dbi;
  MDB_cursor *read_cursor;

  HANDLE(mdb_txn_begin(env, NULL, MDB_RDONLY, &read_tx));
  HANDLE(mdb_dbi_open(read_tx, "TEST1", MDB_DUPSORT | MDB_DUPFIXED, &read_dbi));

  HANDLE(mdb_cursor_open(read_tx, read_dbi, &read_cursor));
  HANDLE(mdb_cursor_get(read_cursor, &key, &val, MDB_SET));
  a_retrived = *reinterpret_cast<int *>(val.mv_data);
  ASSERT_EQ(b, a_retrived);

  HANDLE(mdb_cursor_get(read_cursor, &key, &val, MDB_NEXT_DUP));
  a_retrived = *reinterpret_cast<int *>(val.mv_data);

  ASSERT_EQ(new_a, a_retrived);
}
