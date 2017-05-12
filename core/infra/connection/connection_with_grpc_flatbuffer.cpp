
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

namespace connection {
  /**
   * Using
   */
  using Sumeragi = ::iroha::Sumeragi;
  using Hijiri = ::iroha::Hijiri;
  using ConsensusEvent = ::iroha::ConsensusEvent;
  using Ping = ::iroha::Ping;
  using Response = ::iroha::Response;
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
      // ToDo: Copy consensus event twice in toConsensusEvent() and
      // copyConsensusEvent(). What's best solution?
      auto eventOffset =
        flatbuffer_service::copyConsensusEvent(fbb, consensusEvent);
      if (!eventOffset) {
        // FIXME:
        // RPCのエラーではないが、Responseで返すべきか、makeUnexpectedで返すべきか
        // ERROR;
        assert(false);  // ToDo: Do error-handling
      }

      fbb.Finish(*eventOffset);

      auto requestConsensusEventRef =
        flatbuffers::BufferRef<::iroha::ConsensusEvent>(fbb.GetBufferPointer(),
                                                        fbb.GetSize());

      Status status =
        stub_->Verify(&context, requestConsensusEventRef, &responseRef);

      if (status.ok()) {
        logger::info("connection")
          << "response: " << responseRef.GetRoot()->message()->c_str();
        return responseRef.GetRoot();
      } else {
        logger::error("connection") << static_cast<int>(status.error_code())
                                    << ": " << status.error_message();
        return responseRef.GetRoot();
        // std::cout << status.error_code() << ": " << status.error_message();
        // return {"RPC failed", RESPONSE_ERRCONN};
      }
    }

    const ::iroha::Response *Torii(
      const ::iroha::Transaction &transaction) const {
      // Copy transaction to FlatBufferBuilder memory, then create
      // BufferRef<Transaction>
      // and share it to another sumeragi by using stub interface Torii.

      ::grpc::ClientContext clientContext;
      flatbuffers::FlatBufferBuilder fbbTransaction;

      std::vector<uint8_t> hashBlob(transaction.hash()->begin(),
                                    transaction.hash()->end());

      std::vector<flatbuffers::Offset<::iroha::Signature>> signatures;
      for (auto &&txSig : *transaction.signatures()) {
        std::vector<uint8_t> data(*txSig->signature()->begin(),
                                  *txSig->signature()->end());
        signatures.emplace_back(::iroha::CreateSignatureDirect(
          fbbTransaction, txSig->publicKey()->c_str(), &data));
      }

      auto commandOffset = [&] {
        std::size_t length = 0;
        auto commandBuf = extractCommandBuffer(transaction, length);
        flatbuffers::Verifier verifier(commandBuf.get(), length);
        ::iroha::VerifyCommand(verifier, commandBuf.get(),
                               transaction.command_type());
        return flatbuffer_service::CreateCommandDirect(
          fbbTransaction, commandBuf.get(), transaction.command_type());
      }();

      std::vector<uint8_t> attachmentData(
        *transaction.attachment()->data()->begin(),
        *transaction.attachment()->data()->end());

      auto transactionOffset = ::iroha::CreateTransactionDirect(
        fbbTransaction, transaction.creatorPubKey()->c_str(),
        transaction.command_type(), commandOffset, &signatures, &hashBlob,
        transaction.timestamp(),
        ::iroha::CreateAttachmentDirect(
          fbbTransaction, transaction.attachment()->mime()->c_str(),
          &attachmentData
        ));

      fbbTransaction.Finish(transactionOffset);

      flatbuffers::BufferRef<::iroha::Transaction> reqTransactionRef(
        fbbTransaction.GetBufferPointer(), fbbTransaction.GetSize());

      flatbuffers::BufferRef<Response> responseRef;

      Status status =
        stub_->Torii(&clientContext, reqTransactionRef, &responseRef);

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
    flatbuffers::unique_ptr_t extractCommandBuffer(
      const ::iroha::Transaction &transaction, std::size_t &length) {
      flatbuffers::FlatBufferBuilder fbbCommand;
      auto type = transaction.command_type();
      auto obj = transaction.command();
      auto commandOffset =
        flatbuffer_service::CreateCommandDirect(fbbCommand, obj, type);
      fbbCommand.Finish(commandOffset);
      length = fbbCommand.GetSize();
      return fbbCommand.ReleaseBufferPointer();
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
            0);  // ToDo: Currently, if it fails, no signature.
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

      auto responseOffset = ::iroha::CreateResponseDirect(
        fbbResponse, "OK!!", ::iroha::Code::UNDECIDED,
        sign(fbbResponse,
             hash::sha3_256_hex(flatbuffer_service::toString(
               *request->GetRoot()
                 ->transactions()
                 ->Get(0)
                 ->tx_nested_root()))));  // ToDo: #(tx) = 1, ToDo:

      fbbResponse.Finish(responseOffset);

      *response = flatbuffers::BufferRef<::iroha::Response>(
        fbbResponse.GetBufferPointer(), fbbResponse.GetSize());

      return Status::OK;
    }

    Status Torii(ServerContext *context,
                 const flatbuffers::BufferRef<Transaction> *transactionRef,
                 flatbuffers::BufferRef<Response> *responseRef) override {
      logger::debug("SumeragiConnectionServiceImpl::Torii") << "RPC works";

      // This reference remains until next calling
      // SumeragiConnectionServiceImpl::Torii() method.
      fbbResponse.Clear();

      {
        const auto tx = transactionRef->GetRoot();
        flatbuffers::FlatBufferBuilder fbb;
        auto txoffset = flatbuffer_service::copyTransaction(fbb, *tx);
        if (!txoffset) {
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

      auto responseOffset = ::iroha::CreateResponseDirect(
        fbbResponse, "OK!!", ::iroha::Code::UNDECIDED,
        sign(fbbResponse, hash::sha3_256_hex(flatbuffer_service::toString(
          *transactionRef->GetRoot()))));
      fbbResponse.Finish(responseOffset);

      *responseRef = flatbuffers::BufferRef<Response>(
        fbbResponse.GetBufferPointer(), fbbResponse.GetSize());

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
   * Interface: Verify::send()
   ************************************************************************************/
  namespace iroha {
    namespace SumeragiImpl {
      namespace Verify {
        bool send(const std::string &ip, const ::iroha::ConsensusEvent &event) {
          logger::info("Connection with grpc") << "Send!";
          if (::peer::service::isExistIP(ip)) {
            logger::info("Connection with grpc") << "IP exists: " << ip;
            SumeragiConnectionClient client(grpc::CreateChannel(
              ip + ":" +
              std::to_string(config::IrohaConfigManager::getInstance()
                               .getGrpcPortNumber(50051)),
              grpc::InsecureChannelCredentials()));
            // TODO return tx validity
            auto reply = client.Verify(event);
            return true;
          } else {
            logger::info("Connection with grpc") << "IP doesn't exist: " << ip;
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

  namespace MemberShipService {
    namespace HijiriImpl {
      namespace Kagami {
        bool send(const std::string &ip, const ::iroha::Ping &ping) {  // TODO
          logger::info("Connection with grpc") << "Send!";
          logger::info("Connection with grpc") << "IP exists: " << ip;
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
  }  // namespace MemberShipService

  /************************************************************************************
   * Run server
   ************************************************************************************/
  std::mutex wait_for_server;
  ServerBuilder builder;
  grpc::Server *server = nullptr;
  std::condition_variable server_cv;

  void initialize_peer() {
    // ToDo catch exception of to_string


    logger::info("Connection GRPC") << " initialize_peer ";
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

    wait_for_server.lock();
    server = builder.BuildAndStart().release();
    wait_for_server.unlock();
    server_cv.notify_one();

    server->Wait();
    return 0;
  }

  void finish() {
    server->Shutdown();
    delete server;
  }
} // namespace connection