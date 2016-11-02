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

#include "commands/command.hpp"

#include "../service/json_parse.hpp"

#include <algorithm>

namespace transaction {

enum class TransactionType {
    addPeer = 0,
    modifyPeer = 1,
    removePeer = 2,
    transfer = 3,
    signatory = 4,
    signatoryAdd = 5,
    signatoryDelete = 6,
    domainDefinition = 7,
    domainRenewal = 8,
    aliasDefinition = 9,
    aliasRenewal = 10,
    assetDefinition = 11,
    message = 12,
    chaincodeInit = 13,
    chaincodeInvoke = 14,
    chaincodeUpdate  = 15,
    chaincodeDestroy = 16,
    interchain = 17
};

template<typename T,
    std::enable_if_t<
    std::is_base_of<command::Command, T>::value,std::nullptr_t
    > = nullptr
>
class Transaction {

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

    T command;
    std::string hash;
    std::vector<txSignature> txSignatures;

public:

    Transaction(T command):
        command(command)
    {}

    std::string getHash() const{
        return hash;
    }

    std::string getAsJSON() const{
        return command.getAsJson();
    }

    std::vector<txSignature> getTxSignatures(){
        return txSignatures;
    }

    void addTxSignature(std::string pubKey,std::string signature){
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


    using Object = json_parse::Object;
    using Rule = json_parse::Rule;
    using Type = json_parse::Type ;
    Object dump() {
        Object obj = Object(Type::DICT);
        auto txSigs   = Object(Type::LIST);
        for(auto&& tSig : txSignatures) {
            auto txSig = Object(Type::DICT);
            txSig.dictSub["publicKey"] = Object(Type::STR, tSig.push_back(tSig.publicKey));
            txSig.dictSub["signature"] = Object(Type::STR, tSig.push_back(tSig.signature));
            txSigs.listSub.push_back(txSig);
        }
        obj.dictSub["txSignatures"] = txSigs;
        obj.dictSub["hash"] =  Object(Type::STR, hash);
        obj.dictSub["command"] = command.getJsonParseRule();
        return obj;
    }

    static Rule getJsonParseRule() {
        Rule obj = Rule(Type::DICT);
        auto txSigs   = Rule(Type::LIST);
        auto txSig = Rule(Type::DICT);
        txSig.dictSub["publicKey"] = Rule(Type::STR);
        txSig.dictSub["signature"] = Rule(Type::STR);
        txSigs.listSub = txSig;
        obj.dictSub["txSignatures"] = txSigs;
        obj.dictSub["hash"] =  Rule(Type::STR);
        obj.dictSub["command"] = T::getJsonParseRule();
        return obj;
    }

};

}  // namespace transaction

#endif  // CORE_DOMAIN_TRANSACTIONS_TRANSACTION_HPP_
