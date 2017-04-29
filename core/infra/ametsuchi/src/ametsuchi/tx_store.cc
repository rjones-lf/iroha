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

#include <ametsuchi/exception.h>
#include <ametsuchi/tx_store.h>
#include <asset_generated.h>
#include <transaction_generated.h>

namespace ametsuchi {


merkle::hash_t TxStore::append(const std::vector<uint8_t> *blob) {
  auto tx = flatbuffers::GetRoot<iroha::Transaction>(blob->data());

  MDB_val c_key, c_val;
  int res;
  // 1. append TX in the end of TX STORE
  {
    c_key.mv_data = &(++tx_store_total);
    c_key.mv_size = sizeof(tx_store_total);
    c_val.mv_data = (void *)blob->data();
    c_val.mv_size = blob->size();

    if ((res = mdb_cursor_put(trees_.at("tx_store").second, &c_key, &c_val,
                              MDB_NOOVERWRITE | MDB_APPEND)) != 0) {
      AMETSUCHI_CRITICAL(res, MDB_KEYEXIST);
      AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
      AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
      AMETSUCHI_CRITICAL(res, EACCES);
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }

  // 2. insert record into index depending on the command
  {
    auto creator = tx->creatorPubKey();
    switch (tx->command_type()) {
      case iroha::Command::AssetCreate:
        put_tx_into_tree_by_key(trees_.at("index_asset_create").second, creator,
                                tx_store_total);
        break;
      case iroha::Command::AssetAdd:
        put_tx_into_tree_by_key(trees_.at("index_asset_add").second, creator,
                                tx_store_total);
      case iroha::Command::AssetRemove:
        put_tx_into_tree_by_key(trees_.at("index_asset_remove").second, creator,
                                tx_store_total);
      case iroha::Command::AssetTransfer:
        put_tx_into_tree_by_key(trees_.at("index_asset_transfer").second,
                                creator, tx_store_total);
        break;
      case iroha::Command::AccountAdd:
        put_tx_into_tree_by_key(trees_.at("index_account_add").second, creator,
                                tx_store_total);
        break;
      case iroha::Command::AccountAddSignatory:
        put_tx_into_tree_by_key(trees_.at("index_account_add_sign").second,
                                creator, tx_store_total);
        break;
      case iroha::Command::AccountRemove:
        put_tx_into_tree_by_key(trees_.at("index_account_remove").second,
                                creator, tx_store_total);
        break;
      case iroha::Command::AccountRemoveSignatory:
        put_tx_into_tree_by_key(trees_.at("index_account_remove_sign").second,
                                creator, tx_store_total);
        break;
      case iroha::Command::AccountSetUseKeys:
        put_tx_into_tree_by_key(trees_.at("index_account_set_use_keys").second,
                                creator, tx_store_total);
        break;
      case iroha::Command::PeerAdd:
        put_tx_into_tree_by_key(trees_.at("index_peer_add").second, creator,
                                tx_store_total);
        break;
      case iroha::Command::PeerChangeTrust:
        put_tx_into_tree_by_key(trees_.at("index_peer_change_trust").second,
                                creator, tx_store_total);
        break;
      case iroha::Command::PeerRemove:
        put_tx_into_tree_by_key(trees_.at("index_peer_remove").second, creator,
                                tx_store_total);
        break;
      case iroha::Command::PeerSetActive:
        put_tx_into_tree_by_key(trees_.at("index_peer_set_active").second,
                                creator, tx_store_total);
        break;
      case iroha::Command::PeerSetTrust:
        put_tx_into_tree_by_key(trees_.at("index_peer_set_trust").second,
                                creator, tx_store_total);
        break;
      default:
        throw exception::InvalidTransaction::WRONG_COMMAND;
    }
  }

  // 3. insert record into index_transfer_sender and index_transfer_receiver
  if (tx->command_type() == iroha::Command::AssetTransfer) {
    // update index_transfer_sender
    auto cmd = tx->command_as_AssetTransfer();
    c_key.mv_data = (void *)(cmd->sender()->data());
    c_key.mv_size = cmd->sender()->size();

    if ((res = mdb_cursor_put(trees_.at("index_transfer_sender").second, &c_key,
                              &c_val, 0)) != 0) {
      AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
      AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
      AMETSUCHI_CRITICAL(res, EACCES);
      AMETSUCHI_CRITICAL(res, EINVAL);
    }

    // update index_transfer_receiver
    c_key.mv_data = (void *)(cmd->receiver()->data());
    c_key.mv_size = cmd->receiver()->size();

    if ((res = mdb_cursor_put(trees_.at("index_transfer_receiver").second,
                              &c_key, &c_val, 0)) != 0) {
      AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
      AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
      AMETSUCHI_CRITICAL(res, EACCES);
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }

  // 4. Push to merkle tree
  merkle::hash_t h;
  assert(tx->hash()->size() == merkle::HASH_LEN);
  std::copy(tx->hash()->begin(), tx->hash()->end(), &h[0]);
  merkleTree_.push(h);
  return merkleTree_.root();
}

void TxStore::init(MDB_txn *append_tx) {
  append_tx_ = append_tx;

  // autoincrement_key => tx (NODUP)
  create_new_tree(append_tx_, "tx_store", MDB_CREATE | MDB_INTEGERKEY);
  create_new_tree(append_tx_, "merkle_tree", MDB_CREATE | MDB_INTEGERKEY);

  auto name_set = {
      "index_asset_create",         "index_asset_add",
      "index_asset_remove",         "index_asset_transfer",
      "index_transfer_sender",      "index_transfer_receiver",
      "index_account_add",          "index_account_add_sign",
      "index_account_remove",       "index_account_remove_sign",
      "index_account_set_use_keys", "index_peer_add",
      "index_peer_change_trust",    "index_peer_remove",
      "index_peer_set_active",      "index_peer_set_trust",
  };

  // TxStore trees: [pubkey] => [autoincrement_key] (DUP)
  for (auto name : name_set) {
    create_new_tree(append_tx_, name, MDB_DUPSORT | MDB_DUPFIXED | MDB_CREATE);
  }

  set_tx_total();
  assert(get_trees_total() == trees_.size());
}

void TxStore::close_cursors() {
  for (auto &&e : trees_) {
    MDB_cursor *cursor = e.second.second;
    if (cursor != nullptr) {
      mdb_cursor_close(cursor);
    }
  }
}

TxStore::TxStore(size_t merkle_leaves) : merkleTree_(merkle_leaves) {}

TxStore::~TxStore() = default;

void TxStore::set_tx_total() {
  MDB_val c_key, c_val;
  int res;

  MDB_cursor *cursor = trees_.at("tx_store").second;

  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_LAST)) != 0) {
    if (res == MDB_NOTFOUND) {
      tx_store_total = 0u;
    }
    AMETSUCHI_CRITICAL(res, EINVAL);
  } else {
    tx_store_total = *reinterpret_cast<size_t *>(c_key.mv_data);
  }
}
void TxStore::close_dbi(MDB_env *env) {
  for (auto &&it : trees_) {
    auto dbi = it.second.first;
    mdb_dbi_close(env, dbi);
  }
}
uint32_t TxStore::get_trees_total() {
  TX_STORE_TREES_TOTAL = 18;
  return TX_STORE_TREES_TOTAL;
}


void TxStore::put_tx_into_tree_by_key(MDB_cursor *cursor,
                                      const flatbuffers::String *acc_pub_key,
                                      size_t &tx_store_total) {
  MDB_val c_key, c_val;
  int res;

  c_key.mv_data = (void *)(acc_pub_key->data());
  c_key.mv_size = acc_pub_key->size();
  c_val.mv_data = &tx_store_total;
  c_val.mv_size = sizeof(tx_store_total);

  if ((res = mdb_cursor_put(cursor, &c_key, &c_val, 0)) != 0) {
    AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
    AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
    AMETSUCHI_CRITICAL(res, EACCES);
    AMETSUCHI_CRITICAL(res, EINVAL);
  }
}


std::vector<AM_val> TxStore::getTxByKey(const std::string &tree_name,
                                        const flatbuffers::String *pubKey,
                                        bool uncommitted, MDB_env *env) {
  MDB_val c_key, c_val;
  MDB_cursor *cursor;
  MDB_txn *tx;
  int res;

  // query asset by public key
  c_key.mv_data = (void *)pubKey->data();
  c_key.mv_size = pubKey->size();

  if (uncommitted) {
    cursor = trees_.at(tree_name).second;
    tx = append_tx_;
  } else {
    // create read-only transaction, create new RO cursor
    if ((res = mdb_txn_begin(env, nullptr, MDB_RDONLY, &tx)) != 0) {
      AMETSUCHI_CRITICAL(res, MDB_PANIC);
      AMETSUCHI_CRITICAL(res, MDB_MAP_RESIZED);
      AMETSUCHI_CRITICAL(res, MDB_READERS_FULL);
      AMETSUCHI_CRITICAL(res, ENOMEM);
    }

    if ((res = mdb_cursor_open(tx, trees_.at(tree_name).first, &cursor)) != 0) {
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }

  // if sender has no such tx, then it is pub_key
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET)) != 0) {
    if (res == MDB_NOTFOUND) {
      return std::vector<AM_val>{};
    }
    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  std::vector<AM_val> ret;
  // index tree has transactions. try to find asset with the same `pk`
  // iterate over creator's transactions, O(N), where N is number of different
  // transactions,
  MDB_val tx_key, tx_val;
  MDB_cursor *tx_cursor;

  if (uncommitted) {
    tx_cursor = trees_.at("tx_store").second;
  } else {
    if ((res = mdb_cursor_open(tx, trees_.at("tx_store").first, &tx_cursor)) !=
        0) {
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }

  do {
    tx_key = c_val;
    if ((res = mdb_cursor_get(tx_cursor, &tx_key, &tx_val, MDB_FIRST)) != 0) {
      AMETSUCHI_CRITICAL(res, MDB_NOTFOUND);
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
    ret.push_back(AM_val(tx_val));
    if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_NEXT_DUP)) != 0) {
      if (res == MDB_NOTFOUND) {
        break;
      }
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  } while (res == 0);

  if (!uncommitted) {
    mdb_cursor_close(cursor);
    mdb_cursor_close(tx_cursor);
    mdb_txn_abort(tx);
  }

  return ret;
}

void TxStore::create_new_tree(MDB_txn *append_tx, const std::string &name,
                              uint32_t flags, MDB_cmp_func *dupsort) {
  trees_[name] = init_btree(append_tx, name, flags, dupsort);
}

std::vector<AM_val> TxStore::getAssetTransferBySender(
    const flatbuffers::String *senderKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_transfer_sender", senderKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAssetTransferByReceiver(
    const flatbuffers::String *receiverKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_transfer_receiver", receiverKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAssetCreateByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_asset_create", pubKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAssetAddByKey(const flatbuffers::String *pubKey,
                                              bool uncommitted, MDB_env *env) {
  return getTxByKey("index_asset_add", pubKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAssetRemoveByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_asset_remove", pubKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAssetTransferByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_asset_transfer", pubKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAccountAddByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_account_add", pubKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAccountAddSignByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_account_add_sign", pubKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAccountRemoveByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_account_remove", pubKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAccountRemoveSignByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_account_remove_sign", pubKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getAccountSetUseKeysByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_account_set_use_keys", pubKey, uncommitted, env);
}

std::vector<AM_val> TxStore::getPeerAddByKey(const flatbuffers::String *pubKey,
                                             bool uncommitted, MDB_env *env) {
  return getTxByKey("index_peer_add", pubKey, uncommitted, env);
}
std::vector<AM_val> TxStore::getPeerChangeTrustByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_peer_change_trust", pubKey, uncommitted, env);
}
std::vector<AM_val> TxStore::getPeerRemoveByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_peer_remove", pubKey, uncommitted, env);
}
std::vector<AM_val> TxStore::getPeerSetActiveByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_peer_set_active", pubKey, uncommitted, env);
}
std::vector<AM_val> TxStore::getPeerSetTrustByKey(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  return getTxByKey("index_peer_set_trust", pubKey, uncommitted, env);
}
merkle::hash_t TxStore::merkle_root() { return merkleTree_.root(); }

void TxStore::commit() {
  int res;
  MDB_val c_key, c_val;

  // Clear old hashes
  if ((res = mdb_drop(append_tx_, trees_.at("merkle_tree").first, 0))) {
    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  auto last_block = merkleTree_.last_block();
  auto begin = merkleTree_.last_block_begin(),
       end = merkleTree_.last_block_end();

  for (; begin < end; ++begin) {
    c_key.mv_data = (void *)&begin;
    c_key.mv_size = sizeof(begin);
    c_val.mv_data = (void *)last_block.at(begin).data();
    c_val.mv_size = merkle::HASH_LEN;

    if ((res = mdb_cursor_put(trees_.at("merkle_tree").second, &c_key, &c_val,
                              MDB_APPEND))) {  // ???
      AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
      AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
      AMETSUCHI_CRITICAL(res, EACCES);
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }
}
void TxStore::init_merkle_tree() {
  auto records = read_all_records(trees_.at("merkle_tree").second);
  for (auto &record : records) {
    merkle::hash_t hash;
    assert(record.second.size == merkle::HASH_LEN);
    std::copy(
        static_cast<const uint8_t *>(record.second.data),
        static_cast<const uint8_t *>(record.second.data) + record.second.size,
        hash.data());
    merkleTree_.push(hash);
    assert((merkleTree_.last_block_end() - 1) == *(size_t *)record.first.data);
  }
}
}
