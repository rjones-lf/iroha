/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

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
#include "remove.hpp"

#include "../objects/account.hpp"
#include "../objects/asset.hpp"
#include "../objects/peer.hpp"

#include "../../util/logger.hpp"

#include "../../service/peer_service.hpp"
#include "../../infra/config/peer_service_with_json.hpp"

namespace command {

    template <>
    void Remove<object::Account>::execution() {
        logger::debug("Remove<Account>") << "save publicKey:" << object::Account::publicKey << " name:" << object::Account::name;
        //repository::account::remove(object::Account::publicKey, object::Account::name);  // replace ametsuchi
    }

    template <>
    void Remove<object::Asset>::execution() {

    }

    template<>
    void Remove<object::Peer>::execution() {
        logger::debug("Remove<Peer>") << "save ip:" << object::Peer::getIP() << " publicKey:" << object::Peer::getPublicKey();

        // delete all database
        // TODO

        // remove peerList
        config::PeerServiceConfig::getInstance().removePeer( object::Peer::getInstance() );
    }
}