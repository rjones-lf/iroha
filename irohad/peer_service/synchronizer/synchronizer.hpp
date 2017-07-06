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

//
// Created by Takumi Yamashita on 2017/04/28.
//

#ifndef IROHA_PEER_SERVICE_SYNCHRONIZER_SYNCHRONIZER_H
#define IROHA_PEER_SERVICE_SYNCHRONIZER_SYNCHRONIZER_H

#include <block.pb.h>
#include <string>

namespace peer_service {
  namespace sync {
    using Block = iroha::protocol::Block;

    bool start();

    // When commit block after consensus, invoke this function
    bool trigger(const Block &);

    enum SYNCHRO_RESULT {
      APPEND_ONGOING,
      APPEND_ERROR,
      APPEND_FINISHED,
    };

    namespace detail {
      bool append_temporary(uint64_t, const Block &&);
      SYNCHRO_RESULT append();
      void appending();
      void clearCache();
    }  // namespace datail

  }  // namespace sync
}  // namespace peer

#endif  // IROHA_PEER_SERVICE_SYNCHRONIZER_SYNCHRONIZER_H
