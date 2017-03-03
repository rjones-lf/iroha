/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <memory>
#include <iostream>
#include "merkle_transaction_repository.hpp"
#include "../world_state_repository.hpp"
#include <util/logger.hpp>
#include <crypto/hash.hpp>

#include <infra/protobuf/api.grpc.pb.h>

namespace merkle_transaction_repository {

    using Api::ConsensusEvent;

    template<typename T>
    std::string hash(const T&);

    template<>
    inline std::string hash<Api::Transaction>(const Api::Transaction& tx){
        return hash::sha3_256_hex(tx.SerializeAsString());
    }

    template<>
    inline std::string hash<std::string>(const std::string& s){
        return hash::sha3_256_hex(s);
    }

    std::string calculateNewRootHash(const ConsensusEvent& event,
                                     std::vector<std::tuple<std::string, std::string>> &batchCommit) {
        std::string lastInsertion = repository::world_state_repository::find("last_insertion");

        if (lastInsertion.empty()) {
            // Note: there is no need to update the tree's DB here, because there is only one transaction--the current!
            return hash(event.transaction());
        }

        std::string parent = repository::world_state_repository::find(lastInsertion + "_parent");
        std::string leftChild = repository::world_state_repository::find(parent + "_leftChild");
        std::string rightChild = repository::world_state_repository::find(parent + "_rightChild");

        if (rightChild.empty()) {
            // insert the event's transaction as the right child
            rightChild = hash(event.transaction());
            std::string newParentHash = hash(leftChild + rightChild);

            if (!batchCommit.empty()) { // TODO: this may not be the best comparison to use
                batchCommit.emplace_back(lastInsertion + "_rightChild", rightChild);
                batchCommit.emplace_back(lastInsertion + "_parent", rightChild);
            }

            // Propagate up the tree to the root
            while (!parent.empty()) {
                std::string newParentHash = hash(leftChild + rightChild);

                if (!batchCommit.empty()) { // TODO: this may not be the best comparison to use
                    batchCommit.emplace_back(newParentHash + "_parent", rightChild);
                    // TODO: delete old, unused nodes
                }

                // increment the right child and the parent, to move up the tree
                rightChild = newParentHash;
                parent = repository::world_state_repository::find(parent + "_parent");
            }

            if (!batchCommit.empty()) { // TODO: this may not be the best comparison to use
                batchCommit.emplace_back("merkle_root", rightChild);
                // TODO: delete old, unused nodes
            }
            return rightChild;

        } else {
            std::string newLeftChild = hash(event.transaction());
            std::string newParentHash = hash(newLeftChild);

            std::string oldParent = parent;

            // Propagate up the tree to the root
            while (!parent.empty()) {

                if (!batchCommit.empty()) { // TODO: this may not be the best comparison to use
                    batchCommit.emplace_back(hash(event.transaction()) + "_parent",
                                             newParentHash);
                    batchCommit.emplace_back(hash(event.transaction()) + "_leftChild",
                                             newLeftChild);
                    // TODO: delete old, unused nodes
                }

                // increment the right child and the parent, to move up the tree
                newLeftChild = newParentHash;
                newParentHash = hash(newLeftChild);

                oldParent = parent;
                parent = repository::world_state_repository::find(parent + "_parent");
            }

            newParentHash = hash(oldParent + newLeftChild);

            // save new root
            if (!batchCommit.empty()) { // TODO: this may not be the best comparison to use
                batchCommit.emplace_back(hash(event.transaction()) + "merkle_root",
                                         newParentHash);
                // TODO: delete old, unused nodes
            }

            return newParentHash;
        }
    }

    //TODO: change bool to throw an exception instead
    bool commit(const ConsensusEvent& event) {
        auto h = hash(event.transaction());
        std::vector<std::tuple<std::string, std::string>> batchCommit
                = {
                        std::make_tuple("last_insertion", h),
                        std::make_tuple(h, event.transaction().SerializeAsString())
                };

        calculateNewRootHash(event, batchCommit);

        return repository::world_state_repository::addBatch<std::string>(batchCommit);
    }

    bool leafExists(const std::string& hash) {
        return !repository::world_state_repository::find(hash).empty();
    }

    std::string getLeaf(const std::string& hash) {
        return repository::world_state_repository::find(hash);
    }

};  // namespace merkle_transaction_repository
