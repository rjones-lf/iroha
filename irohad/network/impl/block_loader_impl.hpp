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

#ifndef IROHA_BLOCK_LOADER_IMPL_HPP
#define IROHA_BLOCK_LOADER_IMPL_HPP

#include "network/block_loader.hpp"

#include <unordered_map>

#include "ametsuchi/block_query.hpp"
#include "ametsuchi/peer_query.hpp"
#include "loader.grpc.pb.h"
#include "logger/logger.hpp"
#include "model/model_crypto_provider.hpp"
#include "validators/block_validator.hpp"
#include "validators/default_validator.hpp"

namespace iroha {
  namespace network {
    class BlockLoaderImpl : public BlockLoader {
     public:
      BlockLoaderImpl(
          std::shared_ptr<ametsuchi::PeerQuery> peer_query,
          std::shared_ptr<ametsuchi::BlockQuery> block_query,
          std::shared_ptr<model::ModelCryptoProvider> crypto_provider,
          std::shared_ptr<shared_model::validation::DefaultBlockValidator> =
              std::make_shared<shared_model::validation::DefaultBlockValidator>());

      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      retrieveBlocks(
          const shared_model::crypto::PublicKey &peer_pubkey) override;

      boost::optional<std::shared_ptr<shared_model::interface::Block>>
      retrieveBlock(
          const shared_model::crypto::PublicKey &peer_pubkey,
          const shared_model::interface::types::HashType &block_hash) override;

     private:
      /**
       * Retrieve peers from database, and find the requested peer by pubkey
       * @param pubkey - public key of requested peer
       * @return peer, if it was found, otherwise nullopt
       * TODO 14/02/17 (@l4l) IR-960 rework method with returning result
       */
      boost::optional<model::Peer> findPeer(
          const shared_model::crypto::PublicKey &pubkey);
      /**
       * Get or create a RPC stub for connecting to peer
       * @param peer for connecting
       * @return RPC stub
       */
      proto::Loader::Stub &getPeerStub(const model::Peer &peer);

      std::unordered_map<model::Peer, std::unique_ptr<proto::Loader::Stub>>
          peer_connections_;
      std::shared_ptr<ametsuchi::PeerQuery> peer_query_;
      std::shared_ptr<ametsuchi::BlockQuery> block_query_;
      std::shared_ptr<model::ModelCryptoProvider> crypto_provider_;
      std::shared_ptr<shared_model::validation::DefaultBlockValidator>
          stateless_validator_;

      logger::Logger log_;
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_BLOCK_LOADER_IMPL_HPP
