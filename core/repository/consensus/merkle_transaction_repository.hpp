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
#include "../../consensus/consensus_event.hpp"

namespace merkle_transaction_repository {


struct MerkleNode {
    std::string hash;
    std::string parent;
    std::string left;
    std::string right;

    MerkleNode() {

    }

    MerkleNode(
        const std::string hash,
        const std::string parent,
        const std::string leftChild,
        const std::string rightChild
    ):
            hash(hash),
            parent(parent),
            left(leftChild),
            right(rightChild)
    {}

    bool isRoot() {
       return parent.empty();
    }

    bool isLeaf() {
       return left.empty() && right.empty();
    }

};

//TODO: change bool to throw an exception instead
bool commit(const Event::ConsensusEvent& event){
    return false; // TODO: fill this function
};

bool leafExists(const std::string& hash){
    return false; // TODO: fill this function
}

std::string getLeaf(const std::string& hash){
    return ""; // TODO: fill this function
}

template <typename T>
std::string calculateNewRoot(
    const Event::ConsensusEvent& event,
    std::vector<std::tuple<std::string,std::string>> &batchCommit
);

};  // namespace merkle_transaction_repository

#endif  // CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
