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

#ifndef CORE_CONSENSUS_CONSENSUSEVENT_HPP_
#define CORE_CONSENSUS_CONSENSUSEVENT_HPP_

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <algorithm>

#include "event.hpp"

#include "../crypto/signature.hpp"
#include "../util/logger.hpp"
#include "../model/transaction.hpp"
#include "../service/json_parse.hpp"

namespace event {

template <typename T>
class ConsensusEvent: public T, public Event {

    struct eventSignature{
        std::string publicKey;
        std::string signature;

        eventSignature(
            std::string pubKey,
            std::string sig
        ):
                publicKey(pubKey),
                signature(sig)
        {}
    };
    std::vector<eventSignature> eventSignatures;

public:
    unsigned long long order = 0;

    ConsensusEvent(
        const std::string& senderPubkey,
        const std::string& receiverPubkey,
        const std::string& name,
        const int& value
    );
    ConsensusEvent(
        const std::string& senderPubkey,
        const std::string& domain,
        const std::string& name,
        const unsigned long long& value,
        const unsigned int& precision
    );
    ConsensusEvent(
        const std::string& senderPubkey,
        const std::string& ownerPublicKey,
        const std::string& name
    );


    using Rule = json_parse::Rule;
    using Type = json_parse::Type;
    using Object = json_parse::Object;

    // WIP
    ConsensusEvent(Object obj);

    void addSignature(const std::string& publicKey, const std::string& signature){
        eventSignatures.push_back(eventSignature(publicKey, signature));
    }

    std::string getHash() {
        return T::getHash();
    }

    int getNumValidSignatures() {
        return std::count_if(
            eventSignatures.begin(), eventSignatures.end(),
            [hash = getHash()](eventSignature sig){
                return signature::verify(sig.signature, hash, sig.publicKey);
        });
    }

    bool eventSignatureIsEmpty(){
        return eventSignatures.empty();
    }

    Object dump() {
        Object obj = Object(Type::DICT);
        obj.dictSub.insert( std::make_pair( "order", Object(Type::INT, (int)order)));
        auto eventSigs   = Object(Type::LIST);
        for(auto&& eSig : eventSignatures) {
            auto eventSig = Object(Type::DICT);
            eventSig.dictSub.insert( std::make_pair( "publicKey", Object(Type::STR, eSig.publicKey)));
            eventSig.dictSub.insert( std::make_pair( "signature", Object(Type::STR, eSig.signature)));
            eventSigs.listSub.push_back(eventSig);
        }
        obj.dictSub.insert( std::make_pair( "eventSignatures", eventSigs));
        obj.dictSub.insert( std::make_pair( "transaction", T::dump()));
        return obj;
    }

    static Rule getJsonParseRule() {
        auto rule = Rule(Type::DICT);
        rule.dictSub.insert( std::make_pair( "order", Rule(Type::INT)));
        auto eventSigs  = Rule(Type::LIST);
        auto eventSig   = Rule(Type::DICT);
        eventSig.dictSub.insert( std::make_pair( "publicKey", Rule(Type::STR)));
        eventSig.dictSub.insert( std::make_pair( "signature", Rule(Type::STR)));
        eventSigs.listSub.push_back(eventSig);
        rule.dictSub.insert( std::make_pair( "eventSignatures", eventSigs));
        rule.dictSub.insert( std::make_pair( "transaction", T::getJsonParseRule()));
        return rule;
    }

};
};  // namespace ConsensusEvent

#endif  // CORE_CONSENSUS_CONSENSUSEVENT_HPP_