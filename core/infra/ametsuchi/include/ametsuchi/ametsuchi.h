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

#ifndef AMETSUCHI_DB_H
#define AMETSUCHI_DB_H

#include <ametsuchi/currency.h>
#include <ametsuchi/merkle_tree/merkle_tree.h>
#include <ametsuchi/tx_store.h>
#include <ametsuchi/wsv.h>
#include <commands_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <lmdb.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

extern "C" {
#include <SimpleFIPS202.h>
}

#ifndef AMETSUCHI_MAX_DB_SIZE
#define AMETSUCHI_MAX_DB_SIZE (1024L * 1024 * 1024 * 1024)  // 1 TB
#endif

#ifndef AMETSUCHI_BLOCK_SIZE
#define AMETSUCHI_BLOCK_SIZE (1024)  // the number of leafs in merkle tree
#endif

namespace ametsuchi {


/**
 * Main class for the database.
 *  - single Ametsuchi instance for the single database
 *  - single writer thread
 *  - multiple readers threads, new read-only transaction for each thread
 *  - all data is stored as root flatbuffers
 */
class Ametsuchi {
 public:
  explicit Ametsuchi(const std::string &db_folder);
  ~Ametsuchi();

  /**
   * Append root flatbuffer Transaction to the Ametsuchi database.
   * @throw exception::InvalidTransaction with the reason (one of enum values)
   * @throw exception::InternalError with the reason (one of enum values)
   * @param tx root type Transaction (contents of TransactionWrapper->tx array)
   * @return new merkle root
   */
  // TODO make Flatbuffer vector
  merkle::hash_t append(const std::vector<uint8_t> *tx);
  merkle::hash_t append(const std::vector<std::vector<uint8_t> *> &batch);

  /**
   * Commit appended data to database. Commit creates the latest 'checkpoint',
   * when you can not rollback.
   */
  void commit();

  /**
   * You can rollback appended transaction(s) to previous commit.
   */
  void rollback();

  // ********************
  // Ametsuchi queries:
  /**
 * Returns all assets, which belong to user with \p pubKey.
 * @param pubKey - account's public key
 * @param uncommitted - if true, include uncommitted changes to search.
 * Otherwise create new read-only TX
 * @return 0 or * pairs <pointer, size>, which are mmaped into memory.
 */
  std::vector<const ::iroha::Asset *> accountGetAllAssets(
      const flatbuffers::String *pubKey, bool uncommitted = false);

  /**
   * Returns specific asset, which belong to user with \p pubKey.
   * @param pubKey - account's public key
   * @param ledger_name - ledger name
   * @param domain_name - domain name
   * @param asset_name - asset (currency) name
   * @param uncommitted - if true, include uncommitted changes to search.
 * Otherwise create new read-only TX
   * @return pair <pointer, size>, which are mmaped from disk
   */
  const ::iroha::Asset *accountGetAsset(const flatbuffers::String *pubKey,
                                        const flatbuffers::String *ledger_name,
                                        const flatbuffers::String *domain_name,
                                        const flatbuffers::String *asset_name,
                                        bool uncommitted = false);


  const ::iroha::Asset *assetidGetAsset(const std::string &&ledger_name,
                                        const std::string &&domain_name,
                                        const std::string &&asset_name,
                                        bool uncommitted = false);

  const std::vector<const ::iroha::AccountPermissionLedger*> assetGetPermissionLedger(const flatbuffers::String *pubKey);

  const std::vector<const ::iroha::AccountPermissionDomain*> assetGetPermissionDomain(const flatbuffers::String *pubKey);

  const std::vector<const ::iroha::AccountPermissionAsset*>  assetGetPermissionAsset(const flatbuffers::String *pubKey);

  const ::iroha::Peer *pubKeyGetPeer(const flatbuffers::String *pubKey,
                                     bool uncommitted = false);

  std::vector<AM_val> getAssetTransferBySender(
      const flatbuffers::String *senderKey, bool uncommitted = false);

  std::vector<AM_val> getAssetTransferByReceiver(
      const flatbuffers::String *receiverKey, bool uncommitted = false);

  std::vector<AM_val> getCommandByKey(const flatbuffers::String *pubKey,
                                      iroha::Command command,
                                      bool uncommitted = false);

  const flatbuffers::String* getMerkleRoot();

 private:
  /* for internal use only */

  std::string path_;
  MDB_env *env;
  MDB_stat mst;
  MDB_txn *append_tx_;  // pointer to db transaction

  TxStore tx_store;
  WSV wsv;

  uint32_t AMETSUCHI_TREES_TOTAL;


  void init();

  void init_append_tx();
  void abort_append_tx();
};

}  // namespace ametsuchi

#endif  // AMETSUCHI_DB_H
