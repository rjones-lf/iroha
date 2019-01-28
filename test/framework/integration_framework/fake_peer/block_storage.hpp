/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_BLOCK_STORAGE_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_BLOCK_STORAGE_HPP_

#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include <boost/thread/shared_mutex.hpp>
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
      using BlockPtr = std::shared_ptr<const shared_model::proto::Block>;
      using HeightType = shared_model::interface::types::HeightType;
      using HashType = shared_model::crypto::Hash;

      BlockStorage();
      BlockStorage(const BlockStorage &);
      BlockStorage(BlockStorage &&);
      BlockStorage operator=(const BlockStorage &) = delete;
      BlockStorage operator=(BlockStorage &&) = delete;

      void storeBlock(const BlockPtr &block);

      BlockPtr getBlockByHeight(HeightType height) const;
      BlockPtr getBlockByHash(const HashType &hash) const;
      BlockPtr getTopBlock() const;

     private:
      std::unordered_map<HeightType, BlockPtr> blocks_by_height_;
      std::unordered_map<HashType, BlockPtr, HashType::Hasher> blocks_by_hash_;
      mutable std::shared_timed_mutex block_maps_mutex_;

      logger::Logger log_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_BLOCK_STORAGE_HPP_ */
