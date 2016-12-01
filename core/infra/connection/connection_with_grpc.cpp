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

#include "../../consensus/connection/connection.hpp"

#include "../../util/logger.hpp"
#include "../../service/peer_service.hpp"

#include <grpc++/grpc++.h>

#include "connection.grpc.pb.h"

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

using grpc::Channel;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ClientContext;
using grpc::Status;

using connection_object::IrohaConnection;

template<typename T>
using ConsensusEvent = event::ConsensusEvent<T>;
template<typename T>
using Transaction = transaction::Transaction<T>;
template<typename T>
using Transfer = command::Transfer<T>;
template<typename T>
using Add = command::Add<T>;
using object::Asset;
using object::Domain;

namespace connection {

    std::vector<std::string> receiver_ips;
    std::vector<
        std::function<void(
            std::string from,
            std::unique_ptr<::event::Event> message)
        >
    > receivers;

    class IrohaConnectionClient {
        public:
        IrohaConnectionClient(std::shared_ptr<Channel> channel)
            : stub_(IrohaConnection::NewStub(channel)) {}

        std::string Operation(const std::string& message) {
            connection_object::StatusResponse response;
            connection_object::ConsensusEvent event;
            ClientContext context;

            Status status = stub_->Operation(&context, event, &response);
            if (status.ok()) {
                return response.value();
            } else {
                std::cout << status.error_code() << ": "
                    << status.error_message() << std::endl;
                return "RPC failed";
            }
        }

        private:
        std::unique_ptr<IrohaConnection::Stub> stub_;
    };

    class IrohaConnectionServiceImpl final : public IrohaConnection::Service {
        public:
        Status Operation(ServerContext* context,
            const connection_object::ConsensusEvent* event,
            connection_object::StatusResponse* response
        ) override {
            logger::info("connection", "operation");
            for(auto& f: receivers){
                //f("from",event->DebugString());
            }
            response->set_value("OK");
            return Status::OK;
        }
    };

    IrohaConnectionServiceImpl service;
    ServerBuilder builder;

    void initialize_peer() {
        std::string server_address("0.0.0.0:50051");
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
    }

    bool send(const std::string& ip,
        const std::unique_ptr<event::Event>& event) {
        
        if(find( receiver_ips.begin(), receiver_ips.end() , ip) != receiver_ips.end()){
            IrohaConnectionClient client(grpc::CreateChannel(
                ip + ":50051", grpc::InsecureChannelCredentials())
            );
            //std::string reply = client.Operation(event);
            //logger::info("connection", "send successfull :"+ reply);
            return true;
        }else{
            logger::error("connection", "not found");
            return false;
        }
    }

    bool sendAll(
        const std::unique_ptr<
            event::Event
        >& msg
    ) {
        // WIP
        //logger::info("connection", "send mesage"+ msg);
        for(auto& ip : receiver_ips){
            if( ip != peer::getMyIp()){
        //        send( ip, msg);
            }
        }
        return true;
    }

    bool receive(const std::function<void(
        const std::string& from,
        std::unique_ptr<
            event::Event
        >&& message)>& callback) {
        receivers.push_back(callback);
        return true;
    }

    void addSubscriber(std::string ip) {
        receiver_ips.push_back(ip);
    }

    int run() {
        std::unique_ptr<Server> server(builder.BuildAndStart());
        server->Wait();
        return 0;
    }

};