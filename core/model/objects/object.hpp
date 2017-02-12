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

#ifndef IROHA_OBJECT_H
#define IROHA_OBJECT_H

//#include "../../service/json_parse.hpp"
#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>
#include "../../util/logger.hpp"

#include "asset.hpp"
#include "simple_asset.hpp"
#include "peer.hpp"
#include "message.hpp"
#include "account.hpp"
#include "domain.hpp"

namespace object {

    enum class ObjectValueT : std::uint8_t {
        null,
        simpleAsset,
        asset,
        domain,
        account,
        message,
        peer,
    };

    // There is kind of Currency, Asset,Domain,Account,Message and Peer. Associate SmartContract with Asset.
    struct Object {
        SimpleAsset*    simpleAsset;
        Asset*          asset;
        Domain*         domain;
        Account*        account;
        Message*        message;
        Peer*           peer;

        ObjectValueT type;

        Object();

        Object(const SimpleAsset& rhs);
        Object(const Asset& rhs);
        Object(const Domain& rhs);
        Object(const Account& rhs);
        Object(const Message& rhs);
        Object(const Peer& rhs);

        SimpleAsset* AsSimpleAsset();
        Asset*       AsAsset();
        Domain*      AsDomain();
        Account*     AsAccount();
        Message*     AsMessage();
        Peer*        AsPeer();
    };
}

#endif //IROHA_OBJECT_H
