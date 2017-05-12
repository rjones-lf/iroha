
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


#include <service/flatbuffer_service.h>
#include <grpc++/grpc++.h>

#include <crypto/hash.hpp>
#include <crypto/signature.hpp>
#include <infra/config/iroha_config_with_json.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <membership_service/peer_service.hpp>
#include <service/connection.hpp>
#include <utils/datetime.hpp>
#include <utils/expected.hpp>
#include <utils/logger.hpp>

#include <endpoint.grpc.fb.h>
#include <main_generated.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <asset_generated.h>

namespace connection {
  /**
   * Using
   */
  using Sumeragi = ::iroha::Sumeragi;
  using Hijiri = ::iroha::Hijiri;
  using ConsensusEvent = ::iroha::ConsensusEvent;
  using Ping = ::iroha::Ping;
  using Response = ::iroha::Response;
  using AssetResponse = ::iroha::AssetResponse;
  using AssetQuery = ::iroha::AssetQuery;
  using Transaction = ::iroha::Transaction;
  using TransactionWrapper = ::iroha::TransactionWrapper;
  using Signature = ::iroha::Signature;

  using grpc::Channel;
  using grpc::Server;
  using grpc::ServerBuilder;
  using grpc::ServerContext;
  using grpc::ClientContext;
  using grpc::Status;

  /**
   * Enum
   */
  enum ResponseType {
    RESPONSE_OK,
    RESPONSE_INVALID_SIG,  // wrong signature
    RESPONSE_ERRCONN,      // connection error
  };


  /************************************************************************************
   * Interface: Verify, Torii :: receive()
   ************************************************************************************/
  template<class CallBackFunc>
  class Receiver {
  public:
    VoidHandler set(CallBackFunc &&rhs) {
      if (receiver_) {
        return makeUnexpected(exception::DuplicateSetArgumentException(
          "Receiver<" + std::string(typeid(CallBackFunc).name()) + ">",
          __FILE__));
      }

      receiver_ = std::make_shared<CallBackFunc>(rhs);
      return {};
    }

    // ToDo rewrite operator() overload.
    void invoke(const std::string &from, flatbuffers::unique_ptr_t &&arg) {
      (*receiver_)(from, std::move(arg));
    }

  private:
    std::shared_ptr<CallBackFunc> receiver_;
  };

  template<class CallBackFunc, class T>
  class ReceiverWithReturen {
  public:
    VoidHandler set(CallBackFunc &&rhs) {
        if (receiver_) {
            return makeUnexpected(exception::DuplicateSetArgumentException(
                    "Receiver<" + std::string(typeid(CallBackFunc).name()) + ">",
                    __FILE__));
        }

        receiver_ = std::make_shared<CallBackFunc>(rhs);
        return {};
    }

    // ToDo rewrite operator() overload.
    T invoke(const std::string &from, flatbuffers::unique_ptr_t &&arg) {
        return (*receiver_)(from, std::move(arg));
    }

  private:
    std::shared_ptr<CallBackFunc> receiver_;
  };

  /**
   * Verify
   */
  namespace iroha {
    namespace SumeragiImpl {
      namespace Verify {
        Receiver<Verify::CallBackFunc> receiver;

        void receive(Verify::CallBackFunc&& callback) {
          receiver.set(std::move(callback));
        }

      } // namespace Verify
    } // namespace SumeragiImpl
  } // namespace iroha

  /**
   * Torii
   */
  namespace iroha {
    namespace SumeragiImpl {
      namespace Torii {
        Receiver<Torii::CallBackFunc> receiver;

        void receive(Torii::CallBackFunc&& callback) {
          receiver.set(std::move(callback));
        }

      }  // namespace Torii
    }  // namespace SumeragiImpl
  }  // namespace iroha

  /************************************************************************************
   * RPC: Verify, Torii
   ************************************************************************************/
  /**
   * SumeragiConnectionClient
   * - Verify, Torii
   */
  class SumeragiConnectionClient {
  public:
    explicit SumeragiConnectionClient(std::shared_ptr<Channel> channel)
      : stub_(Sumeragi::NewStub(channel)) {}

    const ::iroha::Response *Verify(
      const ::iroha::ConsensusEvent &consensusEvent) const {
      ClientContext context;
      flatbuffers::BufferRef<Response> responseRef;
      logger::info("connection") << "Operation";
      logger::info("connection")
        << "size: " << consensusEvent.peerSignatures()->size();
      logger::info("connection")
        << "Transaction: "
        << flatbuffer_service::toString(
          *consensusEvent.transactions()->Get(0)->tx_nested_root());

      flatbuffers::FlatBufferBuilder fbb;

      auto eventOffset =
        flatbuffer_service::copyConsensusEvent(fbb, consensusEvent);

      if (!eventOffset) {
        // FIXME: Argment BufferRef<Response>, return grpc::Status
        assert(false);
      }

      fbb.Finish(*eventOffset);

      auto reqEventRef =
        flatbuffers::BufferRef<::iroha::ConsensusEvent>(fbb.GetBufferPointer(),
                                                        fbb.GetSize());

      Status status =
        stub_->Verify(&context, reqEventRef, &responseRef);

      if (status.ok()) {
        logger::info("connection")
          << "response: " << responseRef.GetRoot()->message()->c_str();
        return responseRef.GetRoot();
      } else {
        logger::error("connection") << static_cast<int>(status.error_code())
                                    << ": " << status.error_message();
        return responseRef.GetRoot();
      }
    }

    const ::iroha::Response *Torii(
      const ::iroha::Transaction &tx) const {
      // Copy transaction to FlatBufferBuilder memory, then create
      // BufferRef<Transaction>
      // and share it to another sumeragi by using stub interface Torii.
      logger::info("connection") << "Operation";
      logger::info("connection")
          << "tx: "  << flatbuffer_service::toString(tx);

      ::grpc::ClientContext clientContext;
      flatbuffers::FlatBufferBuilder xbb;

      auto txOffset = flatbuffer_service::copyTransaction(xbb, tx);
      if (!txOffset) {
        assert(false);
      }

      xbb.Finish(txOffset.value());

      flatbuffers::BufferRef<::iroha::Transaction> requestTxRef(
        xbb.GetBufferPointer(), xbb.GetSize());

      flatbuffers::BufferRef<Response> responseRef;

      Status status =
        stub_->Torii(&clientContext, requestTxRef, &responseRef);

      if (status.ok()) {
        logger::info("connection")
          << "response: " << responseRef.GetRoot()->message();
        return responseRef.GetRoot();
      } else {
        logger::error("connection") << static_cast<int>(status.error_code())
                                    << ": " << status.error_message();
        // std::cout << status.error_code() << ": " << status.error_message();
        return responseRef.GetRoot();
      }
    }

  private:
    std::unique_ptr<Sumeragi::Stub> stub_;
  };

  /**
   * SumeragiConnectionServiceImpl
   */
  class SumeragiConnectionServiceImpl final : public ::iroha::Sumeragi::Service {
  public:
    Status Verify(ServerContext *context,
                  const flatbuffers::BufferRef<ConsensusEvent> *request,
                  flatbuffers::BufferRef<Response> *response) override {
      fbbResponse.Clear();

      {
        flatbuffers::FlatBufferBuilder fbb;
        auto event =
          flatbuffer_service::copyConsensusEvent(fbb, *request->GetRoot());
        if (!event) {
          fbb.Clear();
          auto responseOffset = ::iroha::CreateResponseDirect(
            fbbResponse, "CANCELLED", ::iroha::Code::FAIL,
            0);  // FIXME: Currently, if it fails, no signature.
          fbbResponse.Finish(responseOffset);

          *response = flatbuffers::BufferRef<::iroha::Response>(
            fbbResponse.GetBufferPointer(), fbbResponse.GetSize());
          return Status::CANCELLED;
        }

        fbb.Finish(event.value());

          const std::string from = "from";
          connection::iroha::SumeragiImpl::Verify::receiver.invoke(
                  from, std::move(fbb.ReleaseBufferPointer()));
      }

      auto tx_str = flatbuffer_service::toString(
        *request->GetRoot()
          ->transactions()
          ->Get(0) // Future work: #(tx) = 1
          ->tx_nested_root());
      auto responseOffset = ::iroha::CreateResponseDirect(
        fbbResponse, "OK!!", ::iroha::Code::UNDECIDED,
        flatbuffer_service::primitives::CreateSignature(
          fbbResponse, tx_str, datetime::unixtime()));

      fbbResponse.Finish(responseOffset);

      *response = flatbuffers::BufferRef<::iroha::Response>(
        fbbResponse.GetBufferPointer(), fbbResponse.GetSize());

      return Status::OK;
    }

    Status Torii(ServerContext *context,
                 const flatbuffers::BufferRef<Transaction> *txRef,
                 flatbuffers::BufferRef<Response> *responseRef) override {
      logger::debug("SumeragiConnectionServiceImpl::Torii") << "RPC works";

      // This reference remains until next calling
      // SumeragiConnectionServiceImpl::Torii() method.
      fbbResponse.Clear();

      {
        const auto tx = txRef->GetRoot();
        flatbuffers::FlatBufferBuilder fbb;
        auto txoffset = flatbuffer_service::copyTransaction(fbb, *tx);
        if (!txoffset) {
          fbb.Clear();
          auto responseOffset = ::iroha::CreateResponseDirect(
            fbbResponse, "CANCELLED", ::iroha::Code::FAIL,
            0);  // FIXME: Currently, if it fails, no signature.
          fbbResponse.Finish(responseOffset);

          *responseRef = flatbuffers::BufferRef<Response>(
            fbbResponse.GetBufferPointer(), fbbResponse.GetSize());
          return Status::CANCELLED;
        }

        fbb.Finish(txoffset.value());
        connection::iroha::SumeragiImpl::Torii::receiver.invoke(
          "from",  // TODO: Specify 'from'
          fbb.ReleaseBufferPointer());
      }

      auto tx_str = flatbuffer_service::toString(*txRef->GetRoot());

      auto responseOffset = ::iroha::CreateResponseDirect(
        fbbResponse, "OK!!", ::iroha::Code::UNDECIDED,
        flatbuffer_service::primitives::CreateSignature(
          fbbResponse, tx_str, datetime::unixtime()));

      fbbResponse.Finish(responseOffset);

      *responseRef = flatbuffers::BufferRef<Response>(
        fbbResponse.GetBufferPointer(), fbbResponse.GetSize());

      return Status::OK;
    }

  private:
    flatbuffers::FlatBufferBuilder fbbResponse;
  };


  /************************************************************************************
   * Interface: Verify::send()
   ************************************************************************************/
  namespace iroha {
    namespace SumeragiImpl {
      namespace Verify {
        bool send(const std::string &ip, const ::iroha::ConsensusEvent &event) {
          logger::info("connection") << "Send!";
          if (::peer::service::isExistIP(ip)) {
            logger::info("connection") << "IP exists: " << ip;
            SumeragiConnectionClient client(grpc::CreateChannel(
              ip + ":" +
              std::to_string(config::IrohaConfigManager::getInstance()
                               .getGrpcPortNumber(50051)),
              grpc::InsecureChannelCredentials()));
            // TODO return tx validity
            auto reply = client.Verify(event);
            if (!reply) {
              logger::error("connection") << ::iroha::EnumNameCode(reply->code())
                                          << ", " << reply->message();
              return false;
            }
            return true;
          } else {
            logger::info("connection") << "IP doesn't exist: " << ip;
            return false;
          }
        }

        bool sendAll(const ::iroha::ConsensusEvent &event) {
          auto receiver_ips = config::PeerServiceConfig::getInstance().getGroup();
          for (const auto &p : receiver_ips) {
            if (p["ip"].get<std::string>() !=
                config::PeerServiceConfig::getInstance().getMyIp()) {
              logger::info("connection") << "Send to " << p["ip"].get<std::string>();
              send(p["ip"].get<std::string>(), event);
            }
          }
          return true;
        }

      } // namespace Verify
    } // namespace SumeragiImpl
  } // namespace iroha

  /************************************************************************************
   * Hijiri
   ************************************************************************************/
  class HijiriConnectionClient {
  public:
    explicit HijiriConnectionClient(std::shared_ptr<Channel> channel)
      : stub_(Hijiri::NewStub(channel)) {}

    const ::iroha::Response *Kagami(const ::iroha::Ping &ping) const {
        ::grpc::ClientContext clientContext;
        flatbuffers::FlatBufferBuilder fbbPing;

        auto pingOffset = ::iroha::CreatePingDirect(
            fbbPing, ping.message()->c_str(), ping.sender()->c_str()
        );
        fbbPing.Finish(pingOffset);

        flatbuffers::BufferRef<::iroha::Ping> reqPingRef(
            fbbPing.GetBufferPointer(), fbbPing.GetSize()
        );

        flatbuffers::BufferRef<Response> responseRef;


        auto res = stub_->Kagami(&clientContext, reqPingRef, &responseRef);;
        logger::info("Connection with grpc") << "Send!";

        if (res.ok()) {
            logger::info("connection")
                    << "response: " << responseRef.GetRoot()->message();
            return responseRef.GetRoot();
        } else {
            logger::error("connection") << static_cast<int>(res.error_code())
                << ": " << res.error_message();
            // std::cout << status.error_code() << ": " << status.error_message();
            return responseRef.GetRoot();
        }
    }

  private:
    std::unique_ptr<Hijiri::Stub> stub_;
  };

  class HijiriConnectionServiceImpl final : public ::iroha::Hijiri::Service {
  public:
    Status Kagami(ServerContext *context,
                  const flatbuffers::BufferRef<Ping> *request,
                  flatbuffers::BufferRef<Response> *responseRef
    ) override {
      fbbResponse.Clear();
      {
          auto responseOffset = ::iroha::CreateResponseDirect(
              fbbResponse, "OK!!", ::iroha::Code::UNDECIDED,
              sign(fbbResponse, hash::sha3_256_hex(
                  request->GetRoot()->message()->str() +
                  request->GetRoot()->message()->str())
              )
          );
          fbbResponse.Finish(responseOffset);

          *responseRef = flatbuffers::BufferRef<Response>(
                  fbbResponse.GetBufferPointer(), fbbResponse.GetSize()
          );

          return Status::OK;
      }
    }

    // ToDo: Unite the way to hash (below double(?) hash)
    flatbuffers::Offset<::iroha::Signature> sign(
      flatbuffers::FlatBufferBuilder &fbb, const std::string &tx) {
      const auto stamp = datetime::unixtime();
      const auto hashWithTimestamp =
        hash::sha3_256_hex(tx + std::to_string(stamp));
      const auto signature = signature::sign(
        hashWithTimestamp,
        config::PeerServiceConfig::getInstance().getMyPublicKey(),
        config::PeerServiceConfig::getInstance().getMyPrivateKey());
      const std::vector<uint8_t> sigblob(signature.begin(), signature.end());
      return ::iroha::CreateSignatureDirect(
        fbb, config::PeerServiceConfig::getInstance().getMyPublicKey().c_str(),
        &sigblob, stamp);
    };

  private:
    flatbuffers::FlatBufferBuilder fbbResponse;
  };

  namespace memberShipService {
    namespace HijiriImpl {
      namespace Kagami {
        bool send(const std::string &ip, const ::iroha::Ping &ping) {  // TODO
          logger::info("Connection with grpc") << "Send!";
          logger::info("Connection with grpc") << "IP is: " << ip;
          HijiriConnectionClient client(grpc::CreateChannel(
            ip + ":" +
            std::to_string(config::IrohaConfigManager::getInstance()
                             .getGrpcPortNumber(50051)),
            grpc::InsecureChannelCredentials()));
          auto reply = client.Kagami(ping);
          return true;
        }
      }  // namespace Kagami
    }  // namespace HijiriImpl
    namespace SumeragiImpl {
      namespace Torii {
        bool send(const std::string &ip, const ::iroha::Transaction &tx) {
          logger::info("Connection with grpc") << "Send!";
          if (::peer::service::isExistIP(ip)) {
            logger::info("Connection with grpc") << "IP Exist: " << ip;
            SumeragiConnectionClient client(grpc::CreateChannel(
                ip + ":" +
                std::to_string(config::IrohaConfigManager::getInstance()
                                   .getGrpcPortNumber(50051)),
                grpc::InsecureChannelCredentials()));
            auto reply = client.Torii(tx);
            return true;
          }
        }
      } // namespace Torii
    } // namespace Sumeragi Impl
  }  // namespace MemberShipService

  /************************************************************************************
   * AssetRepository
   ************************************************************************************/
  namespace iroha {
      namespace AssetRepositoryImpl {
         namespace AccountGetAsset {
             // ToDo more clear
             ReceiverWithReturen<AccountGetAsset::CallBackFunc, std::vector<const ::iroha::Asset *>> receiver;

             void receive(AccountGetAsset::CallBackFunc&& callback) {
                 receiver.set(std::move(callback));
             }
         }
      }
  }
  /**
   * AssetRepositoryConnectionServiceImpl
   */
  class AssetRepositoryConnectionServiceImpl final : public ::iroha::AssetRepository::Service {
  public:
      Status AccountGetAsset(ServerContext *context,
                    const flatbuffers::BufferRef<::iroha::AssetQuery> *requestRef,
                    flatbuffers::BufferRef<::iroha::AssetResponse> *responseRef) override {
          fbbResponse.Clear();
          std::vector<const ::iroha::Asset *> assets;
          {
              const auto q = requestRef->GetRoot();
              flatbuffers::FlatBufferBuilder fbb;
              auto req_offset = ::iroha::CreateAssetQueryDirect(fbb,
                  q->pubKey()->c_str(),
                  q->ledger_name()->c_str(),
                  q->domain_name()->c_str(),
                  q->asset_name()->c_str(),
                  q->uncommitted()
              );

              fbb.Finish(req_offset);
              assets = connection::iroha::AssetRepositoryImpl::AccountGetAsset::receiver.invoke(
                      "from",  // TODO: Specify 'from'
                      fbb.ReleaseBufferPointer()
              );

              std::vector<uint8_t> types;
              std::vector<flatbuffers::Offset<::iroha::Asset>> res_assets;
              {
                  flatbuffers::FlatBufferBuilder fbb_;
                  for(const ::iroha::Asset* asset: assets){
                      if(asset->asset_type() == ::iroha::AnyAsset::Currency){
                          fbb_.Clear();
                          res_assets.push_back(::iroha::CreateAsset(fbb_,
                              asset->asset_type(),
                              ::iroha::CreateCurrencyDirect(fbb_,
                                  asset->asset_as_Currency()->currency_name()->c_str(),
                                  asset->asset_as_Currency()->domain_name()->c_str(),
                                  asset->asset_as_Currency()->ledger_name()->c_str(),
                                  asset->asset_as_Currency()->description()->c_str(),
                                  asset->asset_as_Currency()->amount()->c_str(),
                                  asset->asset_as_Currency()->precision()
                              ).Union()
                          ));
                      }
                  }
              }
              auto responseOffset = ::iroha::CreateAssetResponseDirect(
                    fbbResponse, "Success", ::iroha::Code::COMMIT,
                    &res_assets
              );
              fbbResponse.Finish(responseOffset);

              *responseRef = flatbuffers::BufferRef<AssetResponse>(
                      fbbResponse.GetBufferPointer(), fbbResponse.GetSize());
          }
          return Status::OK;
      }
  private:
      flatbuffers::Offset<::iroha::Signature> sign(
              flatbuffers::FlatBufferBuilder &fbb, const std::string &tx) {
          const auto stamp = datetime::unixtime();
          const auto hashWithTimestamp =
                  hash::sha3_256_hex(tx + std::to_string(stamp));
          const auto signature = signature::sign(
                  hashWithTimestamp,
                  config::PeerServiceConfig::getInstance().getMyPublicKey(),
                  config::PeerServiceConfig::getInstance().getMyPrivateKey());
          const std::vector<uint8_t> sigblob(signature.begin(), signature.end());
          return ::iroha::CreateSignatureDirect(
                  fbb, config::PeerServiceConfig::getInstance().getMyPublicKey().c_str(),
                  &sigblob, stamp);
      };

      flatbuffers::FlatBufferBuilder fbbResponse;
  };


  /************************************************************************************
   * Run server
   ************************************************************************************/
  std::mutex wait_for_server;
  ServerBuilder builder;
  grpc::Server *server = nullptr;
  std::condition_variable server_cv;
  bool server_ready = false;

  void initialize_peer() {
    // ToDo catch exception of to_string


    logger::info("Connection GRPC") << " initialize_peer ";
  }

  void wait_till_ready() {
    std::unique_lock<std::mutex> lk(wait_for_server);
    server_cv.wait(lk, [] { return server_ready; });
  }

  int run() {
    logger::info("Connection GRPC") << " RUN ";
    auto address =
      "0.0.0.0:" +
      std::to_string(
        config::IrohaConfigManager::getInstance().getGrpcPortNumber(50051));
    SumeragiConnectionServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    {
      std::lock_guard<std::mutex> lk(wait_for_server);
      //wait_for_server.lock();
      server = builder.BuildAndStart().release();
      server_ready = true;
      //wait_for_server.unlock();
    }
    server_cv.notify_one();

    server->Wait();
    return 0;
  }

  void finish() {
    server->Shutdown();
    delete server;
  }
} // namespace connection