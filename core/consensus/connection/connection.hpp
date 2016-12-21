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

#include "../../infra/protobuf/event.grpc.pb.h"
#include "../consensus_event.hpp"

namespace connection {

    struct Config{
        std::string name;
        std::string ip_addr;
        std::string port;
    };

    void initialize_peer();

    bool send(
        const std::string& ip,
        const Event::ConsensusEvent& msg)
    ;

    bool sendAll(
        const Event::ConsensusEvent& msg
    );

    bool receive(const std::function<void(
        const std::string&,
        Event::ConsensusEvent&)
    >& callback);


    void addSubscriber(std::string ip);

    int run();

    void finish();

};  // end connection

// implements
#endif
