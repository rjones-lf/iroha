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

#include "network/impl/block_loader_impl.hpp"
#include <grpc++/create_channel.h>

using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace iroha::network;

BlockLoaderImpl::BlockLoaderImpl(
    std::shared_ptr<PeerQuery> peer_query,
    std::shared_ptr<BlockQuery> block_query)
    : peer_query_(std::move(peer_query)),
      block_query_(std::move(block_query)) {
  log_ = logger::log("BlockLoaderImpl");
}

rxcpp::observable<Block> BlockLoaderImpl::retrieveBlocks(
    model::Peer::KeyType peer_pubkey) {
  return rxcpp::observable<>::create<Block>(
      [this, peer_pubkey](auto subscriber) {
        nonstd::optional<Block> top_block;
        block_query_->getTopBlocks(1)
            .as_blocking()
            .subscribe([&top_block](auto block) {
              top_block = block;
            });
        if (not top_block.has_value()) {
          log_->error("Failed to retrieve top block");
          subscriber.on_completed();
        }

        auto peer = this->findPeer(peer_pubkey);
        if (not peer.has_value()) {
          log_->error("Cannot find peer");
          subscriber.on_completed();
        }

        proto::BlocksRequest request;
        grpc::ClientContext context;
        protocol::Block block;

        // request next block to our top
        request.set_height(top_block->height + 1);

        auto reader =
            this->getPeerStub(peer.value()).retrieveBlocks(&context, request);
        while (reader->Read(&block)) {
          subscriber.on_next(factory_.deserialize(block));
        }
        reader->Finish();
        subscriber.on_completed();
      });
}

nonstd::optional<Block> BlockLoaderImpl::retrieveBlock(
    Peer::KeyType peer_pubkey, Block::HashType block_hash) {
  auto peer = findPeer(peer_pubkey);
  if (not peer.has_value()) {
    log_->error("Cannot find peer");
    return nonstd::nullopt;
  }

  proto::BlockRequest request;
  grpc::ClientContext context;
  protocol::Block block;

  // request block with specified hash
  request.set_hash(block_hash.to_string());

  auto status =
      getPeerStub(peer.value()).retrieveBlock(&context, request, &block);
  if (not status.ok()) {
    log_->error(status.error_message());
    return nonstd::nullopt;
  }
  return factory_.deserialize(block);
}

nonstd::optional<Peer> BlockLoaderImpl::findPeer(Peer::KeyType pubkey) {
  auto peers = peer_query_->getLedgerPeers();
  if (not peers.has_value()) {
    log_->error("Failed to retrieve peers");
    return nonstd::nullopt;
  }

  auto it = std::find_if(
      peers.value().begin(), peers.value().end(),
      [pubkey](auto peer) { return peer.pubkey == pubkey; });
  if (it == peers.value().end()) {
    log_->error("Failed to find requested peer");
    return nonstd::nullopt;
  }

  return *it;
}

proto::Loader::Stub &BlockLoaderImpl::getPeerStub(const Peer &peer) {
  auto it = peer_connections_.find(peer);
  if (it == peer_connections_.end()) {
    it = peer_connections_.insert(std::make_pair(peer, proto::Loader::NewStub(
        grpc::CreateChannel(peer.address,
                            grpc::InsecureChannelCredentials())))).first;
  }
  return *it->second;
}
