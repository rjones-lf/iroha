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

#include <string>
#include <memory>
#include "event_repository.hpp"

#include <algorithm>

namespace repository {
    namespace event {

        std::vector<
        std::unique_ptr<consensus_event::ConsensusEvent>
        > consensusEvents;

        bool add(const std::string &hash, std::unique_ptr<consensus_event::ConsensusEvent> event) {
            consensusEvents.push_back(std::move(event));
            return true;
        }

        bool update(const std::string &hash, const consensus_event::ConsensusEvent &consensusEvent) {

        }

        bool remove(const std::string &hash) {
            consensusEvents.erase(
                    std::remove_if(
                            consensusEvents.begin(),
                            consensusEvents.end(),
                            [hash](auto&& event) -> bool {
                                return event->getHash() == hash;
                            }
                    ),
                    consensusEvents.end()
            );
        }

        bool empty() {
            return consensusEvents.empty();
        }

        std::vector<std::unique_ptr<consensus_event::ConsensusEvent>>&& findAll(){
            return std::move(consensusEvents);
        }

        std::unique_ptr<
        consensus_event::ConsensusEvent
        >&& findNext() {
            auto res = std::move(consensusEvents.front());
            consensusEvents.pop_back();
            return std::move(res);
        }

        std::unique_ptr<consensus_event::ConsensusEvent> find(std::string hash) {
            for (auto&& event : consensusEvents){
                if ( event->getHash() == hash) {
                    return std::move(event);
                }
            }
            return std::unique_ptr<consensus_event::ConsensusEvent>(nullptr);
        }
    };
};
