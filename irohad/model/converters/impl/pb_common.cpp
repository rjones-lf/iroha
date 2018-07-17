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

#include "model/converters/pb_common.hpp"
#include "model/command.hpp"
#include "model/commands/all.hpp"
#include "model/domain.hpp"
#include "qry_responses.pb.h"

namespace iroha {
  namespace model {
    namespace converters {

      protocol::Peer serializePeer(iroha::model::Peer iroha_peer) {
        protocol::Peer res;
        res.set_address(iroha_peer.address);
        res.set_peer_key(iroha_peer.pubkey.data(), iroha_peer.pubkey.size());
        return res;
      }

      iroha::model::Peer deserializePeer(protocol::Peer pb_peer) {
        iroha::model::Peer res;
        res.address = pb_peer.address();
        std::copy(pb_peer.peer_key().begin(),
                  pb_peer.peer_key().end(),
                  res.pubkey.begin());
        return res;
      }

      iroha::protocol::Account serializeAccount(
          const iroha::model::Account &account) {
        iroha::protocol::Account pb_account{};
        pb_account.set_account_id(account.account_id);
        pb_account.set_domain_id(account.domain_id);
        pb_account.set_json_data(account.json_data);
        pb_account.set_quorum(account.quorum);
        return pb_account;
      }

      iroha::protocol::Asset serializeAsset(const iroha::model::Asset &asset) {
        iroha::protocol::Asset pb_asset{};
        pb_asset.set_asset_id(asset.asset_id);
        pb_asset.set_domain_id(asset.domain_id);
        pb_asset.set_precision(asset.precision);
        return pb_asset;
      }

      iroha::protocol::AccountAsset serializeAccountAsset(
          const iroha::model::AccountAsset &account_asset) {
        iroha::protocol::AccountAsset pb_account_asset{};
        pb_account_asset.set_account_id(account_asset.account_id);
        pb_account_asset.set_asset_id(account_asset.asset_id);
        pb_account_asset.set_balance(account_asset.balance);
        return pb_account_asset;
      }

      iroha::protocol::Domain serializeDomain(
          const iroha::model::Domain &domain) {
        iroha::protocol::Domain pb_domain{};
        pb_domain.set_domain_id(domain.domain_id);
        pb_domain.set_default_role(domain.default_role);
        return pb_domain;
      }
    }  // namespace converters
  }    // namespace model
}  // namespace iroha
