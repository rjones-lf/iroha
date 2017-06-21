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
#ifndef IROHA_PEER_H
#define IROHA_PEER_H

#include <common.hpp>

namespace iroha {
  namespace dao {

    /**
     * Peer is DAO, which contains information about network participants
     */
    struct Peer {

      /**
       * Address of peer for connection
       */
      const std::string address;

      /**
       * Public key of peer
       */
      const iroha::ed25519::pubkey_t pubkey;
    };
  }
}
#endif //IROHA_PEER_H
