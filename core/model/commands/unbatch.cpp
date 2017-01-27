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
#include "unbatch.hpp"
#include "../../repository/domain/account_repository.hpp"
#include "../../crypto/hash.hpp"

#include "../../util/logger.hpp"

namespace command {

    template <>
    void Unbatch<object::Asset>::execution() {
        logger::debug("Unbatch<...>") << "| from publicKey :" << senderPublicKey << " |  -" << object::Asset::value << "-> | to publicKey : " << receiverPublicKey << "| ";
        object::Account sender      = repository::account::findByUuid(hash::sha3_256_hex(senderPublicKey));
        object::Account receiver    = repository::account::findByUuid(hash::sha3_256_hex(receiverPublicKey));

        auto senderUuid = hash::sha3_256_hex(senderPublicKey);
        auto receiverUuid = hash::sha3_256_hex(receiverPublicKey);
        for (auto& as1: sender.assets) {
            if (object::Asset::name == std::get<0>(as1)) {
                for (auto& as2: sender.assets) {
                    if (object::Asset::name == std::get<0>(as2)) {
                        repository::account::update_quantity(
                            senderUuid,object::Asset::name, std::get<1>(as1) - object::Asset::value
                        );
                        repository::account::update_quantity(
                            receiverUuid,object::Asset::name, std::get<1>(as2) + object::Asset::value
                        );
                    }
                }
            }
        }
        
    }

}