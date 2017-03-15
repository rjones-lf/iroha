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
#include <util/datetime.hpp>
#include <service/peer_service.hpp>

#include <infra/config/peer_service_with_json.hpp>
#include <infra/config/iroha_config_with_json.hpp>

#include <repository/transaction_repository.hpp>
#include <repository/domain/asset_repository.hpp>
#include <repository/domain/account_repository.hpp>

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
using grpc::ServerReader;

namespace connection {

    using Api::Sumeragi;
    using Api::Izanami;
    using Api::TransactionRepository;
    using Api::AssetRepository;

    using Api::Query;
    using Api::StatusResponse;


    using Api::Query;
    using Api::ConsensusEvent;
    using Api::StatusResponse;
    using Api::Transaction;
    using Api::TransactionResponse;
    using Api::AssetResponse;
    using Api::RecieverConfirmation;
    using Api::Signature;

    enum ResponseType {
        RESPONSE_OK,
        // wrong signature
        RESPONSE_INVALID_SIG,
        // connection error
        RESPONSE_ERRCONN,
    };

    using Response = std::pair<std::string, ResponseType>;

    // TODO: very dirty solution, need to be out of here
    #include <crypto/signature.hpp>
    std::function<RecieverConfirmation(const std::string&)> sign = [](const std::string &hash) {
        RecieverConfirmation confirm;
        Signature signature;
        signature.set_publickey(config::PeerServiceConfig::getInstance().getMyPublicKey());
        signature.set_signature(signature::sign(
            hash,
            config::PeerServiceConfig::getInstance().getMyPublicKey(),
            config::PeerServiceConfig::getInstance().getMyPrivateKey())
        );
        confirm.set_hash(hash);
        confirm.mutable_signature()->Swap(&signature);
        return confirm;
    };

    std::function<bool(const RecieverConfirmation&)> valid = [](const RecieverConfirmation &c) {
        return signature::verify(c.signature().signature(), c.hash(), c.signature().publickey());
    };

    namespace iroha {
        namespace Sumeragi {
            namespace Verify {
                std::vector<
                        std::function<void(
                                const std::string& from,
                                ConsensusEvent& message)
                        >
                > receivers;
            };
            namespace Torii {
                std::vector<
                        std::function<void(
                                const std::string& from,
                                Transaction& message
                        )>
                > receivers;
            }
        };
        namespace Izanami {
            namespace Izanagi {
                std::vector<
                        std::function<void(
                                const std::string& from,
                                TransactionResponse& txResponse
                        )>
                > receivers;
            }
        }
        namespace TransactionRepository {
            namespace find {
                std::vector<
                        std::function<void(
                                const std::string& from,
                                Query& message
                        )>
                > receivers;
            };
            namespace fetch {
                std::vector<
                        std::function<void(
                                const std::string& from,
                                Query& message
                        )>
                > receivers;
            };

            namespace fetchStream {
                std::vector<
                        std::function<void(
                                const std::string& from,
                                Query& message
                        )>
                > receivers;
            };
        }
        namespace AssetRepository {
            namespace find {
                std::vector<
                        std::function<void(
                                const std::string &from,
                                Query &message
                        )>
                > receivers;
            }
        }
    };

    class SumeragiConnectionClient {
    public:
        explicit SumeragiConnectionClient(std::shared_ptr<Channel> channel)
                : stub_(Sumeragi::NewStub(channel)) {}

        Response Verify(const ConsensusEvent& consensusEvent) {
            StatusResponse response;
            logger::info("connection")  <<  "Operation";
            logger::info("connection")  <<  "size: "    <<  consensusEvent.eventsignatures_size();
            logger::info("connection")  <<  "name: "    <<  consensusEvent.transaction().asset().name();

            ClientContext context;

            Status status = stub_->Verify(&context, consensusEvent, &response);

            if (status.ok()) {
                logger::info("connection")  << "response: " << response.value();
                return {response.value(), valid(response.confirm()) ? RESPONSE_OK : RESPONSE_INVALID_SIG};
            } else {
                logger::error("connection") << status.error_code() << ": " << status.error_message();
                //std::cout << status.error_code() << ": " << status.error_message();
                return {"RPC failed", RESPONSE_ERRCONN};
            }
        }

        Response Torii(const Transaction& transaction) {
            StatusResponse response;

            ClientContext context;

            Status status = stub_->Torii(&context, transaction, &response);

            if (status.ok()) {
                logger::info("connection")  << "response: " << response.value();
                return {response.value(), RESPONSE_OK};
            } else {
                logger::error("connection") << status.error_code() << ": " << status.error_message();
                //std::cout << status.error_code() << ": " << status.error_message();
                return {"RPC failed", RESPONSE_ERRCONN};
            }
        }

        bool Kagami() {
            StatusResponse response;
            ClientContext context;
            Query query;
            Status status = stub_->Kagami(&context, query, &response);
            if (status.ok()) {
                logger::info("connection")  << "response: " << response.value();
                return true;
            } else {
                logger::error("connection") << status.error_code() << ": " << status.error_message();
                return false;
            }
        }

    private:
        std::unique_ptr<Sumeragi::Stub> stub_;
    };

    class IzanamiConnectionClient {
    public:
        explicit IzanamiConnectionClient(std::shared_ptr<Channel> channel)
        : stub_(Izanami::NewStub(channel)) {}

        bool Izanagi(const TransactionResponse& txResponse) {
            StatusResponse response;
            logger::info("connection")  <<  "Operation";
            logger::info("connection")  <<  "size: "    <<  txResponse.transaction_size();
            logger::info("connection")  <<  "message: "    <<  txResponse.message();

            ClientContext context;
            Status status = stub_->Izanagi(&context, txResponse, &response);

            if (status.ok()) {
                logger::info("connection")  << "response: " << response.value();
                return true;
            } else {
                logger::error("connection") << status.error_code() << ": " << status.error_message();
                return false;
            }
        }

    private:
        std::unique_ptr<Izanami::Stub> stub_;
    };

    class SumeragiConnectionServiceImpl final : public Sumeragi::Service {
    public:

        Status Verify(
                ServerContext*          context,
                const ConsensusEvent*   pevent,
                StatusResponse*         response
        ) override {
            RecieverConfirmation confirm;
            ConsensusEvent event;
            event.CopyFrom(pevent->default_instance());
            event.mutable_eventsignatures()->CopyFrom(pevent->eventsignatures());
            event.mutable_transaction()->CopyFrom(pevent->transaction());
            event.set_status(pevent->status());
            logger::info("connection") << "size: " << event.eventsignatures_size();
            auto dummy = "";
            for (auto& f: iroha::Sumeragi::Verify::receivers){
                f(dummy, event);
            }
            confirm = sign(pevent->transaction().hash());
            response->set_value("OK");
            response->mutable_confirm()->CopyFrom(confirm);
            return Status::OK;
        }

        Status Torii(
            ServerContext*      context,
            const Transaction*  transaction,
            StatusResponse*     response
        ) override {
            RecieverConfirmation confirm;
            auto dummy = "";
            Transaction tx;
            tx.CopyFrom(*transaction);
            for (auto& f: iroha::Sumeragi::Torii::receivers){
                f(dummy, tx);
            }
            confirm = sign(transaction->hash());
            response->set_value("OK");
            response->mutable_confirm()->CopyFrom(confirm);
            return Status::OK;
        }

        Status Kagami(
            ServerContext*      context,
            const Query*          query,
            StatusResponse*     response
        ) override {
            response->set_message("OK, no problem!");
            response->set_value("Alive");
            response->set_timestamp(datetime::unixtime());
            return Status::OK;
        }

    };

    class IzanamiConnectionServiceImpl final : public Izanami::Service {
    public:

        Status Izanagi(
                ServerContext*          context,
                const TransactionResponse*   txResponse,
                StatusResponse*         response
        ) override {
            TransactionResponse txres;
            txres.CopyFrom(txResponse->default_instance());
            txres.set_message(txResponse->message());
            txres.set_code(txResponse->code());
            txres.mutable_transaction()->CopyFrom(txResponse->transaction());
            logger::info("connection") << "size: " << txres.transaction_size();
            auto dummy = "";
            for (auto& f: iroha::Izanami::Izanagi::receivers){
                f(dummy, txres);
            }
            response->set_message("OK, no problem!");
            response->set_value("OK");
            response->set_timestamp(datetime::unixtime());
            return Status::OK;
        }
    };

    class TransactionRepositoryServiceImpl final : public TransactionRepository::Service {
      public:

        Status find(
            ServerContext*          context,
            const Query*              query,
            TransactionResponse*   response
        ) override {
            Query q;
            q.CopyFrom(*query);
            // ToDo use query
            auto transactions = repository::transaction::findAll();
            for(auto tx: transactions){
                response->add_transaction()->CopyFrom(tx);
            }
            response->set_message("OK");
            return Status::OK;
        }

        Status fetch(
            ServerContext*          context,
            const Query*              query,
            TransactionResponse*   response
        ) override {
            Query q;
            q.CopyFrom(*query);
            auto dummy = "";
            for (auto& f: iroha::TransactionRepository::find::receivers){
                f(dummy, q);
            }
            response->set_message("OK");
            return Status::OK;
        }

        Status fetchStream(
            ServerContext* context,
            ServerReader<Transaction>* reader,
            StatusResponse* response
        ) override {
            Query q;
            std::vector<Transaction> txs;
            Transaction tx;
            while(reader->Read(&tx)){
                txs.push_back(tx);
            }
            response->set_message("OK");
            return Status::OK;
        }
    };

    class AssetRepositoryServiceImpl final : public AssetRepository::Service {
    public:
        Status find(
            ServerContext*          context,
            const Query*              query,
            AssetResponse*         response
        ) override {
            std::string name = "default";
            Query q;
            q.CopyFrom(*query);
            logger::info("connection") << "AssetRepositoryService: " << q.DebugString();

            if(q.value().find("name")!=q.value().end()){
                name = q.value().at("name").valuestring();
            }

            auto sender = q.senderpubkey();
            if(q.type() == "asset"){
                response->mutable_asset()->CopyFrom(repository::asset::find(sender, name));
                logger::info("connection") << "AssetRepositoryService: " << response->asset().DebugString();
            }else if(q.type() == "account"){
                auto account = repository::account::find(sender);

                response->mutable_account()->CopyFrom(account);
            }
            response->set_message("OK");
            return Status::OK;
        }
    };

    namespace iroha {

        namespace Sumeragi {

            SumeragiConnectionServiceImpl service;

            namespace Verify {

                bool receive(
                    const std::function<void(
                        const std::string&,
                        ConsensusEvent&)>& callback
                ) {
                    receivers.push_back(callback);
                    return true;
                }

                bool send(
                    const std::string &ip,
                    const ConsensusEvent &event
                ) {
                    auto receiver_ips = config::PeerServiceConfig::getInstance().getIpList();
                    if (find(receiver_ips.begin(), receiver_ips.end(), ip) != receiver_ips.end()) {
                        SumeragiConnectionClient client(
                            grpc::CreateChannel(
                                ip + ":" + std::to_string(config::IrohaConfigManager::getInstance().getGrpcPortNumber(50051)),
                                grpc::InsecureChannelCredentials()
                            )
                        );
                        // TODO return tx validity
                        auto reply = client.Verify(event);
                        return true;
                    } else {
                        return false;
                    }
                }

                bool sendAll(
                    const ConsensusEvent &event
                ) {
                    auto receiver_ips = config::PeerServiceConfig::getInstance().getIpList();
                    for (auto &ip : receiver_ips) {
                        if (ip != config::PeerServiceConfig::getInstance().getMyIp()) {
                            send(ip, event);
                        }
                    }
                    return true;
                }

            }

            namespace Torii {

                bool receive(
                    const std::function<void(
                    const std::string &,
                    Transaction&)
                > &callback){
                    receivers.push_back(callback);
                    return true;
                }

            }

        }


        namespace PeerService {

            namespace Sumeragi {
                bool send(
                        const std::string &ip,
                        const Transaction &transaction
                ) {
                    auto receiver_ips = config::PeerServiceConfig::getInstance().getIpList();
                    if (find(receiver_ips.begin(), receiver_ips.end(), ip) != receiver_ips.end()) {
                        SumeragiConnectionClient client(
                                grpc::CreateChannel(
                                        ip + ":" + std::to_string(
                                                config::IrohaConfigManager::getInstance().getGrpcPortNumber(50051)),
                                        grpc::InsecureChannelCredentials()
                                )
                        );
                        // TODO return tx validity
                        auto reply = client.Torii(transaction);
                        return true;
                    } else {
                        return false;
                    }
                }

                bool ping(
                        const std::string &ip
                ) {
                    auto receiver_ips = config::PeerServiceConfig::getInstance().getIpList();
                    if (find(receiver_ips.begin(), receiver_ips.end(), ip) != receiver_ips.end()) {
                        SumeragiConnectionClient client(
                                grpc::CreateChannel(
                                        ip + ":" + std::to_string(
                                                config::IrohaConfigManager::getInstance().getGrpcPortNumber(50051)),
                                        grpc::InsecureChannelCredentials()
                                )
                        );
                        return client.Kagami();
                    } else {
                        logger::error("Connection_with_grpc") << "Unexpected ip: " << ip;
                        return false;
                    }
                }
            }

            namespace Izanami {
                bool send(
                        const std::string& ip,
                        const TransactionResponse &txResponse
                ) {
                    IzanamiConnectionClient client(
                            grpc::CreateChannel(
                                    ip + ":" + std::to_string(
                                            config::IrohaConfigManager::getInstance().getGrpcPortNumber(50051)),
                                    grpc::InsecureChannelCredentials()
                            )
                    );
                    return client.Izanagi(txResponse);
                }
            }
        }


        namespace Izanami {
            namespace Izanagi {
                bool receive(const std::function<void(
                        const std::string &,
                        TransactionResponse&)
                > &callback) {
                    receivers.push_back(callback);
                }
            }
        }

        namespace TransactionRepository {
            TransactionRepositoryServiceImpl service;

            namespace find {
                bool receive(
                    const std::function<void(
                        const std::string &,
                        Query &)> &callback
                ) {
                    receivers.push_back(callback);
                    return true;
                }
            };

            namespace fetch {
                bool receive(
                    const std::function<void(
                        const std::string &,
                        Query &)> &callback
                ) {
                    receivers.push_back(callback);
                    return true;
                }
            };

            namespace fetchStream {
                bool receive(
                    const std::function<void(
                        const std::string &,
                        Query &)> &callback
                ) {
                    receivers.push_back(callback);
                    return true;
                }
            };
        };

        namespace AssetRepository {
            namespace find {
                AssetRepositoryServiceImpl service;

                bool receive(
                        const std::function<void(
                                const std::string &,
                                Query &)> &callback
                ) {
                    receivers.push_back(callback);
                    return true;
                }
            };
        }

    };

    ServerBuilder builder;

    void initialize_peer() {
        std::string server_address("0.0.0.0:" + std::to_string(config::IrohaConfigManager::getInstance().getGrpcPortNumber(50051)));
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&iroha::Sumeragi::service);
        builder.RegisterService(&iroha::TransactionRepository::service);
        builder.RegisterService(&iroha::AssetRepository::find::service);
    }

    int run() {
        std::unique_ptr<Server> server(builder.BuildAndStart());
        server->Wait();
        return 0;
    }
    void finish(){
    }

};
