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

#ifndef __IROHA_CONSENSUS_SUMERAGI_HPP__
#define __IROHA_CONSENSUS_SUMERAGI_HPP__

#include <memory>
#include <thread>
#include <vector>

#include <block.pb.cc>

namespace consensus {
    namespace sumeragi {

        void initialize();

        void processBlock(const iroha::protocol::Block &block);

        void panic(const iroha::protocol::Block &block);

    };  // namespace sumeragi
};
#endif  // CORE_CONSENSUS_SUMERAGI_HPP_
