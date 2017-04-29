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
#ifndef AMETSUCHI_WSV_H
#define AMETSUCHI_WSV_H


#include <ametsuchi/common.h>
#include <flatbuffers/flatbuffers.h>
#include <lmdb.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <commands_generated.h>

namespace ametsuchi {

class WSV {
 public:
  WSV();
  ~WSV();

  void update(const std::vector<uint8_t> *blob);

  void init(MDB_txn *append_tx);

  /**
   * Close every cursor used in wsv
   */
  void close_cursors();

  /**
  * Close every dbi used in wsv
  */
  void close_dbi(MDB_env *env);


  // WSV queries:
  AM_val accountGetAsset(const flatbuffers::String *pubKey,
                         const flatbuffers::String *ledger_name,
                         const flatbuffers::String *domain_name,
                         const flatbuffers::String *asset_name,
                         bool uncommitted = false, MDB_env *env = nullptr);

  std::vector<AM_val> accountGetAllAssets(const flatbuffers::String *pubKey,
                                          bool uncommitted = true,
                                          MDB_env *env = nullptr);

  AM_val pubKeyGetPeer(const flatbuffers::String *pubKey,
                       bool uncommitted = false, MDB_env *env = nullptr);

  /*
   * Get total number of trees
   */
  uint32_t get_trees_total();

 private:
  std::unordered_map<std::string, std::pair<MDB_dbi, MDB_cursor *>> trees_;
  MDB_txn *append_tx_;

  uint32_t wsv_trees_total;

  // [ledger+domain+asset] => ComplexAsset/Currency flatbuffer (without amount)
  std::unordered_map<std::string, std::vector<uint8_t>> created_assets_;

  void read_created_assets();

  // WSV commands:
  void asset_create(const iroha::AssetCreate *command);
  void asset_add(const iroha::AssetAdd *command);
  void asset_remove(const iroha::AssetRemove *command);
  void asset_transfer(const iroha::AssetTransfer *command);
  void account_add(const iroha::AccountAdd *command);
  void account_remove(const iroha::AccountRemove *command);
  void peer_add(const iroha::PeerAdd *command);
  void peer_remove(const iroha::PeerRemove *command);
  // manipulate with account's assets using these functions
  void account_add_currency(const flatbuffers::String *acc_pub_key,
                            const flatbuffers::Vector<uint8_t> *asset_fb);
  void account_remove_currency(const flatbuffers::String *acc_pub_key,
                               const flatbuffers::Vector<uint8_t> *asset_fb);
};
}

#endif  // AMETSUCHI_WSV_H
