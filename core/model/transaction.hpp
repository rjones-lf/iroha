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

#include "../service/json_parse_with_json_nlohmann.hpp"
#include "../crypto/hash.hpp"
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

    std::unique_ptr<T> command;
    std::string hash;
    std::vector<txSignature> txSignatures;

public:

    Transaction(std::unique_ptr<T> command):
        command(std::move(command))
    {}

    std::string getHash() {
        auto parser = json_parse_with_json_nlohman::JsonParse<T>();
        return hash::sha3_256_hex(parser.dump(command->dump()));
    }

    std::string getAsJSON() const{
        return command.getAsJson();
    }

    std::vector<txSignature> getTxSignatures(){
        return txSignatures;
    }

    void addTxSignature(std::string pubKey,std::string signature){
        std::cout <<"+"<< pubKey << std::endl;
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
    json_parse::Object dump() {
        Object obj = Object(Type::DICT);
        auto txSigs   = Object(Type::LIST);
        for(auto&& tSig : txSignatures) {
            auto txSig = Object(Type::DICT);
            txSig.dictSub.insert( std::make_pair( "publicKey", Object(Type::STR, tSig.publicKey)));
            txSig.dictSub.insert( std::make_pair( "signature", Object(Type::STR, tSig.signature)));
            txSigs.listSub.push_back(txSig);
        }
        obj.dictSub.insert( std::make_pair( "txSignatures", txSigs));
        obj.dictSub.insert( std::make_pair( "hash",  Object(Type::STR, getHash())));
        obj.dictSub.insert( std::make_pair( "command", command->dump()));
        return obj;
    }

    static Rule getJsonParseRule() {
        Rule obj = Rule(Type::DICT);
        auto txSigs   = Rule(Type::LIST);
        auto txSig = Rule(Type::DICT);
        txSig.dictSub.insert( std::make_pair( "publicKey", Rule(Type::STR)));
        txSig.dictSub.insert( std::make_pair( "signature", Rule(Type::STR)));
        txSigs.listSub.reset(&txSig);
        obj.dictSub.insert( std::make_pair( "txSignatures", std::move(txSigs)));
        obj.dictSub.insert( std::make_pair( "hash",  Rule(Type::STR)));
        obj.dictSub.insert( std::make_pair( "command", T::getJsonParseRule()));
        return obj;
    }

};

}  // namespace transaction

#endif  // CORE_DOMAIN_TRANSACTIONS_TRANSACTION_HPP_
