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

#include <grpc++/grpc++.h>

#include <consensus/connection/connection.hpp>
#include <util/logger.hpp>
#include <service/peer_service.hpp>

#include "../protobuf/event.grpc.pb.h"

#include <model/commands/add.hpp>
#include <model/commands/transfer.hpp>
#include <model/commands/update.hpp>

#include <model/objects/asset.hpp>
#include <model/objects/domain.hpp>
#include <model/objects/account.hpp>

#include <infra/config/peer_service_with_json.hpp>
#include <infra/config/iroha_config_with_json.hpp>

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

using Event::IrohaConnection;

template<typename T>
using ConsensusEvent = event::ConsensusEvent<T>;
template<typename T>
using Transaction = transaction::Transaction<T>;
template<typename T>
using Transfer = command::Transfer<T>;
template<typename T>
using Add = command::Add<T>;
template<typename T>
using Update = command::Update<T>;
using object::Asset;
using object::Domain;

namespace connection {

    std::vector<std::string> receiver_ips;
    std::vector<
        std::function<void(
           const std::string& from,
           Event::ConsensusEvent& message)
        >
    > receivers;

    class IrohaConnectionClient {
        public:
        explicit IrohaConnectionClient(std::shared_ptr<Channel> channel)
            : stub_(IrohaConnection::NewStub(channel)) {}

        std::string Operation(const Event::ConsensusEvent& consensusEvent) {
            Event::StatusResponse response;
            logger::info("connection")  <<  "Operation";
            logger::info("connection")  <<  "size: "    <<  consensusEvent.eventsignatures_size();
            logger::info("connection")  <<  "name: "    <<  consensusEvent.transaction().asset().name();

            ClientContext context;

            Status status = stub_->Operation(&context, consensusEvent, &response);
            if (status.ok()) {
                logger::info("connection")  << "response: " << response.value();
                return response.value();
            } else {
                logger::error("connection") << status.error_code() << ": " << status.error_message();
                //std::cout << status.error_code() << ": " << status.error_message();
                return "RPC failed";
            }
        }

        private:
        std::unique_ptr<IrohaConnection::Stub> stub_;
    };

    class IrohaConnectionServiceImpl final : public IrohaConnection::Service {
        public:
        Status Operation(ServerContext* context,
            const Event::ConsensusEvent* pevent,
            Event::StatusResponse* response
        ) override {
            Event::ConsensusEvent event;
            event.CopyFrom(pevent->default_instance());
            event.mutable_eventsignatures()->CopyFrom(pevent->eventsignatures());
            event.mutable_transaction()->CopyFrom(pevent->transaction());
            logger::info("connection") << "size: " << event.eventsignatures_size();
            auto dummy = "";
            for (auto& f: receivers){
                f(dummy, event);
            }
            response->set_value("OK");
            return Status::OK;
        }
    };

    IrohaConnectionServiceImpl service;
    ServerBuilder builder;

    void initialize_peer() {
        std::string server_address("0.0.0.0:" + std::to_string(config::IrohaConfigManager::getInstance().getGrpcPortNumber(50051)));
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
    }

    bool send(
        const std::string& ip,
        const Event::ConsensusEvent& event
    ) {
        logger::info("connection") << "start send";
        if (find(receiver_ips.begin(), receiver_ips.end(), ip) != receiver_ips.end()){
            logger::info("connection")  <<  "create client";
            IrohaConnectionClient client(grpc::CreateChannel(
                ip + ":" + std::to_string(config::IrohaConfigManager::getInstance().getGrpcPortNumber(50051)),
                grpc::InsecureChannelCredentials())
            );
            logger::info("connection")  <<  "invoke client Operation";
            logger::info("connection")  <<  "size " <<  event.eventsignatures_size();
            std::string reply = client.Operation(event);
            return true;
        }else{
            logger::error("connection") <<  "not found";
            return false;
        }
    }

    bool sendAll(
        const Event::ConsensusEvent& event
    ) {
        // WIP
        for (auto& ip : receiver_ips){
            if (ip != config::PeerServiceConfig::getInstance().getMyIp()){
                send( ip, event);
            }
        }
        return true;
    }

    bool receive(const std::function<void(
        const std::string&,
        Event::ConsensusEvent&)>& callback) {
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
