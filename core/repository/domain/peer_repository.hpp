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

#ifndef __CORE_REPOSITORY_DOMAIN_PEER_REPOSITORY_HPP__
#define __CORE_REPOSITORY_DOMAIN_PEER_REPOSITORY_HPP__

#include <string>
#include <memory>
#include <vector>

#include "../../model/objects/peer.hpp"

namespace repository{
    namespace peer {

        // SampleAsset has only quantity no logic, so this value is int.
        bool update_quantity(
            const std::string& uuid,
            const std::string& assetName,
            long newValue
        );

        object::Peer findByUuid(const std::string& uuid);

        bool add(
            std::string &ip,
            std::string &publicKey
        );

    };
};
#endif  // CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
