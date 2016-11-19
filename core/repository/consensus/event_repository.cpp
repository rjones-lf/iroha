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
#include "../../service/json_parse.hpp"

std::mutex m;

namespace repository {
    namespace event {

        static std::vector<std::unique_ptr<::event::Event>> events;

        bool add(
            const std::string &hash,
            std::unique_ptr<::event::Event> event
        ){
            logger::info("repo::event", "event::add");
            std::lock_guard<std::mutex> lock(m);
            events.push_back(std::move(event));
            logger::info("repo::event", "events size = "+std::to_string(events.size()));
            return true;
        };
        
        bool update(
            const std::string &hash,
            std::unique_ptr<::event::Event> event
        );

        bool remove(const std::string &hash);

        bool empty(){
            return events.empty();
        }

        std::vector<
            std::unique_ptr<::event::Event>
        >&& findAll(){
            std::lock_guard<std::mutex> lock(m);
            std::vector<
                std::unique_ptr<::event::Event>
            > res;
            for(auto&& e : events){
                res.push_back(std::move(e));
            }
            logger::info("repo::event", "events size = "+std::to_string(res.size()));
            return std::move(res);
        };

        std::unique_ptr<::event::Event>& findNext();

        std::unique_ptr<::event::Event>& find(std::string hash);

    };
};
