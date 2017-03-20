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

#include <crypto/signature.hpp>
namespace event {

template <typename T>
class ConsensusEvent: public T {

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

        // move constructor
        eventSignature(eventSignature&& other){
            publicKey = std::move(other.publicKey);
            signature = std::move(other.signature);
        }
    };

    std::vector<eventSignature> _eventSignatures;

public:
    int order;

    template<typename... Args>
    ConsensusEvent(
        Args&&... args
    ):
        T(std::forward<Args>(args)...)
    {}

    void addSignature(const std::string& publicKey, const std::string& signature){
        _eventSignatures.push_back(std::move(eventSignature(publicKey, signature)));
    }

    std::vector<std::tuple<std::string,std::string>> eventSignatures() const{
        std::vector<std::tuple<std::string,std::string>> res(_eventSignatures.size());
        for(const auto& sig: _eventSignatures){
            res.push_back(std::make_tuple(sig.publicKey,sig.signature));
        }
        return res;
    };

    void execution(){
        T::execution();
    }

};
};  // namespace ConsensusEvent

#endif  // CORE_CONSENSUS_CONSENSUSEVENT_HPP_