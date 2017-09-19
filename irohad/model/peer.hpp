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

#ifndef IROHA_PEER_HPP
#define IROHA_PEER_HPP

#include <common/types.hpp>

namespace iroha {
  namespace model {

    /**
     * Peer is Model, which contains information about network participants
     */
    struct Peer {
      /**
       * IP address of peer for connection
       */
      std::string address;

      /**
       * Public key of peer
       */
      ed25519::pubkey_t pubkey;

      using KeyType = decltype(pubkey);

      bool operator==(const Peer &obj) const {
        if (address == obj.address &&
            pubkey == obj.pubkey)
          return true;
        else
          return false;
      };
    };
  }
}

namespace std {
  template<>
  struct hash<iroha::model::Peer> {
    std::size_t operator()(const iroha::model::Peer &obj) const {
      // todo add pubkey hash combination to result
      return std::hash<std::string>()(obj.address);
    }
  };
}
#endif  // IROHA_PEER_H
