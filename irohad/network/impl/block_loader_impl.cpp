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

#include "backend/protobuf/block.hpp"
#include "builders/protobuf/transport_builder.hpp"
#include "interfaces/common_objects/peer.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::network;
using namespace shared_model::crypto;
using namespace shared_model::interface;

BlockLoaderImpl::BlockLoaderImpl(
    std::shared_ptr<PeerQuery> peer_query,
    std::shared_ptr<BlockQuery> block_query,
    std::shared_ptr<shared_model::validation::DefaultBlockValidator>
        stateless_validator)
    : peer_query_(std::move(peer_query)),
      block_query_(std::move(block_query)),
      stateless_validator_(stateless_validator) {
  log_ = logger::log("BlockLoaderImpl");
}

const char *kPeerNotFound = "Cannot find peer";
const char *kTopBlockRetrieveFail = "Failed to retrieve top block";
const char *kPeerRetrieveFail = "Failed to retrieve peers";
const char *kPeerFindFail = "Failed to find requested peer";

rxcpp::observable<std::shared_ptr<Block>> BlockLoaderImpl::retrieveBlocks(
    const PublicKey &peer_pubkey) {
  return rxcpp::observable<>::create<std::shared_ptr<Block>>(
      [this, peer_pubkey](auto subscriber) {
        boost::optional<std::shared_ptr<Block>> top_block;
        block_query_->getTopBlocks(1)
            .subscribe_on(rxcpp::observe_on_new_thread())
            .as_blocking()
            .subscribe([&top_block](auto block) { top_block = block; });
        if (not top_block) {
          log_->error(kTopBlockRetrieveFail);
          subscriber.on_completed();
          return;
        }

        auto peer = this->findPeer(peer_pubkey);
        if (not peer) {
          log_->error(kPeerNotFound);
          subscriber.on_completed();
          return;
        }

        proto::BlocksRequest request;
        grpc::ClientContext context;
        protocol::Block block;

        // request next block to our top
        request.set_height((*top_block)->height() + 1);

        auto reader =
            this->getPeerStub(**peer).retrieveBlocks(&context, request);
        while (reader->Read(&block)) {
          shared_model::proto::TransportBuilder<
              shared_model::proto::Block,
              shared_model::validation::DefaultSignableBlockValidator>()
              .build(block)
              .match(
                  // success case
                  [&subscriber](
                      const iroha::expected::Value<shared_model::proto::Block>
                          &result) {
                    subscriber.on_next(
                        std::move(std::make_shared<shared_model::proto::Block>(
                            result.value)));
                  },
                  // fail case
                  [this,
                   &context](const iroha::expected::Error<std::string> &error) {
                    log_->error(error.error);
                    context.TryCancel();
                  });
        }
        reader->Finish();
        subscriber.on_completed();
      });
}

boost::optional<std::shared_ptr<Block>> BlockLoaderImpl::retrieveBlock(
    const PublicKey &peer_pubkey, const types::HashType &block_hash) {
  auto peer = findPeer(peer_pubkey);
  if (not peer) {
    log_->error(kPeerNotFound);
    return boost::none;
  }

  proto::BlockRequest request;
  grpc::ClientContext context;
  protocol::Block block;

  // request block with specified hash
  request.set_hash(toBinaryString(block_hash));

  auto status = getPeerStub(**peer).retrieveBlock(&context, request, &block);
  if (not status.ok()) {
    log_->warn(status.error_message());
    return boost::none;
  }

  // stateless validation of block
  auto result = std::make_shared<shared_model::proto::Block>(std::move(block));
  auto answer = stateless_validator_->validate(*result);
  if (answer.hasErrors()) {
    log_->error(answer.reason());
    return boost::none;
  }

  return boost::optional<std::shared_ptr<Block>>(std::move(result));
}

boost::optional<std::shared_ptr<shared_model::interface::Peer>>
BlockLoaderImpl::findPeer(const shared_model::crypto::PublicKey &pubkey) {
  auto peers = peer_query_->getLedgerPeers();
  if (not peers) {
    log_->error(kPeerRetrieveFail);
    return boost::none;
  }

  auto &blob = pubkey.blob();
  auto it = std::find_if(
      peers.value().begin(), peers.value().end(), [&blob](const auto &peer) {
        return peer->pubkey().blob() == blob;
      });
  if (it == peers.value().end()) {
    log_->error(kPeerFindFail);
    return boost::none;
  }
  return *it;
}

proto::Loader::Stub &BlockLoaderImpl::getPeerStub(
    const shared_model::interface::Peer &peer) {
  auto it = peer_connections_.find(peer.address());
  if (it == peer_connections_.end()) {
    it = peer_connections_
             .insert(std::make_pair(
                 peer.address(),
                 proto::Loader::NewStub(grpc::CreateChannel(
                     peer.address(), grpc::InsecureChannelCredentials()))))
             .first;
  }
  return *it->second;
}
