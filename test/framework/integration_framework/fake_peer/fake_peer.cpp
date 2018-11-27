/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/fake_peer.hpp"

#include <boost/assert.hpp>
#include "consensus/yac/impl/yac_crypto_provider_impl.hpp"
#include "consensus/yac/transport/impl/network_impl.hpp"
#include "consensus/yac/transport/yac_network_interface.hpp"
#include "consensus/yac/yac_crypto_provider.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "cryptography/keypair.hpp"
#include "framework/integration_framework/fake_peer/behaviour/behaviour.hpp"
#include "framework/integration_framework/fake_peer/block_storage.hpp"
#include "framework/integration_framework/fake_peer/network/loader_grpc.hpp"
#include "framework/integration_framework/fake_peer/network/mst_network_notifier.hpp"
#include "framework/integration_framework/fake_peer/network/ordering_gate_network_notifier.hpp"
#include "framework/integration_framework/fake_peer/network/ordering_service_network_notifier.hpp"
#include "framework/integration_framework/fake_peer/network/yac_network_notifier.hpp"
#include "framework/result_fixture.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "main/server_runner.hpp"
#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"
#include "network/impl/async_grpc_client.hpp"
#include "ordering/impl/ordering_gate_transport_grpc.hpp"
#include "ordering/impl/ordering_service_transport_grpc.hpp"

using namespace shared_model::crypto;
using namespace framework::expected;

static std::shared_ptr<shared_model::interface::Peer> createPeer(
    const std::shared_ptr<shared_model::interface::CommonObjectsFactory>
        &common_objects_factory,
    const std::string &address,
    const PublicKey &key) {
  std::shared_ptr<shared_model::interface::Peer> peer;
  common_objects_factory->createPeer(address, key)
      .match(
          [&peer](iroha::expected::Result<
                  std::unique_ptr<shared_model::interface::Peer>,
                  std::string>::ValueType &result) {
            peer = std::move(result.value);
          },
          [&address](const iroha::expected::Result<
                     std::unique_ptr<shared_model::interface::Peer>,
                     std::string>::ErrorType &error) {
            BOOST_THROW_EXCEPTION(
                std::runtime_error("Failed to create peer object for peer "
                                   + address + ". " + error.error));
          });
  return peer;
}

namespace integration_framework {
  namespace fake_peer {

    FakePeer::FakePeer(
        const std::string &listen_ip,
        size_t internal_port,
        const boost::optional<Keypair> &key,
        const std::shared_ptr<shared_model::interface::Peer> &real_peer,
        const std::shared_ptr<shared_model::interface::CommonObjectsFactory>
            &common_objects_factory,
        std::shared_ptr<TransportFactoryType> transaction_factory,
        std::shared_ptr<shared_model::interface::TransactionBatchParser>
            batch_parser,
        std::shared_ptr<shared_model::interface::TransactionBatchFactory>
            transaction_batch_factory)
        : common_objects_factory_(common_objects_factory),
          listen_ip_(listen_ip),
          internal_port_(internal_port),
          keypair_(std::make_unique<Keypair>(
              key.value_or(DefaultCryptoAlgorithmType::generateKeypair()))),
          this_peer_(createPeer(
              common_objects_factory, getAddress(), keypair_->publicKey())),
          real_peer_(real_peer),
          async_call_(std::make_shared<AsyncCall>()),
          mst_transport_(
              std::make_shared<MstTransport>(async_call_,
                                             transaction_factory,
                                             batch_parser,
                                             transaction_batch_factory,
                                             keypair_->publicKey())),
          yac_transport_(std::make_shared<YacTransport>(async_call_)),
          os_transport_(std::make_shared<OsTransport>(transaction_batch_factory,
                                                      async_call_)),
          og_transport_(std::make_shared<OgTransport>(real_peer_->address(),
                                                      async_call_)),
          mst_network_notifier_(std::make_shared<MstNetworkNotifier>()),
          yac_network_notifier_(std::make_shared<YacNetworkNotifier>()),
          os_network_notifier_(std::make_shared<OsNetworkNotifier>()),
          og_network_notifier_(std::make_shared<OgNetworkNotifier>()),
          yac_crypto_(
              std::make_shared<iroha::consensus::yac::CryptoProviderImpl>(
                  *keypair_, common_objects_factory)) {
      mst_transport_->subscribe(mst_network_notifier_);
      yac_transport_->subscribe(yac_network_notifier_);
      os_transport_->subscribe(os_network_notifier_);
      og_transport_->subscribe(og_network_notifier_);
      log_ = logger::log(
          "IntegrationTestFramework "
          "(fake peer at "
          + getAddress() + ")");
    }

    FakePeer &FakePeer::setBehaviour(
        const std::shared_ptr<Behaviour> &behaviour) {
      behaviour_ = behaviour;
      behaviour_->adopt(shared_from_this());
      return *this;
    }

    const std::shared_ptr<Behaviour> &FakePeer::getBehaviour() const {
      return behaviour_;
    }

    FakePeer &FakePeer::setBlockStorage(
        const std::shared_ptr<BlockStorage> &block_storage) {
      if (block_storage_) {
        block_storage_->claimNotUsingPeer(shared_from_this());
      }
      block_storage_ = block_storage;
      block_storage_->claimUsingPeer(shared_from_this());
      return *this;
    }

    FakePeer &FakePeer::removeBlockStorage() {
      if (block_storage_) {
        block_storage_->claimNotUsingPeer(shared_from_this());
      }
      block_storage_.reset();
      return *this;
    }

    std::shared_ptr<BlockStorage> FakePeer::getBlockStorage() const {
      return block_storage_;
    }

    void FakePeer::run() {
      // start instance
      synchronizer_transport_ =
          std::make_shared<LoaderGrpc>(shared_from_this());
      log_->info("starting listening server");
      internal_server_ = std::make_unique<ServerRunner>(getAddress());
      internal_server_->append(yac_transport_)
          .append(mst_transport_)
          .append(os_transport_)
          .append(og_transport_)
          .append(synchronizer_transport_)
          .run()
          .match(
              [this](const iroha::expected::Result<int, std::string>::ValueType
                         &val) {
                const size_t bound_port = val.value;
                BOOST_VERIFY_MSG(
                    bound_port == internal_port_,
                    ("Server started on port " + std::to_string(bound_port)
                     + " instead of requested " + std::to_string(internal_port_)
                     + "!")
                        .c_str());
              },
              [this](const auto &err) {
                log_->error("coul not start server!");
              });
    }

    std::string FakePeer::getAddress() const {
      return listen_ip_ + ":" + std::to_string(internal_port_);
    }

    const Keypair &FakePeer::getKeypair() const {
      return *keypair_;
    }

    rxcpp::observable<MstMessagePtr> FakePeer::get_mst_states_observable() {
      return mst_network_notifier_->get_observable();
    }

    rxcpp::observable<YacMessagePtr> FakePeer::get_yac_states_observable() {
      return yac_network_notifier_->get_observable();
    }

    rxcpp::observable<OsBatchPtr> FakePeer::get_os_batches_observable() {
      return os_network_notifier_->get_observable();
    }

    rxcpp::observable<OgProposalPtr> FakePeer::get_og_proposals_observable() {
      return og_network_notifier_->get_observable();
    }

    std::shared_ptr<shared_model::interface::Signature> FakePeer::makeSignature(
        const shared_model::crypto::Blob &hash) const {
      auto bare_signature =
          shared_model::crypto::DefaultCryptoAlgorithmType::sign(hash,
                                                                 *keypair_);
      std::shared_ptr<shared_model::interface::Signature> signature_with_pubkey;
      common_objects_factory_
          ->createSignature(keypair_->publicKey(), bare_signature)
          .match([&signature_with_pubkey](
                     iroha::expected::Value<
                         std::unique_ptr<shared_model::interface::Signature>> &
                         sig) { signature_with_pubkey = std::move(sig.value); },
                 [this](iroha::expected::Error<std::string> &reason) {
                   BOOST_THROW_EXCEPTION(std::runtime_error(
                       "Cannot build signature: " + reason.error));
                 });
      return signature_with_pubkey;
    }

    iroha::consensus::yac::VoteMessage FakePeer::makeVote(
        iroha::consensus::yac::YacHash yac_hash) {
      iroha::consensus::yac::YacHash my_yac_hash = yac_hash;
      my_yac_hash.block_signature = makeSignature(
          shared_model::crypto::Blob(yac_hash.vote_hashes.block_hash));
      return yac_crypto_->getVote(my_yac_hash);
    }

    void FakePeer::sendMstState(const iroha::MstState &state) {
      mst_transport_->sendState(*real_peer_, state);
    }

    void FakePeer::sendYacState(
        const std::vector<iroha::consensus::yac::VoteMessage> &state) {
      yac_transport_->sendState(*real_peer_, state);
    }

    void FakePeer::voteForTheSame(const YacMessagePtr &incoming_votes) {
      using iroha::consensus::yac::VoteMessage;
      log_->debug("Got a YAC state message with {} votes.",
                  incoming_votes->size());
      if (incoming_votes->size() > 1) {
        // TODO mboldyrev 24/10/2018 IR-1821: rework ignoring states for
        //                                    accepted commits
        log_->debug(
            "Ignoring state with multiple votes, "
            "because it probably refers to an accepted commit.");
        return;
      }
      std::vector<VoteMessage> my_votes;
      my_votes.reserve(incoming_votes->size());
      std::transform(incoming_votes->cbegin(),
                     incoming_votes->cend(),
                     std::back_inserter(my_votes),
                     [this](const VoteMessage &incoming_vote) {
                       log_->debug(
                           "Sending agreement for proposal (Round ({}, {}), "
                           "hash ({}, {})).",
                           incoming_vote.hash.vote_round.block_round,
                           incoming_vote.hash.vote_round.reject_round,
                           incoming_vote.hash.vote_hashes.proposal_hash,
                           incoming_vote.hash.vote_hashes.block_hash);
                       return makeVote(incoming_vote.hash);
                     });
      sendYacState(my_votes);
    }

    void FakePeer::sendProposal(
        std::unique_ptr<shared_model::interface::Proposal> proposal) {
      os_transport_->publishProposal(std::move(proposal),
                                     {real_peer_->address()});
    }

    void FakePeer::sendBatch(
        const std::shared_ptr<shared_model::interface::TransactionBatch>
            &batch) {
      og_transport_->propagateBatch(batch);
    }

    bool FakePeer::sendBlockRequest(const LoaderBlockRequest &request) {
      return synchronizer_transport_->sendBlockRequest(real_peer_->address(),
                                                       request);
    }

    size_t FakePeer::sendBlocksRequest(const LoaderBlocksRequest &request) {
      return synchronizer_transport_->sendBlocksRequest(real_peer_->address(),
                                                        request);
    }

  }  // namespace fake_peer
}  // namespace integration_framework
