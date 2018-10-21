/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/fake_peer.hpp"

#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "cryptography/keypair.hpp"
#include "main/server_runner.hpp"
#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"
#include "network/impl/async_grpc_client.hpp"

using namespace shared_model::crypto;

namespace integration_framework {

  FakePeer::FakePeer(
      const std::string &listen_ip,
      size_t internal_port,
      const boost::optional<Keypair> &key,
      std::shared_ptr<TransportFactoryType> transaction_factory,
      std::shared_ptr<shared_model::interface::TransactionBatchParser>
          batch_parser,
      std::shared_ptr<shared_model::interface::TransactionBatchFactory>
          transaction_batch_factory)
      : listen_ip_(listen_ip),
        internal_port_(internal_port),
        keypair_(std::make_unique<Keypair>(
            key.value_or(DefaultCryptoAlgorithmType::generateKeypair()))),
        mst_transport_(std::make_shared<iroha::network::MstTransportGrpc>(
            std::make_shared<
                iroha::network::AsyncGrpcClient<google::protobuf::Empty>>(),
            transaction_factory,
            batch_parser,
            transaction_batch_factory,
            keypair_->publicKey())),
        log_(logger::log("IntegrationTestFramework (fake peer at "
                         + getAddress() + ")")) {}

  void FakePeer::run() {
    // start instance
    log_->info("starting listening server");
    internal_server_ = std::make_unique<ServerRunner>(getAddress());
    internal_server_->append(mst_transport_).run();
  }

  void FakePeer::subscribeForMstNotifications(
      std::shared_ptr<iroha::network::MstTransportNotification> notification) {
    return mst_transport_->subscribe(notification);
  }

  std::string FakePeer::getAddress() const {
    return listen_ip_ + ":" + std::to_string(internal_port_);
  }

  const Keypair &FakePeer::getKeypair() const {
    return *keypair_.get();
  }

}  // namespace integration_framework
