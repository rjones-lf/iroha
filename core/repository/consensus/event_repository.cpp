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
#include "../../service/json_parse.hpp"

namespace repository {
    namespace event {

        std::vector<std::unique_ptr<::event::Event>> events;

        bool add(
            const std::string &hash,
            std::unique_ptr<::event::Event> event
        ){
            events.push_back(std::move(event));
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
        >& findAll(){
            return events;
        };

        std::unique_ptr<::event::Event>& findNext();

        std::unique_ptr<::event::Event>& find(std::string hash);

    };
};
