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

#include <cappuccino.hpp>
#include <json.hpp>

#include <consensus/connection/connection.hpp>
#include <crypto/hash.hpp>
#include <infra/config/iroha_config_with_json.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <infra/protobuf/api.pb.h>
#include <server/http_server.hpp>
#include <service/peer_service.hpp>
#include <transaction_builder/transaction_builder.hpp>
#include <util/logger.hpp>

// -- WIP --
#include <grpc++/grpc++.h>
using grpc::Channel;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ClientContext;
using grpc::Status;
// -------

namespace http {
    using namespace Api;

    using txbuilder::TransactionBuilder;
    using type_signatures::Remove;
    using type_signatures::Domain;
    using type_signatures::Account;
    using type_signatures::Asset;
    using type_signatures::SimpleAsset;
    using type_signatures::Peer;

    const auto assetName = "PointDemo";

    using nlohmann::json;
    using Request = Cappuccino::Request;
    using Response = Cappuccino::Response;


    json responseError(std::string message){
        return json({
                            {"message", std::move(message)},
                            {"status", 400}
                    });
    }

    enum class RequestType{
        Int,
        Str,
        Bool,
        Float
    };

    std::vector<std::string> split(const std::string& str, const std::string& delim) noexcept{
        std::vector<std::string> result;
        std::string::size_type pos = 0;
        while(pos != std::string::npos) {
            auto p = str.find(delim, pos);
            if(p == std::string::npos){
                result.push_back(str.substr(pos));
                break;
            }else{
                result.push_back(str.substr(pos, p - pos));
            }
            pos = p + delim.size();
        }
        return result;
    }

    std::string Torii(std::unique_ptr<Sumeragi::Stub> stub_,const Transaction& transaction) {
        StatusResponse response;

        ClientContext context;

        Status status = stub_->Torii(&context, transaction, &response);

        if (status.ok()) {
            logger::info("connection")  << "response: " << response.value();
            return response.value();
        } else {
            logger::error("connection") << status.error_code() << ": " << status.error_message();
            //std::cout << status.error_code() << ": " << status.error_message();
            return "RPC failed";
        }
    }

    void server() {
        logger::info("server") << "initialize server!";

        std::vector<std::string> params = {"", "-p", std::to_string(config::IrohaConfigManager::getInstance().getHttpPortNumber(1204))};
        std::vector<char*> argv;
        for (const auto& arg : params)
            argv.push_back((char*)arg.data());
        argv.push_back(nullptr);
        Cappuccino::Cappuccino( argv.size() - 1, argv.data() );

        Cappuccino::route<Cappuccino::Method::POST>("/account/register", [](std::shared_ptr<Request> request) -> Response {
            auto res = Response(request);
            auto data = request->json();
            std::string uuid = hash::sha3_256_hex(data["publicKey"].get<std::string>());

            Api::Domain domain;
            domain.set_ownerpublickey("pubkey1");
            domain.set_name("name");
            auto txDomain = TransactionBuilder<Remove<Domain>>()
                .setSenderPublicKey("karin")
                .setDomain(domain)
                .build();

            Torii(
                Sumeragi::NewStub(grpc::CreateChannel(
                    ::peer::myself::getIp() + ":" +
                    std::to_string(config::IrohaConfigManager::getInstance().getGrpcPortNumber(50051)),
                    grpc::InsecureChannelCredentials()
                )),
                txDomain
            );

            res.json(json({
              {"status",  200},
              {"message", "successful"},
              {"uuid",   uuid}
            }));

            return res;
        });

        Cappuccino::route<Cappuccino::Method::GET>( "/account",[](std::shared_ptr<Request> request) -> Response{
            std::string uuid = request->params("uuid");
            auto res = Response(request);

            res.json(json({
                  {"status",  200}
            }));

            return res;
        });

        Cappuccino::route<Cappuccino::Method::POST>( "/asset/operation",[](std::shared_ptr<Request> request) -> Response{
            auto res = Response(request);
            auto data = request->json();
            if(!data.empty()) {

            }
            res.json(json({
              {"status",  200},
              {"message", "Ok"}
            }));
            return res;
        });

        Cappuccino::route<Cappuccino::Method::GET>( "/history/transaction",[](std::shared_ptr<Request> request) -> Response{
            std::string uuid = request->params("uuid");
            auto res = Response(request);
            auto tx_json = json::array();

            res.json(json({
              {"status",  200},
              {"history", tx_json}
            }));
            return res;
        });

        logger::info("server") << "start server!";
        // runnning
        Cappuccino::run();

    }
};  // namespace http