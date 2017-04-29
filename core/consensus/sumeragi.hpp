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

#ifndef CORE_CONSENSUS_SUMERAGI_HPP_
#define CORE_CONSENSUS_SUMERAGI_HPP_

#define COMPARATOR(code) [](auto&& l, auto&& r) -> bool { return code; }

#include <memory>
#include <thread>
#include <vector>

namespace iroha {
struct ConsensusEvent;
}

namespace sumeragi {

using iroha::ConsensusEvent;

void initializeSumeragi();
void loop();

void getNextOrder(std::unique_ptr<ConsensusEvent> event);

void processTransaction(flatbuffers::unique_ptr_t&& event);

void panic(const ConsensusEvent& event);

void setAwkTimer(const int sleepMillisecs,
                 const std::function<void(void)> action);
void determineConsensusOrder(/*std::vector<double> trustVector*/);

};  // namespace sumeragi

#endif  // CORE_CONSENSUS_SUMERAGI_HPP_
