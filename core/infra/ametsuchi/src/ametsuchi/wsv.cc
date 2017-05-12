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

#include <ametsuchi/comparator.h>
#include <ametsuchi/currency.h>
#include <ametsuchi/wsv.h>
#include <transaction_generated.h>
#include <iostream>

namespace ametsuchi {
__int128_t parse(const flatbuffers::String *str) {
  __int128_t res = 0;
  for (int i = 0; i < str->Length(); i++) {
    if ('0' <= str->Get(i) && str->Get(i) <= '9') {
      res = 10 * res + str->Get(i) - '0';
    }
  }
  if (str->Get(0) == '-') {
    res *= -1;
  }
  return res;
}

const flatbuffers::String *to_String(__int128_t value) {
  flatbuffers::FlatBufferBuilder fbb;
  std::vector<char> buf;
  __uint128_t tmp = value < 0 ? -value : value;
  do {
    buf.push_back((tmp % 10) + '0');
    tmp /= 10;
  } while (tmp != 0);
  if (value < 0) {
    buf.push_back('-');
  }
  std::reverse(buf.begin(), buf.end());
  fbb.CreateString(std::string(std::begin(buf), std::end(buf)));
  return flatbuffers::GetRoot<flatbuffers::String>(
      fbb.GetCurrentBufferPointer());
}

void WSV::init(MDB_txn *append_tx) {
  append_tx_ = append_tx;

  // [pubkey] => assets (DUP)
  trees_["wsv_pubkey_assets"] = init_btree(
      append_tx_, "wsv_pubkey_assets", MDB_DUPSORT | MDB_DUPFIXED | MDB_CREATE,
      comparator::cmp_assets);

  // [pubkey] => account (NODUP)
  trees_["wsv_pubkey_account"] =
      init_btree(append_tx_, "wsv_pubkey_account", MDB_CREATE);

  // [ledger_name+domain_name+asset_name] => creator public key (NODUP)
  trees_["wsv_assetid_asset"] =
      init_btree(append_tx_, "wsv_assetid_asset", MDB_CREATE);

  // [ip] => peer (NODUP)
  trees_["wsv_pubkey_peer"] =
      init_btree(append_tx_, "wsv_pubkey_peer", MDB_CREATE);

  // we should know created assets, so read entire table in memory
  read_created_assets();

  assert(get_trees_total() == trees_.size());
}

void WSV::update(const std::vector<uint8_t> *blob) {
  auto tx = flatbuffers::GetRoot<iroha::Transaction>(blob->data());
  // 4. update WSV
  {
    switch (tx->command_type()) {
      //  Use for operate Asset.
      case iroha::Command::Add: {
        add(tx->command_as_Add());
        break;
      }
      case iroha::Command::Subtract: {
        subtract(tx->command_as_Subtract());
        break;
      }
      case iroha::Command::Transfer: {
        transfer(tx->command_as_Transfer());
        break;
      }
      // Use for meta operate in domain.
      case iroha::Command::AssetCreate: {
        asset_create(tx->command_as_AssetCreate());
        break;
      }
      case iroha::Command::AssetRemove: {
        asset_remove(tx->command_as_AssetRemove());
        break;
      }
      // Use for peer operate
      case iroha::Command::PeerAdd: {
        peer_add(tx->command_as_PeerAdd());
        break;
      }
      case iroha::Command::PeerRemove: {
        peer_remove(tx->command_as_PeerRemove());
        break;
      } /*
       case iroha::Command::PeerSetActive: {
         peer_set_active(tx->command_as_PeerSetActive());
         break;
       }
       case iroha::Command::PeerSetTrust: {
         peer_set_trust(tx->command_as_PeerSetTrust());
         break;
       }
       case iroha::Command::PeerChangeTrust: {
         peer_change_trust(tx->command_as_PeerChangeTrust());
         break;
       }*/
      // Use for account operate
      case iroha::Command::AccountAdd: {
        account_add(tx->command_as_AccountAdd());
        break;
      }
      case iroha::Command::AccountRemove: {
        account_remove(tx->command_as_AccountRemove());
        break;
      }
      /*
      case iroha::Command::AccountAddSignatory: {
        account_add_signatory(tx->command_as_AccountAddSignatory());
        break;
      }
      case iroha::Command::AccountRemoveSignatory: {
        account_remove_signatory(tx->command_as_AccountRemoveSignatory());
        break;
      }
      case iroha::Command::AccountSetUseKeys: {
        account_set_use_keys(tx->command_as_AccountSetUseKeys());
        break;
      }
      case iroha::Command::AccountMigrate: {
        account_migrate(tx->command_as_AccountMigrate());
        break;
      }

      case iroha::Command::ChaincodeAdd: {
        chaincode_add(tx->command_as_ChaincodeAdd());
        break;
      }
      case iroha::Command::ChaincodeRemove: {
        chaincode_remove(tx->command_as_ChaincodeRemove());
        break;
      }
      case iroha::Command::ChaincodeExecute: {
        chaincode_execute(tx->command_as_ChaincodeExecute());
        break;
      }*/
      case iroha::Command::PermissionAdd: {
        permisson_add(tx->command_as_PermissionAdd());
        break;
      }
      case iroha::Command::PermissionRemove: {
        permisson_remove(tx->command_as_PermissionRemove());
        break;
      }
      default: {
        console->critical("Not implemented. Yet.");
        throw exception::InternalError::NOT_IMPLEMENTED;
      }
    }
  }
}
WSV::WSV() {}
WSV::~WSV() {}

void WSV::read_created_assets() {
  auto records = read_all_records(trees_["wsv_assetid_asset"].second);
  created_assets_.clear();
  for (auto &&asset : records) {
    std::string assetid{(char *)asset.first.data,
                        (char *)asset.first.data + asset.first.size};

    std::vector<uint8_t> assetfb{(char *)asset.second.data,
                                 (char *)asset.second.data + asset.second.size};

    created_assets_[assetid] = assetfb;
  }
}


void WSV::close_cursors() {
  for (auto &&e : trees_) {
    MDB_cursor *cursor = e.second.second;
    if (cursor) mdb_cursor_close(cursor);
  }
}

// WSV commands:
//  Use for operate Asset.
void WSV::add(const iroha::Add *command) {
  // Now only Currency is supported
  if (command->asset_nested_root()->asset_type() != iroha::AnyAsset::Currency) {
    // TODO: How to check the asset_type?
    printf("This asset is not an currency \n");
    throw exception::InternalError::NOT_IMPLEMENTED;
  }

  account_add_currency(command->accPubKey(), command->asset());
}

void WSV::subtract(const iroha::Subtract *command) {
  // Now only currency is supported
  if (command->asset_nested_root()->asset_type() != iroha::AnyAsset::Currency)
    throw exception::InternalError::NOT_IMPLEMENTED;

  // TODO: it may write exeption when can't transfer becouse of sender has not
  // asset.
  this->account_subtract_currency(command->accPubKey(), command->asset());
}

void WSV::transfer(const iroha::Transfer *command) {
  // Now only currency is supported
  if (command->asset_nested_root()->asset_type() != iroha::AnyAsset::Currency)
    throw exception::InternalError::NOT_IMPLEMENTED;

  // TODO: it may write exeption when can't transfer becouse of sender has not
  // asset.
  this->account_subtract_currency(command->sender(), command->asset());
  this->account_add_currency(command->receiver(), command->asset());
}


// Use for meta operate in domain
void WSV::asset_create(const iroha::AssetCreate *command) {
  MDB_val c_key, c_val;
  int res;

  auto ln = command->ledger_name();
  auto dn = command->domain_name();
  auto an = command->asset_name();

  // in this order: ledger+domain+asset
  std::string pk;
  pk += ln->data();
  pk += dn->data();
  pk += an->data();

  // create Asset
  // TODO: Now only Currency is supported, not supporte ComplexAsset
  flatbuffers::FlatBufferBuilder fbb;
  auto asset = iroha::CreateAsset(
      fbb, iroha::AnyAsset::Currency,
      iroha::CreateCurrencyDirect(fbb, an->data(), dn->data(), ln->data())
          .Union());
  fbb.Finish(asset);

  auto ptr = fbb.GetBufferPointer();

  c_key.mv_data = (void *)(pk.c_str());
  c_key.mv_size = reinterpret_cast<size_t>(pk.size());
  c_val.mv_data = (void *)ptr;
  c_val.mv_size = fbb.GetSize();

  // Put and sort by assetid
  if ((res = mdb_cursor_put(trees_.at("wsv_assetid_asset").second, &c_key,
                            &c_val, 0))) {
    if (res == MDB_KEYEXIST) {
      throw exception::InvalidTransaction::ASSET_EXISTS;
    }
    AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
    AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
    AMETSUCHI_CRITICAL(res, EACCES);
    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  created_assets_[pk] = std::vector<uint8_t>{ptr, ptr + fbb.GetSize()};
}

void WSV::asset_remove(const iroha::AssetRemove *command) {
  auto cursor = trees_.at("wsv_assetid_asset").second;
  MDB_val c_key, c_val;
  int res;

  std::string pk = command->asset_name()->data();
  pk += command->domain_name()->data();
  pk += command->ledger_name()->data();

  c_key.mv_data = (void *)(pk.c_str());
  c_key.mv_size = reinterpret_cast<size_t>(pk.size());

  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND) {
      throw exception::InvalidTransaction::ASSET_NOT_FOUND;
    }

    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  if ((res = mdb_cursor_del(cursor, 0))) {
    AMETSUCHI_CRITICAL(res, EACCES);
    AMETSUCHI_CRITICAL(res, EINVAL);
  }
  created_assets_.erase(pk);
}


void WSV::account_add_currency(const flatbuffers::String *acc_pub_key,
                               const flatbuffers::Vector<uint8_t> *asset_fb) {
  int res;
  MDB_val c_key, c_val;
  auto cursor = trees_.at("wsv_pubkey_assets").second;
  std::vector<uint8_t> copy;
  const iroha::Currency *currency =
      flatbuffers::GetRoot<iroha::Asset>(asset_fb->Data())->asset_as_Currency();

  try {
    // may throw ASSET_NOT_FOUND
    auto account_asset = accountGetAsset(acc_pub_key, currency->ledger_name(),
                                           currency->domain_name(),
                                           currency->currency_name(), true);
    auto account_currency = account_asset->asset_as_Currency();

    //assert(asset_fb->size() == account_asset.size);

    flatbuffers::FlatBufferBuilder fbb;
    auto copy_asset =
        iroha::CreateAsset(fbb,iroha::AnyAsset::Currency,
                           iroha::CreateCurrency(fbb,fbb.CreateSharedString(account_currency->currency_name()),
                                                 fbb.CreateSharedString(account_currency->domain_name()),
                                                 fbb.CreateSharedString(account_currency->ledger_name()),
                                                 fbb.CreateSharedString(account_currency->description()),
                                                 fbb.CreateSharedString(account_currency->amount()),
                                                 account_currency->precision()).Union());
    fbb.Finish(copy_asset);
    copy = {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()};

    // update the copy
    auto copy_fb = flatbuffers::GetMutableRoot<iroha::Asset>(copy.data());
    auto copy_cur = static_cast<iroha::Currency *>(copy_fb->mutable_asset());

    Currency current(parse(account_currency->amount()), account_currency->precision());
    Currency delta(parse(currency->amount()), currency->precision());
    current = current + delta;



    flatbuffers::FlatBufferBuilder fbb2;
    auto new_amount = fbb2.CreateString(current.to_string(current.get_amount()));
    fbb2.Finish(new_amount);
    *copy_cur->mutable_amount() = *flatbuffers::GetRoot<flatbuffers::String>(fbb2.GetBufferPointer());
    copy_cur->mutate_precision(current.get_precision());

    // write to tree
    c_key.mv_data = (void *)acc_pub_key->data();
    c_key.mv_size = acc_pub_key->size();
    c_val.mv_data = (void *)copy.data();
    c_val.mv_size = copy.size();

    // cursor is at the correct asset, just replace with a copy of FB and flag
    // MDB_CURRENT

    if ((res = mdb_cursor_put(cursor, &c_key, &c_val, MDB_CURRENT))) {
      AMETSUCHI_CRITICAL(res, MDB_KEYEXIST);
      AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
      AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
      AMETSUCHI_CRITICAL(res, EACCES);
      AMETSUCHI_CRITICAL(res, EINVAL);
    }

  } catch (exception::InvalidTransaction e) {
    // Create new Asset
    if (e == exception::InvalidTransaction::ASSET_NOT_FOUND) {
      // write to tree
      c_key.mv_data = (void *)acc_pub_key->data();
      c_key.mv_size = acc_pub_key->size();
      c_val.mv_data = (void *)asset_fb->Data();
      c_val.mv_size = asset_fb->size();

      if ((res = mdb_cursor_put(cursor, &c_key, &c_val, 0))) {
        AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
        AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
        AMETSUCHI_CRITICAL(res, EACCES);
        AMETSUCHI_CRITICAL(res, EINVAL);
      }
    } else {
      throw;
    }
  }
}

void WSV::account_subtract_currency(
    const flatbuffers::String *acc_pub_key,
    const flatbuffers::Vector<uint8_t> *asset_fb) {
  int res;
  MDB_val c_key, c_val;
  auto cursor = trees_.at("wsv_pubkey_assets").second;
  std::vector<uint8_t> copy;
  const iroha::Currency *currency =
      flatbuffers::GetRoot<iroha::Asset>(asset_fb->Data())->asset_as_Currency();

  try {
    // may throw ASSET_NOT_FOUND
    auto account_asset = accountGetAsset(acc_pub_key, currency->ledger_name(),
                                         currency->domain_name(),
                                         currency->currency_name(), true);
    auto account_currency = account_asset->asset_as_Currency();

    //assert(asset_fb->size() == account_asset.size);

    flatbuffers::FlatBufferBuilder fbb;
    auto copy_asset =
        iroha::CreateAsset(fbb, iroha::AnyAsset::Currency,
             iroha::CreateCurrency(fbb, fbb.CreateSharedString(account_currency->currency_name()),
                 fbb.CreateSharedString(account_currency->domain_name()),
                 fbb.CreateSharedString(account_currency->ledger_name()),
                 fbb.CreateSharedString(account_currency->description()),
                 fbb.CreateSharedString(account_currency->amount()),
                 account_currency->precision()).Union()
        );
    fbb.Finish(copy_asset);
    copy = {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()};

    // update the copy
    auto copy_fb = flatbuffers::GetMutableRoot<iroha::Asset>(copy.data());
    auto copy_cur = static_cast<iroha::Currency *>(copy_fb->mutable_asset());

    Currency current(parse(account_currency->amount()), account_currency->precision());
    Currency delta(parse(currency->amount()), currency->precision());
    current = current - delta;


    flatbuffers::FlatBufferBuilder fbb2;
    auto new_amount = fbb2.CreateString(current.to_string(current.get_amount()));
    fbb2.Finish(new_amount);
    *copy_cur->mutable_amount() = *flatbuffers::GetRoot<flatbuffers::String>(fbb2.GetBufferPointer());
    copy_cur->mutate_precision(current.get_precision());

    // cursor is at the correct asset, just replace with a copy of FB and flag
    // MDB_CURRENT
    if ((res = mdb_cursor_put(cursor, &c_key, &c_val, MDB_CURRENT))) {
      AMETSUCHI_CRITICAL(res, MDB_KEYEXIST);
      AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
      AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
      AMETSUCHI_CRITICAL(res, EACCES);
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  } catch (exception::InvalidTransaction e) {
    // Create new Asset
    if (e == exception::InvalidTransaction::ASSET_NOT_FOUND) {
      // Asset Not Found Error ( can't subtract )
      throw;
    } else {
      throw;
    }
  }
}

void WSV::account_add(const iroha::AccountAdd *command) {
  MDB_val c_key, c_val;
  int res;

  auto account =
      flatbuffers::GetRoot<iroha::Account>(command->account()->data());
  auto pubkey = account->pubKey();

  c_key.mv_data = (void *)(pubkey->data());
  c_key.mv_size = pubkey->size();
  c_val.mv_data = (void *)command->account()->data();
  c_val.mv_size = command->account()->size();

  if ((res = mdb_cursor_put(trees_.at("wsv_pubkey_account").second, &c_key,
                            &c_val, 0))) {
    // account with this public key exists
    if (res == MDB_KEYEXIST) {
      throw exception::InvalidTransaction::ACCOUNT_EXISTS;
    }
    AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
    AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
    AMETSUCHI_CRITICAL(res, EACCES);
    AMETSUCHI_CRITICAL(res, EINVAL);
  }
}

void WSV::account_remove(const iroha::AccountRemove *command) {
  MDB_val c_key, c_val;
  int res;

  auto pubkey = command->pubkey();

  c_key.mv_data = (void *)(pubkey->data());
  c_key.mv_size = pubkey->size();

  // move cursor to account in pubkey_account tree
  auto cursor = trees_.at("wsv_pubkey_account").second;
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND)
      throw exception::InvalidTransaction::ACCOUNT_NOT_FOUND;

    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  // remove it
  if ((res = mdb_cursor_del(cursor, 0))) {
    AMETSUCHI_CRITICAL(res, EACCES);
    AMETSUCHI_CRITICAL(res, EINVAL);
  }


  // move cursor to pubkey in pubkey_assets tree
  cursor = trees_.at("wsv_pubkey_assets").second;
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    AMETSUCHI_CRITICAL(res, EINVAL);
    // do not handle MDB_NOTFOUND! it means, that account has no assets
    if (res == MDB_NOTFOUND) return;
  }

  // remove account from tree with assets
  if ((res = mdb_cursor_del(cursor, MDB_NODUPDATA))) {
    AMETSUCHI_CRITICAL(res, EACCES);
    AMETSUCHI_CRITICAL(res, EINVAL);
  }
}

void WSV::peer_add(const iroha::PeerAdd *command) {
  MDB_cursor *cursor = trees_.at("wsv_pubkey_peer").second;
  MDB_val c_key, c_val;
  int res;

  auto peer = flatbuffers::GetRoot<iroha::Peer>(command->peer()->data());
  auto pubkey = peer->publicKey();

  flatbuffers::GetRoot<iroha::Peer>(peer);

  c_key.mv_data = (void *)(pubkey->data());
  c_key.mv_size = pubkey->size();
  c_val.mv_data = (void *)command->peer()->data();
  c_val.mv_size = command->peer()->size();

  if ((res = mdb_cursor_put(cursor, &c_key, &c_val, 0))) {
    // account with this public key exists
    if (res == MDB_KEYEXIST) {
      throw exception::InvalidTransaction::PEER_EXISTS;
    }
    AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
    AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
    AMETSUCHI_CRITICAL(res, EACCES);
    AMETSUCHI_CRITICAL(res, EINVAL);
  }
}

void WSV::peer_remove(const iroha::PeerRemove *command) {
  auto cursor = trees_.at("wsv_pubkey_peer").second;
  MDB_val c_key, c_val;
  int res;

  auto pubkey = command->peerPubKey();

  c_key.mv_data = (void *)(pubkey->data());
  c_key.mv_size = pubkey->size();

  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND) {
      throw exception::InvalidTransaction::PEER_NOT_FOUND;
    }

    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  if ((res = mdb_cursor_del(cursor, 0))) {
    AMETSUCHI_CRITICAL(res, EACCES);
    AMETSUCHI_CRITICAL(res, EINVAL);
  }
}

void WSV::permisson_add(const iroha::PermissionAdd *command) {
  MDB_val c_key, c_val;
  int res;

  auto pubkey = command->targetAccount();
  c_key.mv_data = (void *)(pubkey->data());
  c_key.mv_size = pubkey->size();

  // move cursor to account in pubkey_account tree
  auto cursor = trees_.at("wsv_pubkey_account").second;
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND)
      throw exception::InvalidTransaction::ACCOUNT_NOT_FOUND;

    AMETSUCHI_CRITICAL(res, EINVAL);
  }

    flatbuffers::FlatBufferBuilder fbb;
    auto account = const_cast<iroha::Account*>(flatbuffers::GetRoot<::iroha::Account>(c_val.mv_data));
    switch(command->permission_type()){
        case iroha::AccountPermission::AccountPermissionRoot: {

        }
        case iroha::AccountPermission::AccountPermissionLedger: {
            auto apl = command->permission_as_AccountPermissionLedger();
            std::vector<flatbuffers::Offset<iroha::AccountPermissionLedgerWrapper>> permissons;
            for (const auto& pw : *account->ledgerPermissions()) {
                auto pvec = std::vector<uint8_t>(
                    pw->permission()->begin(),
                    pw->permission()->end()
                );
                permissons.emplace_back(iroha::CreateAccountPermissionLedgerWrapperDirect(fbb, &pvec));
            }
            fbb.Clear();
            fbb.Finish(iroha::CreateAccountPermissionLedgerDirect(fbb,
                apl->ledger_name()->c_str(),
                apl->domain_add(),
                apl->domain_remove(),
                apl->peer_read(),
                apl->peer_write(),
                apl->account_add(),
                apl->account_remove(),
                apl->account_give_permission()
            ));
            auto ptr = fbb.GetBufferPointer();
            std::vector<uint8_t> nested(ptr, ptr + fbb.GetSize());
            permissons.emplace_back(iroha::CreateAccountPermissionLedgerWrapperDirect(fbb, &nested));
            fbb.Clear();
            fbb.CreateVector(permissons);
            *account->mutable_ledgerPermissions() =
                *const_cast<flatbuffers::Vector<
                    flatbuffers::Offset<iroha::AccountPermissionLedgerWrapper>>*>(
                    flatbuffers::GetRoot<flatbuffers::Vector<
                        flatbuffers::Offset<iroha::AccountPermissionLedgerWrapper>
                    >>(fbb.GetBufferPointer())
                );
            break;
        }
        case iroha::AccountPermission::AccountPermissionDomain: {
            auto apd = command->permission_as_AccountPermissionDomain();
            std::vector<flatbuffers::Offset<iroha::AccountPermissionDomainWrapper>> permissons;
            for (const auto& pw : *account->domainPermissions()) {
                auto pvec = std::vector<uint8_t>(
                        pw->permission()->begin(),
                        pw->permission()->end()
                );
                permissons.emplace_back(iroha::CreateAccountPermissionDomainWrapperDirect(fbb, &pvec));
            }
            fbb.Clear();
            fbb.Finish(iroha::CreateAccountPermissionDomainDirect(fbb,
                apd->domain_name()->c_str(),
                apd->ledger_name()->c_str(),
                apd->account_give_permission(),
                apd->account_add(),
                apd->account_remove(),
                apd->asset_create(),
                apd->asset_remove(),
                apd->asset_update()
            ));
            auto ptr = fbb.GetBufferPointer();
            std::vector<uint8_t> nested(ptr, ptr + fbb.GetSize());
            permissons.emplace_back(iroha::CreateAccountPermissionDomainWrapperDirect(fbb, &nested));
            fbb.Clear();
            fbb.CreateVector(permissons);
            *account->mutable_domainPermissions() =
                *const_cast<flatbuffers::Vector<
                    flatbuffers::Offset<iroha::AccountPermissionDomainWrapper>>*>(
                    flatbuffers::GetRoot<flatbuffers::Vector<
                        flatbuffers::Offset<iroha::AccountPermissionDomainWrapper>
                    >>(fbb.GetBufferPointer())
                );
            break;
        }
        case iroha::AccountPermission::AccountPermissionAsset: {
            auto apa = command->permission_as_AccountPermissionAsset();
            std::vector<flatbuffers::Offset<iroha::AccountPermissionAssetWrapper>> permissons;
            for (const auto& pw : *account->assetPermissions()) {
                auto pvec = std::vector<uint8_t>(
                        pw->permission()->begin(),
                        pw->permission()->end()
                );
                permissons.emplace_back(iroha::CreateAccountPermissionAssetWrapperDirect(fbb, &pvec));
            }
            fbb.Clear();
            fbb.Finish(iroha::CreateAccountPermissionAssetDirect(fbb,
                apa->asset_name()->c_str(),
                apa->domain_name()->c_str(),
                apa->ledger_name()->c_str(),
                apa->account_give_permission(),
                apa->transfer(),
                apa->subtract(),
                apa->read()
            ));
            auto ptr = fbb.GetBufferPointer();
            std::vector<uint8_t> nested(ptr, ptr + fbb.GetSize());
            permissons.emplace_back(iroha::CreateAccountPermissionAssetWrapperDirect(fbb, &nested));
            fbb.Clear();
            fbb.CreateVector(permissons);
            *account->mutable_assetPermissions() =
                *const_cast<flatbuffers::Vector<
                    flatbuffers::Offset<iroha::AccountPermissionAssetWrapper>>*>(
                    flatbuffers::GetRoot<flatbuffers::Vector<
                            flatbuffers::Offset<iroha::AccountPermissionAssetWrapper>
                    >>(fbb.GetBufferPointer())
            );
            break;
        }
    }

    if ((res = mdb_cursor_put(trees_.at("wsv_pubkey_account").second, &c_key, &c_val, 0))) {
        // account with this public key exists
        if (res == MDB_KEYEXIST) {
            throw exception::InvalidTransaction::ACCOUNT_EXISTS;
        }
        AMETSUCHI_CRITICAL(res, MDB_MAP_FULL);
        AMETSUCHI_CRITICAL(res, MDB_TXN_FULL);
        AMETSUCHI_CRITICAL(res, EACCES);
        AMETSUCHI_CRITICAL(res, EINVAL);
    }
}


void WSV::permisson_remove(const iroha::PermissionRemove *command) {}

::iroha::Asset *WSV::accountGetAsset(const flatbuffers::String *pubKey,
                                           const flatbuffers::String *ln,
                                           const flatbuffers::String *dn,
                                           const flatbuffers::String *an,
                                           bool uncommitted, MDB_env *env) {
  MDB_val c_key, c_val;
  MDB_cursor *cursor;
  MDB_txn *tx;
  int res;

  std::string pk;

  pk += ln->data();
  pk += dn->data();
  pk += an->data();

  // if given asset exists, then we can get its blob, which consists of {ledger
  // name, domain name and asset name} to speedup read in DUP btree, because we
  // have custom comparator
  auto blob = created_assets_.find(pk);
  if (blob != created_assets_.end()) {
    // asset found, use asset's flatbuffer to find asset in account faster
    c_val.mv_data = (void *)blob->second.data();
    c_val.mv_size = blob->second.size();
  } else {
    throw exception::InvalidTransaction::ASSET_NOT_FOUND;
  }

  // depending on 'uncommitted' we use RO or RW transaction
  if (uncommitted) {
    // reuse existing cursor and "append" transaction
    cursor = trees_.at("wsv_pubkey_assets").second;
    tx = append_tx_;
  } else {
    // create read-only transaction, create new RO cursor
    if ((res = mdb_txn_begin(env, NULL, MDB_RDONLY, &tx))) {
      AMETSUCHI_CRITICAL(res, MDB_PANIC);
      AMETSUCHI_CRITICAL(res, MDB_MAP_RESIZED);
      AMETSUCHI_CRITICAL(res, MDB_READERS_FULL);
      AMETSUCHI_CRITICAL(res, ENOMEM);
    }

    if ((res = mdb_cursor_open(tx, trees_.at("wsv_pubkey_assets").first,
                               &cursor))) {
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }

  // query asset by public key
  c_key.mv_data = (void *)pubKey->data();
  c_key.mv_size = pubKey->size();

  // May be erase commen: check cast currency
  /*
  if( flatbuffers::GetRoot<iroha::Asset>(c_val.mv_data)->asset_as_Currency() ){
    printf("ok c_val as Currency\n");
    printf("size is : %d\n",c_val.mv_size);
  } else
    printf("dame c_val as Currency\n");
  fflush(stdout);
  */

  // if sender has no such asset, then it is incorrect transaction
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_GET_BOTH))) {
    if (res == MDB_NOTFOUND)
      throw exception::InvalidTransaction::ASSET_NOT_FOUND;
    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  if (!uncommitted) {
    mdb_cursor_close(cursor);
    mdb_txn_abort(tx);
  }
  return flatbuffers::GetMutableRoot<::iroha::Asset>(c_val.mv_data);
}

// asset_id is asset_name + domain_name + ledger_name
const ::iroha::Asset *WSV::assetidGetAsset(const std::string &&assetid,
                                           bool uncommitted, MDB_env *env) {
  MDB_val c_key, c_val;
  MDB_cursor *cursor;
  MDB_txn *tx;
  int res;
  std::string tree_name = "wsv_assetid_asset";

  // query peer by public key
  c_key.mv_data = (void *)(assetid.c_str());
  c_key.mv_size = reinterpret_cast<size_t>(assetid.size());

  if (uncommitted) {
    cursor = trees_.at(tree_name).second;
    tx = append_tx_;
  } else {
    // create read-only transaction, create new RO cursor
    if ((res = mdb_txn_begin(env, NULL, MDB_RDONLY, &tx))) {
      AMETSUCHI_CRITICAL(res, MDB_PANIC);
      AMETSUCHI_CRITICAL(res, MDB_MAP_RESIZED);
      AMETSUCHI_CRITICAL(res, MDB_READERS_FULL);
      AMETSUCHI_CRITICAL(res, ENOMEM);
    }

    if ((res = mdb_cursor_open(tx, trees_.at(tree_name).first, &cursor))) {
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }

  // if pubKey is not fount, throw exception
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND) {
      throw exception::InvalidTransaction::ASSET_NOT_FOUND;
    }
    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  if (!uncommitted) {
    mdb_cursor_close(cursor);
    mdb_txn_abort(tx);
  }
  return flatbuffers::GetRoot<::iroha::Asset>(c_val.mv_data);
}

std::vector<const ::iroha::Asset *> WSV::accountGetAllAssets(
    const flatbuffers::String *pubKey, bool uncommitted, MDB_env *env) {
  MDB_val c_key, c_val;
  MDB_cursor *cursor;
  MDB_txn *tx;
  int res;

  // query asset by public key
  c_key.mv_data = (void *)pubKey->data();
  c_key.mv_size = pubKey->size();

  if (uncommitted) {
    cursor = trees_.at("wsv_pubkey_assets").second;
    tx = append_tx_;
  } else {
    // create read-only transaction, create new RO cursor
    if ((res = mdb_txn_begin(env, NULL, MDB_RDONLY, &tx))) {
      AMETSUCHI_CRITICAL(res, MDB_PANIC);
      AMETSUCHI_CRITICAL(res, MDB_MAP_RESIZED);
      AMETSUCHI_CRITICAL(res, MDB_READERS_FULL);
      AMETSUCHI_CRITICAL(res, ENOMEM);
    }

    if ((res = mdb_cursor_open(tx, trees_.at("wsv_pubkey_assets").first,
                               &cursor))) {
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }

  // if sender has no such asset, then it is incorrect transaction
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND) return std::vector<const ::iroha::Asset *>{};
    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  std::vector<const ::iroha::Asset *> ret;
  // account has assets. try to find asset with the same `pk`
  // iterate over account's assets, O(N), where N is number of different
  // assets,
  do {
    // user's current amount
    ret.push_back(flatbuffers::GetRoot<::iroha::Asset>(c_val.mv_data));

    // move to next asset in user's account
    if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_NEXT_DUP))) {
      if (res == MDB_NOTFOUND) return ret;
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  } while (res == 0);

  if (!uncommitted) {
    mdb_cursor_close(cursor);
    mdb_txn_abort(tx);
  }
  return ret;
}

const ::iroha::AccountPermissionRoot WSV::accountGetPermissionRoot(const flatbuffers::String *pubKey){
  //ToDo
}
const std::vector<const ::iroha::AccountPermissionLedger*> WSV::accountGetPermissionLedger(const flatbuffers::String *pubKey){
  MDB_val c_key, c_val;
  std::vector<const ::iroha::AccountPermissionLedger*> permission_vec;
  int res;

  c_key.mv_data = (void *)(pubKey->data());
  c_key.mv_size = pubKey->size();

  auto cursor = trees_.at("wsv_pubkey_account").second;
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND)
      throw exception::InvalidTransaction::ACCOUNT_NOT_FOUND;

    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  flatbuffers::FlatBufferBuilder fbb;
  auto account = flatbuffers::GetRoot<::iroha::Account>(c_val.mv_data);
  for(const auto& pdw: *account->ledgerPermissions()){
    permission_vec.emplace_back(pdw->permission_nested_root());
  }
  return permission_vec;
}

const std::vector<const ::iroha::AccountPermissionDomain*> WSV::accountGetPermissionDomain(const flatbuffers::String *pubKey){
  MDB_val c_key, c_val;
  std::vector<const ::iroha::AccountPermissionDomain*> permission_vec;
  int res;

  c_key.mv_data = (void *)(pubKey->data());
  c_key.mv_size = pubKey->size();

  auto cursor = trees_.at("wsv_pubkey_account").second;
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND)
      throw exception::InvalidTransaction::ACCOUNT_NOT_FOUND;

    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  flatbuffers::FlatBufferBuilder fbb;
  auto account = flatbuffers::GetRoot<::iroha::Account>(c_val.mv_data);
  for(const auto& pdw: *account->domainPermissions()){
    permission_vec.emplace_back(pdw->permission_nested_root());
  }
  return permission_vec;
}

const std::vector<const ::iroha::AccountPermissionAsset*>  WSV::accountGetPermissionAsset(const flatbuffers::String *pubKey){
  MDB_val c_key, c_val;
  std::vector<const ::iroha::AccountPermissionAsset*> permission_vec;
  int res;

  c_key.mv_data = (void *)(pubKey->data());
  c_key.mv_size = pubKey->size();

  auto cursor = trees_.at("wsv_pubkey_account").second;
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND)
      throw exception::InvalidTransaction::ACCOUNT_NOT_FOUND;

    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  flatbuffers::FlatBufferBuilder fbb;
  auto account = flatbuffers::GetRoot<::iroha::Account>(c_val.mv_data);
  for(const auto& pdw: *account->assetPermissions()){
    permission_vec.emplace_back(pdw->permission_nested_root());
  }
  return permission_vec;
}


const ::iroha::Peer *WSV::pubKeyGetPeer(const flatbuffers::String *pubKey,
                                        bool uncommitted, MDB_env *env) {
  MDB_val c_key, c_val;
  MDB_cursor *cursor;
  MDB_txn *tx;
  int res;

  // query peer by public key
  c_key.mv_data = (void *)pubKey->data();
  c_key.mv_size = pubKey->size();

  if (uncommitted) {
    cursor = trees_.at("wsv_pubkey_peer").second;
    tx = append_tx_;
  } else {
    // create read-only transaction, create new RO cursor
    if ((res = mdb_txn_begin(env, NULL, MDB_RDONLY, &tx))) {
      AMETSUCHI_CRITICAL(res, MDB_PANIC);
      AMETSUCHI_CRITICAL(res, MDB_MAP_RESIZED);
      AMETSUCHI_CRITICAL(res, MDB_READERS_FULL);
      AMETSUCHI_CRITICAL(res, ENOMEM);
    }

    if ((res = mdb_cursor_open(tx, trees_.at("wsv_pubkey_peer").first,
                               &cursor))) {
      AMETSUCHI_CRITICAL(res, EINVAL);
    }
  }

  // if pubKey is not fount, throw exception
  if ((res = mdb_cursor_get(cursor, &c_key, &c_val, MDB_SET))) {
    if (res == MDB_NOTFOUND) {
      throw exception::InvalidTransaction::PEER_NOT_FOUND;
    }
    AMETSUCHI_CRITICAL(res, EINVAL);
  }

  if (!uncommitted) {
    mdb_cursor_close(cursor);
    mdb_txn_abort(tx);
  }
  return flatbuffers::GetRoot<::iroha::Peer>(c_val.mv_data);
}

void WSV::close_dbi(MDB_env *env) {
  for (auto &&it : trees_) {
    auto dbi = it.second.first;
    mdb_dbi_close(env, dbi);
  }
}
uint32_t WSV::get_trees_total() {
  wsv_trees_total = 4;
  return wsv_trees_total;
}
}
