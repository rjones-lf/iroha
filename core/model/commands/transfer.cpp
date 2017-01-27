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
#include "transfer.hpp"
#include "../../repository/domain/account_repository.hpp"
#include "../../crypto/hash.hpp"

#include "../../util/logger.hpp"

namespace command {

    /*
        This does moving, migrating, and transferring.
        EX)

        Transfer<Asset>  = Transfer asset value.
        Transfer<Account> =  Permissions migration
    */

    template <>
    void Transfer<object::Account>::execution() {
        logger::debug("Transfer<Account>") << " publicKey:" << object::Account::publicKey << " name:" << object::Account::name;
//        repository::account::add(object::Account::publicKey, object::Account::name);
    }

    template <>
    void Transfer<object::Asset>::execution() {
        auto senderUuid = hash::sha3_256_hex(senderPublicKey);
        auto receiverUuid = hash::sha3_256_hex(receiverPublicKey);
        object::Account sender = repository::account::findByUuid(senderUuid);
        object::Account receiver = repository::account::findByUuid(receiverUuid);

        for(auto& as1: sender.assets){
            if(name == std::get<0>(as1)){
                for(auto& as2: receiver.assets){
                    if(name == std::get<0>(as2)){
                        if(std::get<1>(as1) >= value) {
                            repository::account::update_quantity(
                                    senderUuid, name, std::get<1>(as1) - value
                            );
                            repository::account::update_quantity(
                                    receiverUuid, name, std::get<1>(as2) + value
                            );
                        }
                    }
                }
            }
        }
    }

}
