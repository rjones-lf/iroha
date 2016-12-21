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

#include "event_repository.hpp"

#include <algorithm>
#include <thread>
#include <mutex>
#include <tuple>

std::mutex m;

namespace repository {
    namespace event {

        static std::vector<
            std::tuple<std::string,Event::ConsensusEvent>
        > events;

        bool add(
            const std::string &hash,
            const Event::ConsensusEvent& event
        ){
            logger::debug("repo::event", "event::add");
            std::lock_guard<std::mutex> lock(m);
            events.push_back(std::make_tuple( hash, std::move(event)));
            logger::debug("repo::event", "events size = "+std::to_string(events.size()));
            return true;
        };
        
        bool update(
            const std::string &hash,
            const Event::ConsensusEvent& event
        );
        
        bool addSignature(
            const std::string &hash,
            const std::string &publicKey,
            const std::string &signature
        ){
            /*
            logger::info("repo::event", "event::add");
            std::lock_guard<std::mutex> lock(m);
            for(auto&& event : events){
                if(std::get<0>(event) == hash){
                    std::get<1>(event)->addSignature(publicKey, signature);
                    return true;
                }
            }
            */
            return false;
        }

        bool remove(const std::string &hash);

        bool empty(){
            return events.empty();
        }

        std::vector<
            Event::ConsensusEvent
        > findAll(){
            std::lock_guard<std::mutex> lock(m);
            std::vector<Event::ConsensusEvent> res;
            for(auto&& e : events){
                res.push_back(std::move(std::get<1>(e)));
            }
            events.clear();
            logger::debug("repo::event", "events size = "+std::to_string(res.size()));
            return res;
        };

        Event::ConsensusEvent findNext();

        Event::ConsensusEvent find(std::string hash);

    };
};
