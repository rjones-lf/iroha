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
#include <connection/connection.hpp>
#include <crypto/signature.hpp>
#include <infra/config/iroha_config_with_json.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <membership_service/peer_service.hpp>
#include <utils/expected.hpp>
#include <utils/logger.hpp>

#include <flatbuffers/flatbuffers.h>
#include <grpc++/grpc++.h>

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
using ConsensusEvent = ::iroha::ConsensusEvent;
using Response = ::iroha::Response;
using Transaction = ::iroha::Transaction;
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

// using Response = std::pair<std::string, ResponseType>;
/*
// TODO: very dirty solution, need to be out of here
std::function<RecieverConfirmation(const std::string&)> sign = [](const
std::string &hash) { RecieverConfirmation confirm; Signature signature;
    signature.set_publickey(config::PeerServiceConfig::getInstance().getMyPublicKey());
    signature.set_signature(signature::sign(
            config::PeerServiceConfig::getInstance().getMyPublicKey(),
            hash,
            config::PeerServiceConfig::getInstance().getMyPrivateKey())
    );
    confirm.set_hash(hash);
    confirm.mutable_signature()->Swap(&signature);
    return confirm;
};

std::function<bool(const RecieverConfirmation&)> valid = [](const
RecieverConfirmation &c) { return signature::verify(c.signature().signature(),
c.hash(), c.signature().publickey());
};
*/

/*
flatbuffers::Offset<::iroha::Signature> sign = [](const std::vector<uint8_t>&
hash) {
  // FIXME: Not implemented
  //return ::iroha::CreateSignatureDirect(fbb,
::peer::myself::getPublicKey().c_str(), )
}
*/

/**
 * Receiver
 * - stores callback function
 */
template <class CallBackFunc>
class Receiver {
 public:
  VoidHandler set(CallBackFunc&& rhs) {
    if (receiver_) {
      return makeUnexpected(exception::DuplicateSetArgumentException(
          "Receiver<" + std::string(typeid(CallBackFunc).name()) + ">",
          __FILE__));
    }

    receiver_ = std::make_shared<CallBackFunc>(rhs);
    return {};
  }

  // ToDo rewrite operator() overload.
  void invoke(const std::string& from, flatbuffers::unique_ptr_t&& arg) {
    (*receiver_)(from, std::move(arg));
  }

 private:
  std::shared_ptr<CallBackFunc> receiver_;
};

namespace iroha {
namespace SumeragiImpl {
namespace Verify {

Receiver<Verify::CallBackFunc> receiver;

}  // namespace Verify
}  // namespace SumeragiImpl
}  // namespace iroha

namespace iroha {
namespace SumeragiImpl {
namespace Torii {

Receiver<Torii::CallBackFunc> receiver;

}  // namespace Torii
}  // namespace SumeragiImpl
}  // namespace iroha

class SumeragiConnectionClient {
 public:
  explicit SumeragiConnectionClient(std::shared_ptr<Channel> channel)
      : stub_(Sumeragi::NewStub(channel)) {}

  const ::iroha::Response* Verify(
      const ::iroha::ConsensusEvent& consensusEvent) const {
    ClientContext context;
    flatbuffers::BufferRef<Response> responseRef;
    logger::info("connection") << "Operation";
    logger::info("connection")
        << "size: " << consensusEvent.peerSignatures()->size();

    // For now, ConsensusEvent has one transaction.
    const auto& transaction = *consensusEvent.transactions()->Get(0)->tx_nested_root();

    auto newConsensusEvent = flatbuffer_service::toConsensusEvent(transaction);

    flatbuffers::FlatBufferBuilder fbb;
    // ToDo: Copy consensus event twice in toConsensusEvent() and
    // copyConsensusEvent(). What's best solution?
    auto eventOffset =
        flatbuffer_service::copyConsensusEvent(fbb, consensusEvent);
    if (!eventOffset) {
      // FIXME:
      // RPCのエラーではないが、Responseで返すべきか、makeUnexpectedで返すべきか
      // ERROR;
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
      logger::error("connection")
          << status.error_code() << ": " << status.error_message();
      return responseRef.GetRoot();
      // std::cout << status.error_code() << ": " << status.error_message();
      // return {"RPC failed", RESPONSE_ERRCONN};
    }
  }

  const ::iroha::Response* Torii(
      const ::iroha::Transaction& transaction) const {
    // Copy transaction to FlatBufferBuilder memory, then create
    // BufferRef<Transaction>
    // and share it to another sumeragi by using stub interface Torii.

    ::grpc::ClientContext clientContext;
    flatbuffers::FlatBufferBuilder fbbTransaction;

    std::vector<uint8_t> hashBlob(transaction.hash()->begin(),
                                  transaction.hash()->end());

    std::vector<flatbuffers::Offset<::iroha::Signature>> signatures;
    for (auto&& txSig : *transaction.signatures()) {
      std::vector<uint8_t> data(*txSig->signature()->begin(),
                                *txSig->signature()->end());
      signatures.emplace_back(::iroha::CreateSignatureDirect(
          fbbTransaction, txSig->publicKey()->c_str(), &data));
    }

    // FIXED: leak reinterpret_cast<>(...)
    // -> extractCommandBuffer(calls flatbuffer_service::CreateCommandDirect())
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
        ::iroha::CreateAttachmentDirect(
            fbbTransaction, transaction.attachment()->mime()->c_str(),
            &attachmentData));

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
      logger::error("connection")
          << status.error_code() << ": " << status.error_message();
      // std::cout << status.error_code() << ": " << status.error_message();
      return responseRef.GetRoot();
    }
  }

 private:
  flatbuffers::unique_ptr_t extractCommandBuffer(
      const ::iroha::Transaction& transaction, std::size_t& length) {
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

class SumeragiConnectionServiceImpl final : public ::iroha::Sumeragi::Service {
 public:
  Status Verify(ServerContext* context,
                const flatbuffers::BufferRef<ConsensusEvent>* request,
                flatbuffers::BufferRef<Response>* response) override {
    assert(request->GetRoot()->peerSignatures() != nullptr);

    flatbuffers::FlatBufferBuilder fbbConsensusEvent;

    std::vector<flatbuffers::Offset<::iroha::Signature>> peerSignatures;
    for (const auto& aPeerSig : *request->GetRoot()->peerSignatures()) {
      std::vector<uint8_t> aPeerSigBlob(aPeerSig->signature()->begin(),
                                        aPeerSig->signature()->end());
      peerSignatures.push_back(::iroha::CreateSignatureDirect(
          fbbConsensusEvent, aPeerSig->publicKey()->c_str(), &aPeerSigBlob,
          1234567));
    }




    flatbuffers::FlatBufferBuilder fbbTransaction;

    auto tx = request->GetRoot()->transactions()->Get(0)->tx_nested_root();
    std::vector<flatbuffers::Offset<::iroha::Signature>> signatures;

    if (tx->signatures() != nullptr) {
      std::vector<uint8_t> signatureBlob(
          tx->signatures()->Get(0)->signature()->begin(),
          tx->signatures()->Get(0)->signature()->end());
      signatures.push_back(::iroha::CreateSignatureDirect(
          fbbTransaction, tx->signatures()->Get(0)->publicKey()->c_str(),
          &signatureBlob, 1234567));
    }
    std::vector<uint8_t> hashes;
    if (tx->hash() != nullptr) {
      hashes.assign(tx->hash()->begin(), tx->hash()->end());
    }

    flatbuffers::Offset<::iroha::Attachment> attachmentOffset;
    std::vector<uint8_t> data;
    if (tx->attachment() != nullptr && tx->attachment()->data() != nullptr &&
        tx->attachment()->mime() != nullptr) {
      data.assign(tx->attachment()->data()->begin(),
                  tx->attachment()->data()->end());

      attachmentOffset = ::iroha::CreateAttachmentDirect(
              fbbTransaction, tx->attachment()->mime()->c_str(), &data);
    }

    std::vector<flatbuffers::Offset<::iroha::TransactionWrapper>> transactions;

    auto newTx = ::iroha::CreateTransactionDirect(
        fbbTransaction, tx->creatorPubKey()->c_str(), tx->command_type(),
        flatbuffer_service::CreateCommandDirect(
            fbbTransaction, tx->command(), tx->command_type()),
        &signatures, &hashes, attachmentOffset
    );

    auto buf = fbbTransaction.GetBufferPointer();
    std::vector<uint8_t> buffer;
    buffer.assign(buf, buf + fbbTransaction.GetSize());

      // TODO: Currently, #(transaction) is one.
    transactions.push_back(::iroha::CreateTransactionWrapperDirect(
        fbbConsensusEvent,&buffer
    ));


    fbbConsensusEvent.Finish(::iroha::CreateConsensusEventDirect(
        fbbConsensusEvent, &peerSignatures, &transactions,
        request->GetRoot()->code())
    );

    const std::string from = "from";
    iroha::SumeragiImpl::Verify::receiver.invoke(
        from, std::move(fbbConsensusEvent.ReleaseBufferPointer()));

    fbbResponse.Clear();
    auto responseOffset = ::iroha::CreateResponseDirect(
        fbbResponse, "OK!!", ::iroha::Code::COMMIT, 0);
    fbbResponse.Finish(responseOffset);

    *response = flatbuffers::BufferRef<::iroha::Response>(
        fbbResponse.GetBufferPointer(), fbbResponse.GetSize());
    return Status::OK;
  }

  Status Torii(ServerContext* context,
               const flatbuffers::BufferRef<Transaction>* transactionRef,
               flatbuffers::BufferRef<Response>* responseRef) override {
    logger::debug("SumeragiConnectionServiceImpl::Torii") << "RPC works";

    {
      const auto transaction = transactionRef->GetRoot();

      flatbuffers::FlatBufferBuilder fbbTransaction;

      std::vector<flatbuffers::Offset<::iroha::Signature>> tx_signatures;

      if (transaction->signatures() != nullptr) {
        for (auto&& txSig : *transaction->signatures()) {
          std::vector<uint8_t> _data;
          if (txSig->signature() != nullptr) {
            for (auto d : *txSig->signature()) {
              _data.emplace_back(d);
            }
            tx_signatures.emplace_back(::iroha::CreateSignatureDirect(
                fbbTransaction, txSig->publicKey()->c_str(), &_data));
          }
        }
      }

      std::vector<uint8_t> _hash;
      if (transaction->hash() != nullptr) {
        _hash.assign(transaction->hash()->begin(), transaction->hash()->end());
      }


      std::vector<uint8_t> attachmentData;

      if (transaction->attachment() != nullptr &&
          transaction->attachment()->data() != nullptr) {
        attachmentData.assign(transaction->attachment()->data()->begin(),
                              transaction->attachment()->data()->end());
      }

      fbbTransaction.Finish(::iroha::CreateTransactionDirect(
          fbbTransaction, transaction->creatorPubKey()->c_str(),
          transaction->command_type(),
          flatbuffer_service::CreateCommandDirect(fbbTransaction,
                                                  transaction->command(),
                                                  transaction->command_type()),
          &tx_signatures, &_hash,
          ::iroha::CreateAttachmentDirect(
              fbbTransaction, transaction->attachment()->mime()->c_str(),
              &attachmentData)));

      iroha::SumeragiImpl::Torii::receiver.invoke(
          "from",  // TODO: Specify 'from'
          fbbTransaction.ReleaseBufferPointer());
    }

    // This reference remains until next calling
    // SumeragiConnectionServiceImpl::Torii() method.
    fbbResponse.Clear();

    auto responseOffset = ::iroha::CreateResponseDirect(
        fbbResponse, "OK!!", ::iroha::Code::COMMIT, 0);
    fbbResponse.Finish(responseOffset);

    *responseRef = flatbuffers::BufferRef<::iroha::Response>(
        fbbResponse.GetBufferPointer(), fbbResponse.GetSize());

    return Status::OK;
  }

 private:
  flatbuffers::FlatBufferBuilder fbbResponse;
};

namespace iroha {
namespace SumeragiImpl {
namespace Verify {

void receive(Verify::CallBackFunc&& callback) {
  receiver.set(std::move(callback));
}

bool send(const std::string& ip, const ::iroha::ConsensusEvent& event) {
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

bool sendAll(const ::iroha::ConsensusEvent& event) {
  auto receiver_ips = config::PeerServiceConfig::getInstance().getGroup();
  for (const auto& p : receiver_ips) {
    if (p["ip"].get<std::string>() !=
        config::PeerServiceConfig::getInstance().getMyIp()) {
      logger::info("connection") << "Send to " << p["ip"].get<std::string>();
      send(p["ip"].get<std::string>(), event);
    }
  }
  return true;
}
}  // namespace Verify
}  // namespace SumeragiImpl
}  // namespace iroha

namespace iroha {
namespace SumeragiImpl {
namespace Torii {

void receive(Torii::CallBackFunc&& callback) {
  receiver.set(std::move(callback));
}
}  // namespace Torii
}  // namespace SumeragiImpl
}  // namespace iroha

/************************************************************************************
 * Run server
 ************************************************************************************/
std::mutex wait_for_server;
ServerBuilder builder;
grpc::Server* server = nullptr;
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

}  // namespace connection
