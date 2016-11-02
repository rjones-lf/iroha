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
#include "../../service/json_parse.hpp"

namespace merkle_transaction_repository {

    // Think in progress ^^;
//struct MerkleNode {
//    std::string hash;
//    std::string parent;
//    std::string leftChild;
//    std::string rightChild;
//
//    MerkleNode() {
//
//    }
//
//    MerkleNode(
//        const std::string hash,
//        const std::string parent,
//        const std::string leftChild,
//        const std::string rightChild
//    ):
//            hash(hash),
//            parent(parent),
//            left(leftChild),
//            right(rightChild)
//    {}
//
//    bool isRoot() {
//        return parent.empty();
//    }
//
//    bool isLeaf() {
//        return leftChild.empty() && rightChild.empty();
//    }
//
//    using Object = json_parse::Object;
//    using Rule = json_parse::Rule;
//    using Type = json_parse::Type;
//    Object dump() {
//        Object obj = Object(Type::DICT);
//        obj.dictSub["hash"] =  Object(Type::STR, hash);
//        obj.dictSub["parent"] =  Object(Type::STR, parent);
//        obj.dictSub["leftChild"] =  Object(Type::STR, leftChild);
//        obj.dictSub["rightChild"] =  Object(Type::STR, rightChild);
//        return obj;
//    }
//
//    static Rule getJsonParseRule() {
//        Rule obj = Rule(Type::DICT);
//        obj.dictSub["hash"] =  Rule(Type::STR);
//        obj.dictSub["parent"] =  Rule(Type::STR);
//        obj.dictSub["leftChild"] = Rule(Type::STR);
//        obj.dictSub["rightChild"] = Rule(Type::STR);
//        return obj;
//    }
//};

//TODO: change bool to throw an exception instead
template <typename T,typename U>
bool commit(const std::unique_ptr<consensus_event::ConsensusEvent<T,U>> &event){

};

bool leafExists(const std::string& hash){

}

std::string getLeaf(const std::string& hash){

}

template <typename T,typename U>
std::string calculateNewRoot(const std::unique_ptr<consensus_event::ConsensusEvent<T,U>> &event, std::vector<std::tuple<std::string, std::string>> &batchCommit);

};  // namespace merkle_transaction_repository

#endif  // CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
