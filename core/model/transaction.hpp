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
#ifndef CORE_DOMAIN_TRANSACTIONS_TRANSACTION_HPP_
#define CORE_DOMAIN_TRANSACTIONS_TRANSACTION_HPP_

#include "../crypto/signature.hpp"
#include "../util/datetime.hpp"
#include "../crypto/hash.hpp"
#include <algorithm>
#include <cstdint>

namespace transaction {

template <typename T>
class Transaction: public T {

protected:
    struct txSignature{
        std::string publicKey;
        std::string signature;

        txSignature(
                std::string pubKey,
                std::string sig
        ):
            publicKey(pubKey),
            signature(sig)
        {}
    };

    std::string hash;
    std::vector<txSignature> txSignatures;
public:
    std::int64_t    timestamp;
    std::string     senderPubkey;
    std::string     ownerPublicKey; // okay?

    template<typename... Args>
    Transaction(
        std::string&& senderPublickey,
        Args&&... args
    ):
        T(std::forward<Args>(args)...),
        timestamp(datetime::unixtime()),
        senderPubkey(senderPublickey)
    {}

    Transaction():
        timestamp(datetime::unixtime())
    {}

    void execution(){
        T::execution();
    }

    auto getHash() {
        return hash::sha3_256_hex( T::getCommandName() + std::to_string(timestamp) + senderPubkey);
    }

    std::vector<txSignature> getTxSignatures(){
        return txSignatures;
    }

    void addTxSignature(const std::string& pubKey,const std::string& signature){
        txSignatures.push_back(txSignature(pubKey, signature));
    }

    bool isValidSignatures(){
        return std::count_if(
            txSignatures.begin(), txSignatures.end(),
            [hash = getHash()](txSignature sig){
                return signature::verify(sig.signature, hash, sig.publicKey);
            }
        ) == txSignatures.size();
    }

};

}  // namespace transaction

#endif  // CORE_DOMAIN_TRANSACTIONS_TRANSACTION_HPP_
