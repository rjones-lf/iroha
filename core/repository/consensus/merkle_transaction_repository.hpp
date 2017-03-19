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

#ifndef CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
#define CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_

#include <string>
#include <memory>
#include <unordered_map>

#include <infra/protobuf/api.pb.h>

namespace merkle_transaction_repository {

  using Api::ConsensusEvent;

  struct MerkleNode {
    std::string hash;
    std::string parent;
    std::string left;
    std::string right;

    MerkleNode() {

    }

    MerkleNode(
        std::string hash,
        std::string parent,
        std::string leftChild,
        std::string rightChild
    ) :
        hash(std::move(hash)),
        parent(std::move(parent)),
        left(std::move(leftChild)),
        right(std::move(rightChild)) {}

    bool isRoot();

    bool isLeaf();

  };

//TODO: change bool to throw an exception instead
  bool commit(const ConsensusEvent &event);

  bool leafExists(const std::string &hash);

  std::string getLeaf(const std::string &hash);

  std::string calculateNewRoot(
      const ConsensusEvent &event,
      std::vector<std::tuple<std::string, std::string>> &batchCommit
  );

};  // namespace merkle_transaction_repository

#endif  // CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
