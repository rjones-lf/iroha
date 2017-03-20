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

#include <repository/transaction_repository.hpp>

#include <consensus/consensus_event.hpp>
#include <crypto/base64.hpp>
#include <infra/protobuf/api.pb.h>
#include <repository/world_state_repository.hpp>

namespace repository{
    namespace transaction {

        using Api::Transaction;

        bool add(const std::string &hash,const Transaction& tx){
            return world_state_repository::add("transaction_" + hash, tx.SerializeAsString());
        }

        std::vector<Transaction> findAll(){
            std::vector<Transaction> res;
            auto txstr = world_state_repository::findByPrefix("transaction_");
            for(auto txs: txstr){
                Transaction tx;
                tx.ParseFromString(txs);
                res.push_back(tx);
            }
            return res;
        }

        Transaction find(std::string hash){
            std::vector<Transaction> res;
            Transaction tx;
            if(world_state_repository::exists("transaction_" + hash)){
                tx.ParseFromString(world_state_repository::find("transaction_" + hash));
            }
            return tx;
        }

    }
}