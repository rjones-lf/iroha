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

#include "../world_state_repository.hpp"
#include "transaction_repository.hpp"
#include "../../consensus/consensus_event.hpp"
#include "../../crypto/base64.hpp"
#include <string>
#include <vector>
#include <stdexcept>

namespace repository{

    namespace transaction {

        // ====================================
        // ||  This code is temporary ...    ||
        // ====================================
        // ||                                ||
        const auto s  = "840a7c5cb1078cb8778101a08d07c4b7f926c6d28c92259903eb2a5b8e96906a";
        const auto s2 = "659f22d9233e63527b311328da03336b14b6cf416cca52741fba9752553d0fa2";
        const auto s3 = "5c9887c793b7595885e004c43e19df429f619fb5d878a3334636e07e72a7adc2";
        const auto s4 = "e0e0dfd050ccbc1676681808117e0f5980028889ce328bd1c6b29424565c0913";

        std::vector<std::string> split(const std::string& str, const std::string& delim) noexcept{
            std::vector<std::string> result;
            if(str == ""){
                return result;
            }
            std::string::size_type pos = 0;
            while(pos != std::string::npos) {
                auto p = str.find(delim, pos);
                if(p == std::string::npos){
                    result.push_back(str.substr(pos));
                    break;
                }else{
                    result.push_back(str.substr(pos, p - pos));
                }
                pos = p + delim.size();
            }
            return result;
        }

        std::string t2s(Event::Transaction tx){
            std::string tsigs = "";
            for(const Event::TxSignatures& ts: tx.txsignatures()){
                tsigs += ts.publickey() + s3 + ts.signature() + s2;
            }
            std::string transaction = tx.type() + s2 + tx.receivepubkey() + s + tx.hash() + s + tsigs + s + std::to_string(tx.timestamp()) + s + tx.senderpubkey() + s;

            if(tx.has_account()){
                std::string assets = "";
                for(const Event::Asset& as: tx.account().assets()){
                    assets += as.name() + s4 + std::to_string(as.value()) + s3;
                }
                auto account = tx.account().publickey() + s2 + tx.account().name() + s2 + assets;
                return transaction + "ACCOUNT" + s + account;
            }else if(tx.has_asset()){
                auto asset = tx.asset().domain() + s2 + tx.asset().name() + s2 +\
                 std::to_string(tx.asset().value()) + s2 + std::to_string(tx.asset().precision());
                return transaction + "ASSET"    + s + asset;
            }else if(tx.has_domain()){
                auto domain = tx.domain().ownerpublickey() + s2 + tx.domain().name();
                return transaction + "DOMAIN"   + s + domain;
            }else {
                throw std::logic_error("Not implemented other than account/asset/domain");
            }
        }

        Event::Transaction s2t(std::string message){
            auto main = split(message, s);
            Event::Transaction tx;
            if(main.size() < 7){ return tx; }

            auto transfer_resv = split(main[0],s2);
            if(transfer_resv.size() == 2){
                tx.set_receivepubkey(transfer_resv[1]);
            }
            tx.set_type(transfer_resv[0]);
            tx.set_hash(main[1]);

            for(auto&& sig: split( main[2], s2)){
                auto pub_sig = split( sig, s3);
                if(pub_sig.size() != 2){
                    break;
                }
                Event::TxSignatures tsig;
                tsig.set_publickey(pub_sig[0]);
                tsig.set_signature(pub_sig[1]);
                tx.add_txsignatures()->CopyFrom(tsig);
            }
            tx.set_timestamp(std::atoi(main[3].c_str()));
            tx.set_senderpubkey(main[4]);
            if(main[5] == "ACCOUNT"){
                auto account = split(main[6], s2);
                tx.mutable_account()->set_publickey(account[0]);
                tx.mutable_account()->set_name(account[1]);
                for(auto&& as: split(account[2], s3)){
                    auto asset = split(as,s4);
                    if(asset.size() != 2){
                        break;
                    }
                    Event::Asset easset;
                    easset.set_name(asset[0]);
                    easset.set_value(std::atoi(asset[1].c_str()));
                    tx.mutable_account()->add_assets()->CopyFrom(easset);
                }
            }else if(main[5] == "ASSET"){
                auto asset = split(main[6], s2);
                if(asset.size() != 4){ return tx; }
                tx.mutable_asset()->set_domain(asset[0]);
                tx.mutable_asset()->set_name(asset[1]);
                tx.mutable_asset()->set_value(std::atoi(asset[2].c_str()));
                tx.mutable_asset()->set_precision(std::atoi(asset[3].c_str()));
            }else if(main[5] == "DOMAIN"){
                auto domain = split(main[6], s2);
                if(domain.size() != 2){ return tx; }
                tx.mutable_domain()->set_ownerpublickey(domain[0]);
                tx.mutable_domain()->set_name(domain[1]);
            }
            return tx;
        }

        // ====================================
        // ====================================



        void add(const std::string &key,const Event::ConsensusEvent& tx){
            logger::debug("sumeragi") << "addvalue:" << tx.transaction().asset().value();
            world_state_repository::add("transaction_" + key, t2s(tx.transaction()));
        }

        std::vector<Event::Transaction> findAll(){
            auto data = world_state_repository::findByPrefix("transaction_");
            std::vector<Event::Transaction> res;
            for(auto& s: data){
                auto tx = s2t(s);
                res.push_back(s2t(s));
                logger::info("tx repo") << "find All value:" << tx.asset().value();
            }
            return res;
        }

        Event::Transaction find(std::string key){
            std::string txKey = "transaction_" + key;
            return s2t(world_state_repository::find(txKey));
        }


        std::vector<Event::Transaction> findByAssetName(std::string name){
            std::string txKey = "transaction_" + name + "_";
            auto data = world_state_repository::findByPrefix(txKey);
            std::vector<Event::Transaction> res;
            for(auto& s: data){
                res.push_back(s2t(s));
            }
            return res;
        }

    };
};
