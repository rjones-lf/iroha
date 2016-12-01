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
#ifndef __CONNECTION__
#define __CONNECTION__

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

#include "../../model/commands/add.hpp"
#include "../../model/commands/transfer.hpp"

#include "../../model/objects/asset.hpp"
#include "../../model/objects/domain.hpp"
#include "../../model/objects/message.hpp"

#include "../../model/transaction.hpp"

#include "../consensus_event.hpp"
#include "../event.hpp"

namespace connection {

    struct Config{
        std::string name;
        std::string ip_addr;
        std::string port;
    };

    void initialize_peer();

    bool send(
        const std::string& ip,
        const std::unique_ptr<
            ::event::Event
        >& msg);

    bool sendAll(
        const std::unique_ptr<
            ::event::Event
        >& msg);

    bool send(
        const std::string& to,
        const std::unique_ptr<
            ::event::Event
        >& message);

    bool receive(const std::function<void(
            const std::string& from,
            std::unique_ptr<::event::Event>&& message)
        >& callback);

    void addSubscriber(std::string ip);

    int run();

    void finish();

};  // end connection

// implements
#endif
