/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <grpc++/create_channel.h>
#include <algorithm>

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/from_old_model.hpp"
#include "interfaces/common_objects/peer.hpp"
#include "network/impl/block_loader_impl.hpp"

using iroha::Wrapper;
using iroha::makeWrapper;
using namespace iroha::ametsuchi;
using namespace iroha::network;
using namespace shared_model::crypto;
using namespace shared_model::interface;

BlockLoaderImpl::BlockLoaderImpl(
    std::shared_ptr<PeerQuery> peer_query,
    std::shared_ptr<BlockQuery> block_query,
    std::shared_ptr<model::ModelCryptoProvider> crypto_provider)
    : peer_query_(std::move(peer_query)),
      block_query_(std::move(block_query)),
      crypto_provider_(crypto_provider) {
  log_ = logger::log("BlockLoaderImpl");
}

const char *kPeerNotFound = "Cannot find peer";
const char *kTopBlockRetrieveFail = "Failed to retrieve top block";
const char *kInvalidBlockSignatures = "Block signatures are invalid";
const char *kPeerRetrieveFail = "Failed to retrieve peers";
const char *kPeerFindFail = "Failed to find requested peer";

rxcpp::observable<Wrapper<Block>> BlockLoaderImpl::retrieveBlocks(
    const PublicKey &peer_pubkey) {
  return rxcpp::observable<>::create<Wrapper<Block>>([this, peer_pubkey](
                                                         auto subscriber) {
    nonstd::optional<iroha::model::Block> top_block;
    block_query_->getTopBlocks(1)
        .subscribe_on(rxcpp::observe_on_new_thread())
        .as_blocking()
        .subscribe([&top_block](auto block) {
          top_block =
              *std::unique_ptr<iroha::model::Block>(block->makeOldModel());
        });
    if (not top_block.has_value()) {
      log_->error(kTopBlockRetrieveFail);
      subscriber.on_completed();
      return;
    }

    auto peer = this->findPeer(peer_pubkey);
    if (not peer.has_value()) {
      log_->error(kPeerNotFound);
      subscriber.on_completed();
      return;
    }

    proto::BlocksRequest request;
    grpc::ClientContext context;
    protocol::Block block;

    // request next block to our top
    request.set_height(top_block->height + 1);

    auto reader =
        this->getPeerStub(peer.value()).retrieveBlocks(&context, request);
    while (reader->Read(&block)) {
      auto result = makeWrapper<Block, shared_model::proto::Block>(block);
      std::unique_ptr<iroha::model::Block> old_block(result->makeOldModel());
      if (not crypto_provider_->verify(*old_block)) {
        log_->error(kInvalidBlockSignatures);
        context.TryCancel();
      } else {
        subscriber.on_next(std::move(result));
      }
    }
    reader->Finish();
    subscriber.on_completed();
  });
}

nonstd::optional<Wrapper<Block>> BlockLoaderImpl::retrieveBlock(
    const PublicKey &peer_pubkey, const types::HashType &block_hash) {
  auto peer = findPeer(peer_pubkey);
  if (not peer.has_value()) {
    log_->error(kPeerNotFound);
    return nonstd::nullopt;
  }

  proto::BlockRequest request;
  grpc::ClientContext context;
  protocol::Block block;

  // request block with specified hash
  request.set_hash(toBinaryString(block_hash));

  auto status =
      getPeerStub(peer.value()).retrieveBlock(&context, request, &block);
  if (not status.ok()) {
    log_->error(status.error_message());
    return nonstd::nullopt;
  }

  auto result = makeWrapper<Block, shared_model::proto::Block>(block);
  std::unique_ptr<iroha::model::Block> old_block(result->makeOldModel());
  if (not crypto_provider_->verify(*old_block)) {
    log_->error(kInvalidBlockSignatures);
    return nonstd::nullopt;
  }

  return nonstd::optional<Wrapper<Block>>(std::move(result));
}

nonstd::optional<iroha::model::Peer> BlockLoaderImpl::findPeer(const shared_model::crypto::PublicKey &pubkey) {
  auto peers = peer_query_->getLedgerPeers();
  if (not peers) {
    log_->error(kPeerRetrieveFail);
    return nonstd::nullopt;
  }

  auto &blob = pubkey.blob();
  auto it = std::find_if(
      peers.value().begin(), peers.value().end(), [&blob](const auto &peer) {
        return peer->pubkey().blob() == blob;
      });
  if (it == peers.value().end()) {
    log_->error(kPeerFindFail);
    return nonstd::nullopt;
  }

  return *std::unique_ptr<iroha::model::Peer>((*it)->makeOldModel());
}

proto::Loader::Stub &BlockLoaderImpl::getPeerStub(
    const iroha::model::Peer &peer) {
  auto it = peer_connections_.find(peer);
  if (it == peer_connections_.end()) {
    it = peer_connections_
             .insert(std::make_pair(
                 peer,
                 proto::Loader::NewStub(grpc::CreateChannel(
                     peer.address, grpc::InsecureChannelCredentials()))))
             .first;
  }
  return *it->second;
}
