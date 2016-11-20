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
#include "../../consensus/event.hpp"
#include "../../service/json_parse.hpp"

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

    using Object = json_parse::Object;
    using Rule = json_parse::Rule;
    using Type = json_parse::Type;

    Object dump() {
        Object obj = Object(Type::DICT);
        obj.dictSub.insert( std::make_pair( "hash", Object(Type::STR, hash)));
        obj.dictSub.insert( std::make_pair( "parent", Object(Type::STR, parent)));
        obj.dictSub.insert( std::make_pair( "leftChild",  Object(Type::STR, left)));
        obj.dictSub.insert( std::make_pair( "rightChild", Object(Type::STR, right)));
        return obj;
    }

    Rule getJsonParseRule() {
        auto rule = Rule(Type::DICT);
        rule.dictSub.insert( std::make_pair("hash", Rule(Type::STR)));
        rule.dictSub.insert( std::make_pair("parent", Rule(Type::STR)));
        rule.dictSub.insert( std::make_pair("leftChild", Rule(Type::STR)));
        rule.dictSub.insert( std::make_pair("rightChild", Rule(Type::STR)));
        return std::move(rule);
    }
};

//TODO: change bool to throw an exception instead
bool commit(const std::unique_ptr<event::Event>& event){

};

bool leafExists(const std::string& hash){

}

std::string getLeaf(const std::string& hash){

}

template <typename T>
std::string calculateNewRoot(
    const std::unique_ptr<event::Event>& event,
    std::vector<std::tuple<std::string,std::string>> &batchCommit
);

};  // namespace merkle_transaction_repository

#endif  // CORE_REPOSITORY_MERKLETRANSACTIONREPOSITORY_HPP_
