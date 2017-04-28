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

#ifndef AMETSUCHI_COMPARATOR_H
#define AMETSUCHI_COMPARATOR_H

#include <ametsuchi/exception.h>
#include <asset_generated.h>
#include <lmdb.h>
#include <string>

namespace ametsuchi {
namespace comparator {

// MDB_cmp_func
int cmp_assets(const MDB_val* a, const MDB_val* b) {
  auto ac = flatbuffers::GetRoot<iroha::Asset>(a->mv_data)->asset_as_Currency();
  auto bc = flatbuffers::GetRoot<iroha::Asset>(b->mv_data)->asset_as_Currency();

  std::string as;
  as += ac->ledger_name()->data();
  as += ac->domain_name()->data();
  as += ac->currency_name()->data();

  std::string bs;
  bs += bc->ledger_name()->data();
  bs += bc->domain_name()->data();
  bs += bc->currency_name()->data();

  return as.compare(bs);
}

}  // namespace comparator
}  // namespace ametsuchi

#endif  // AMETSUCHI_COMPARATOR_H
