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


#ifndef AMETSUCHI_COMMON_H
#define AMETSUCHI_COMMON_H

#include <ametsuchi/exception.h>
#include <flatbuffers/flatbuffers.h>
#include <lmdb.h>
#include <string>
#include <utility>
#include <vector>

namespace ametsuchi {

extern std::shared_ptr<spdlog::logger> console;

inline std::pair<MDB_dbi, MDB_cursor *> init_btree(
    MDB_txn *append_tx, const std::string &name, uint32_t flags,
    MDB_cmp_func *dupsort = nullptr) {
  int res;
  MDB_dbi dbi;
  MDB_cursor *cursor;
  if ((res = mdb_dbi_open(append_tx, name.c_str(), flags, &dbi)) != 0) {
    AMETSUCHI_CRITICAL(res, MDB_NOTFOUND);
    AMETSUCHI_CRITICAL(res, MDB_DBS_FULL);
  }

  if ((res = mdb_cursor_open(append_tx, dbi, &cursor)) != 0) {
    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  // set comparator for dupsort keys in btree
  if (dupsort != nullptr) {
    if ((res = mdb_set_dupsort(append_tx, dbi, dupsort)) != 0) {
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }

  return std::make_pair(dbi, cursor);
}

/**
 * Represents a value read from a database.
 * Used to prohibit changes of mmaped data by pointer.
 */
struct AM_val {
  // pointer, which points to blob with data
  const void *const data;
  // size of the pointer
  const size_t size;
  explicit AM_val(const MDB_val &a) : data(a.mv_data), size(a.mv_size) {}
};


inline std::vector<std::pair<AM_val, AM_val>> read_all_records(
    MDB_cursor *cursor) {
  MDB_val c_key, c_val;
  int res;

  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_FIRST)) != 0) {
    if (res == MDB_NOTFOUND) {
      return std::vector<std::pair<AM_val, AM_val>>{};
    }
    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  std::vector<std::pair<AM_val, AM_val>> ret;
  do {
    AM_val key(c_key);
    AM_val val(c_val);
    ret.push_back({key, val});

    if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_NEXT)) != 0) {
      if (res == MDB_NOTFOUND) {
        return ret;
      }
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  } while (res == 0);

  return ret;
}
}

#endif  // AMETSUCHI_COMMON_H
