/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BLOCK_STORAGE_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BLOCK_STORAGE_HPP_

#include <unordered_map>
#include <vector>

#include "cryptography/hash.hpp"
#include "interfaces/common_objects/types.hpp"
#include "logger/logger.hpp"

namespace shared_model {
  namespace proto {
    class Block;
  }
}  // namespace shared_model

namespace integration_framework {
  namespace fake_peer {
    class FakePeer;

    class BlockStorage final {
     public:
      using BlockPtr = std::shared_ptr<shared_model::proto::Block>;
      using HeightType = shared_model::interface::types::HeightType;
      using HashType = shared_model::crypto::Hash;

      void storeBlock(const BlockPtr &block);

      BlockPtr getBlockByHeight(HeightType height) const;
      BlockPtr getBlockByHash(const HashType &hash) const;
      BlockPtr getTopBlock() const;

      // Claim that a fake peer uses this storage. Used for logging.
      void claimUsingPeer(const std::shared_ptr<FakePeer> &peer);

      // claim that a fake peer is not using this storage any more.
      void claimNotUsingPeer(const std::shared_ptr<FakePeer> &peer);

     private:
      logger::Logger getLogger() const;

      std::unordered_map<HeightType, BlockPtr> blocks_by_height_;
      std::unordered_map<HashType, BlockPtr, HashType::Hasher> blocks_by_hash_;

      /// The collection of peers claiming to use this block storage.
      // I am using vector (although a kind of set would fit better) because
      // weak pointers seem unsuitable for any comparison, as the shared pointer
      // they point to may change.
      mutable std::vector<std::weak_ptr<FakePeer>> using_peers_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BLOCK_STORAGE_HPP_ */
