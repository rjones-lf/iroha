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

#include "../../service/json_parse_with_json_nlohman.hpp"
#include "../../service/json_parse.hpp"
#include "../../service/json_parse.hpp"

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

    template<typename T>
    connection_object::ConsensusEvent encodeConsensusEvent(std::unique_ptr<T>&& event){
        logger::error("connection","No implements error :"+ std::string(typeid(T).name()));
        throw "No implements";
    }

    template<>
    connection_object::ConsensusEvent encodeConsensusEvent(
        std::unique_ptr<
            ConsensusEvent<
                Transaction<
                    Add<Asset>
                >
            >
        >&& event
    ) {
        connection_object::Asset asset;
        auto txObj = event->dump().dictSub["transaction"];
        auto assetObj = txObj.dictSub["command"].dictSub["object"];

        asset.set_domain(assetObj.dictSub["domain"].str);
        asset.set_name(assetObj.dictSub["name"].str);
        asset.set_value(static_cast<google::protobuf::uint64>(assetObj.dictSub["value"].integer));
        asset.set_precision(static_cast<google::protobuf::uint64>(assetObj.dictSub["precision"].integer));

        connection_object::Transaction tx;
        tx.set_type(event->getCommandName());
        tx.set_senderpubkey(event->dump().dictSub["transaction"].dictSub["senderPublicKey"].str);
        tx.mutable_asset()->CopyFrom(asset);

        connection_object::ConsensusEvent consensusEvent;
        consensusEvent.mutable_transaction()->CopyFrom(tx);

        if(!event->eventSignatures().empty()) {
            for (auto &esig: event->eventSignatures()) {
                connection_object::EventSignature eventSig;
                eventSig.set_publickey(std::get<0>(esig));
                eventSig.set_signature(std::get<1>(esig));
                consensusEvent.add_eventsignatures()->CopyFrom(eventSig);
            }
        }

        return consensusEvent;
    }

    template<typename T>
    T decodeConsensusEvent(const connection_object::ConsensusEvent& event){
        logger::error("connection","No implements error :"+ std::string(typeid(T).name()));
        throw "No implements";
    }

    template<>
    std::unique_ptr<
        ConsensusEvent<
            Transaction<
                Add<Asset>
            >
        >
    > decodeConsensusEvent(
        const connection_object::ConsensusEvent& event
    ) {
        auto tx = event.transaction();
        auto asset = tx.asset();

        auto consensusEvent =  std::make_unique<ConsensusEvent<
            Transaction<
                    Add<Asset>
            >
        >>(
            tx.senderpubkey(),
            asset.domain(),
            asset.name(),
            asset.value(),
            asset.precision()
        );
        for(const auto& esig: event.eventsignatures()){
            consensusEvent->addSignature(esig.publickey(), esig.signature());
        }
        for(const auto& txsig: event.transaction().txsignatures()){
            consensusEvent->addTxSignature(txsig.publickey(), txsig.signature());
        }
        return consensusEvent;
    }

    class IrohaConnectionClient {
        public:
        explicit IrohaConnectionClient(std::shared_ptr<Channel> channel)
            : stub_(IrohaConnection::NewStub(channel)) {}

        std::string Operation(const std::unique_ptr<event::Event>& event) {
            connection_object::StatusResponse response;

            // ToDo refactoring it's only add asset. separate funciton event -> some transaction ... = _ =
            connection_object::ConsensusEvent consensusEvent = encodeConsensusEvent(
                json_parse_with_json_nlohman::parser::load<
                    ConsensusEvent<
                        Transaction<
                           Add<object::Asset>
                        >
                    >
             >(json_parse_with_json_nlohman::parser::dump(event->dump())));

            ClientContext context;

            Status status = stub_->Operation(&context, consensusEvent, &response);
            if (status.ok()) {
                logger::info("connection", "response:" + response.value());
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
            for(auto& f: receivers){
                f("from",
                  std::move(decodeConsensusEvent<std::unique_ptr<ConsensusEvent<
                    Transaction<
                        Add<Asset>
                    >
                  >>>(*event))
                );
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
        const std::unique_ptr<event::Event>& event
    ) {
        if(find( receiver_ips.begin(), receiver_ips.end() , ip) != receiver_ips.end()){
            logger::info("connection", "create client");
            IrohaConnectionClient client(grpc::CreateChannel(
                ip + ":50051", grpc::InsecureChannelCredentials())
            );
            logger::info("connection", "invoke client Operation");
            std::string reply = client.Operation(event);
            return true;
        }else{
            logger::error("connection", "not found");
            return false;
        }
    }

    bool sendAll(
        const std::unique_ptr<
            event::Event
        >& event
    ) {
        // WIP
        logger::info("connection", "send mesage:"+ event->getHash());
        for(auto& ip : receiver_ips){
            if( ip != peer::getMyIp()){
                send( ip, event);
            }
        }
        return true;
    }

    bool receive(const std::function<void(
        const std::string&,
        std::unique_ptr<
            event::Event
        >&&)>& callback) {
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