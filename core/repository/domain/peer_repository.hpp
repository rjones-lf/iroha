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

#include <infra/protobuf/api.pb.h>
#include <transaction_builder/transaction_builder.hpp>
#include <string>
#include <vector>

namespace repository {
  namespace peer {
    std::string add(const std::string &publicKey, const std::string &address,
                    const Api::Trust &trust);

    bool update(const std::string &uuid, const std::string &address,
                const Api::Trust &trust);

    bool remove(const std::string &uuid);

//std::vector<Api::Peer> findAll(const std::string &uuid);
    Api::Peer findByUuid(const std::string &uuid);

    bool exists(const std::string &uuid);
  }
}

#endif // __CORE_REPOSITORY_DOMAIN_PEER_REPOSITORY_HPP__
