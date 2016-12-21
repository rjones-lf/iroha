/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

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

#ifndef CORE_REPOSITORY_UNCONFIRMEDTRANSACTIONREPOSITORY_HPP_
#define CORE_REPOSITORY_UNCONFIRMEDTRANSACTIONREPOSITORY_HPP_

#include <string>
#include <memory>
#include "../../consensus/consensus_event.hpp"

#include "../../infra/protobuf/event.grpc.pb.h"

#include <algorithm>

namespace repository{

    namespace event {

        bool add(const std::string &hash,const Event::ConsensusEvent& event);
 
        bool update(const std::string &hash, const Event::ConsensusEvent& consensusEvent);

        bool remove(const std::string &hash);

        bool addSignature(
            const std::string &hash,
            const std::string &publicKey,
            const std::string &signature
        );

        bool empty();

        std::vector<
            Event::ConsensusEvent
        > findAll();

        Event::ConsensusEvent findNext();

        Event::ConsensusEvent find(std::string hash);
    };
};
#endif  // CORE_REPOSITORY_UNCONFIRMEDTRANSACTIONREPOSITORY_HPP_
