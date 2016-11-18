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

#include "commands/add.hpp"
#include "commands/transfer.hpp"

#include "../service/json_parse.hpp"
#include "../service/json_parse_with_json_nlohman.hpp"
#include "../crypto/hash.hpp"
#include <algorithm>

namespace transaction {

template <typename T>
class Transaction: public T {

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
    std::string senderPubkey;
public:

    Transaction(
        const std::string& senderPubkey,
        const std::string& receiverPubkey,
        const std::string& name,
        const int& value
    ){}
    Transaction(
        const std::string& senderPubkey,
        const std::string& domain,
        const std::string& name,
        const unsigned long long& value,
        const unsigned int& precision
    ){}
    Transaction(
        const std::string& senderPubkey,
        const std::string& ownerPublicKey,
        const std::string& name
    ){}


    auto getHash() {
        return hash::sha3_256_hex(json_parse_with_json_nlohman::parser::dump(T::dump()));
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


    using Object = json_parse::Object;
    using Rule = json_parse::Rule;
    using Type = json_parse::Type ;

    Object dump() {
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
        obj.dictSub.insert( std::make_pair( "command", T::dump()));
        return obj;
    }

    Rule getJsonParseRule() {
        auto rule   = Rule(Type::DICT);
        auto txSigs = Rule(Type::LIST);
        auto txSig  = Rule(Type::DICT);
        txSig.dictSub.insert( std::make_pair( "publicKey", Rule(Type::STR)));
        txSig.dictSub.insert( std::make_pair( "signature", Rule(Type::STR)));
        txSigs.listSub = std::move(txSig);
        rule.dictSub.insert( std::make_pair( "txSignatures", txSigs));
        rule.dictSub.insert( std::make_pair( "hash",  Rule(Type::STR)));
        rule.dictSub.insert( std::make_pair( "command", T::getJsonParseRule()));
        return rule;
    }

};

}  // namespace transaction

#endif  // CORE_DOMAIN_TRANSACTIONS_TRANSACTION_HPP_
