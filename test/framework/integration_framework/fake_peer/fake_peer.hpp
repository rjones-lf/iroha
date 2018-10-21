/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_HPP_

#include <memory>
#include <string>

#include <boost/core/noncopyable.hpp>
#include "interfaces/iroha_internal/abstract_transport_factory.hpp"
#include "logger/logger.hpp"

namespace shared_model {
  namespace crypto {
    class Keypair;
  }
  namespace interface {
    class Transaction;
    class TransactionBatchParser;
    class TransactionBatchFactory;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace protocol {
    class Transaction;
  }
  namespace network {
    class MstTransportGrpc;
    class MstTransportNotification;
  }  // namespace network
}  // namespace iroha
class ServerRunner;

namespace integration_framework {

  class FakePeer final : public boost::noncopyable {
   public:
    using TransportFactoryType =
        shared_model::interface::AbstractTransportFactory<
            shared_model::interface::Transaction,
            iroha::protocol::Transaction>;

    FakePeer(const std::string &listen_ip,
             size_t internal_port,
             const boost::optional<shared_model::crypto::Keypair> &key,
             std::shared_ptr<TransportFactoryType> transaction_factory,
             std::shared_ptr<shared_model::interface::TransactionBatchParser>
                 batch_parser,
             std::shared_ptr<shared_model::interface::TransactionBatchFactory>
                 transaction_batch_factory);

    void run();

    void subscribeForMstNotifications(
        std::shared_ptr<iroha::network::MstTransportNotification> notification);

    std::string getAddress() const;

    const shared_model::crypto::Keypair &getKeypair() const;

   private:
    const std::string listen_ip_;
    size_t internal_port_;
    std::unique_ptr<shared_model::crypto::Keypair> keypair_;
    std::shared_ptr<iroha::network::MstTransportGrpc> mst_transport_;
    std::unique_ptr<ServerRunner> internal_server_;
    logger::Logger log_;
  };

}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_HPP_ */
