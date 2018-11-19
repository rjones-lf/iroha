/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_TYPES_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_TYPES_HPP_

#include <functional>
#include <memory>
#include <vector>

#include <boost/optional.hpp>
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace crypto {
    class Keypair;
    class Hash;
  }
  namespace interface {
    class CommonObjectsFactory;
    class Proposal;
    class Transaction;
    class TransactionBatch;
    class TransactionBatchParser;
    class TransactionBatchFactory;
  }  // namespace interface
  namespace proto {
    class Block;
    class Proposal;
  }
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {
    class TxPresenceCache;
  }  // namespace ametsuchi
  namespace protocol {
    class Transaction;
  }
  namespace network {
    class MstTransportGrpc;
    class MstTransportNotification;
  }  // namespace network
  namespace consensus {
    namespace yac {
      class NetworkImpl;
      class YacCryptoProvider;
      class YacHash;
      struct VoteMessage;
    }  // namespace yac
    struct Round;
  }    // namespace consensus
  namespace ordering {
    namespace transport {
      class OnDemandOsServerGrpc;
    }
    class OrderingGateTransportGrpc;
    class OrderingServiceTransportGrpc;
  }  // namespace ordering
  class MstState;
}  // namespace iroha

namespace integration_framework {
  namespace fake_peer {
    class Behaviour;
    class BlockStorage;
    class ProposalStorage;
    class FakePeer;
    class LoaderGrpc;
    class MstNetworkNotifier;
    class OgNetworkNotifier;
    class OsNetworkNotifier;
    class OnDemandOsNetworkNotifier;
    class YacNetworkNotifier;
    struct BatchesForRound;
    struct MstMessage;

    using MstMessagePtr = std::shared_ptr<MstMessage>;
    using YacMessagePtr =
        std::shared_ptr<const std::vector<iroha::consensus::yac::VoteMessage>>;
    using OgProposalPtr = std::shared_ptr<shared_model::interface::Proposal>;
    using OsBatchPtr =
        std::shared_ptr<shared_model::interface::TransactionBatch>;
    using LoaderBlockRequest = std::shared_ptr<shared_model::crypto::Hash>;
    using LoaderBlocksRequest = shared_model::interface::types::HeightType;
    using LoaderBlockRequestResult =
        boost::optional<const shared_model::proto::Block &>;
    using LoaderBlocksRequestResult =
        std::vector<std::reference_wrapper<const shared_model::proto::Block>>;
    using OrderingProposalRequest = iroha::consensus::Round;
    using OrderingProposalRequestResult =
        boost::optional<shared_model::proto::Proposal &>;

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_TYPES_HPP_ */
