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

#include <infra/protobuf/api.pb.h>
#include <iostream>
#include <infra/config/peer_service_with_json.hpp>

#include <repository/domain/asset_repository.hpp>
#include <repository/domain/account_repository.hpp>
#include <util/logger.hpp>

namespace executor{

    using Api::Transaction;
    void add(const Transaction &tx) {
        logger::info("executor") << "tx has peer?" << (tx.has_peer()?"yes":"no");
        if (tx.has_asset()) {
            // Add<Asset>
            const auto asset = tx.asset();
            repository::asset::add(tx.senderpubkey(),asset.name(),asset);
        } else if (tx.has_domain()) {
            // Add<Domain>
            const auto domain = tx.domain();
        } else if (tx.has_account()) {
            // Add<Account>
            const auto account = tx.account();
            repository::account::add(account.publickey(), account);
            logger::info("executor") << "add account";
        } else if( tx.has_peer() ) {
            logger::info("executor") << "add peer";
            // Temporary - to operate peer service
            peer::Node query_peer(
                    tx.peer().address(),
                    tx.peer().publickey(),
                    tx.peer().trust().value(),
                    tx.peer().trust().isok()
            );
            config::PeerServiceConfig::getInstance().addPeer( query_peer );
            if( config::PeerServiceConfig::getInstance().getMyIp() != query_peer.getIP() &&
                    config::PeerServiceConfig::getInstance().isMyActive() )
                config::PeerServiceConfig::getInstance().sendAllTransactionToNewPeer( query_peer );
        }
    }

    void transfer(const Transaction& tx){
        if(tx.has_asset()){
            // Transfer<Asset>
            auto sender = tx.senderpubkey();
            auto receiver = tx.receivepubkey();
            const auto assetName = tx.asset().name();
            auto senderAsset    = repository::asset::find(  sender,assetName);
            auto receiverAsset  = repository::asset::find(receiver,assetName);
            // **********************************************************************************
            // * This is Transfer<Asset>'s logic.
            // **********************************************************************************


            // **********************************************************************************
            repository::asset::update(sender,assetName,senderAsset);
            repository::asset::update(receiver,assetName,receiverAsset);
        }else if(tx.has_domain()){
            // Transfer<Domain>
        }else if(tx.has_account()){
            // Transfer<Account>
            // Add<Account>
        }else if(tx.has_peer()){
            // Transfer<Peer>
            // nothing this transaction
        }
    }

    void update(const Transaction& tx){
        if(tx.has_asset()){
            // Update<Asset>
        }else if(tx.has_domain()){
            // Update<Domain>
        }else if(tx.has_account()){
            // Update<Account>
            const auto account = tx.account();
            repository::account::update(account.publickey(), account);
        }else if(tx.has_peer()){
            // Temporary - to operate peer service
            peer::Node query_peer(
                    tx.peer().address(),
                    tx.peer().publickey(),
                    tx.peer().trust().value(),
                    tx.peer().trust().isok()
            );
            config::PeerServiceConfig::getInstance().updatePeer( query_peer.getPublicKey(), query_peer );
            // Update<Peer>
        }
    }

    void remove(const Transaction& tx){
        if(tx.has_asset()){
            // Remove<Asset>
            const auto name = tx.account().name();
            repository::asset::remove(tx.senderpubkey(),name);
        }else if(tx.has_domain()){
            // Remove<Domain>
        }else if(tx.has_account()){
            // Remove<Account>
            const auto account = tx.account();
            repository::account::remove(account.publickey());
        }else if(tx.has_peer()){
            // Temporary - to operate peer service
            peer::Node query_peer(
                    tx.peer().address(),
                    tx.peer().publickey(),
                    tx.peer().trust().value(),
                    tx.peer().trust().isok()
            );
            config::PeerServiceConfig::getInstance().removePeer( query_peer.getPublicKey() );
        }
    }

    void contract(const Transaction& tx){
        if(tx.has_asset()){
            // Contract<Asset>
        }else if(tx.has_domain()){
            // Contract<Domain>
        }else if(tx.has_account()){
            // Contract<Account>
        }else if(tx.has_peer()){
            // Contract<Peer>
            // nothing this transaction
        }
    }

    void execute(const Transaction& tx){
        logger::info("executor") << "Executor";
        logger::info("executor")  << "DebugString:"<< tx.DebugString();
        logger::info("executor") << "tx type(): " << tx.type();
        std::string type = tx.type();
        std::transform(cbegin(type), cend(type), begin(type), ::tolower);

        if(type == "add"){
            add(tx);
        }else if(type == "transfer"){
            transfer(tx);
        }else if(type == "update"){
            update(tx);
        }else if(type == "remove"){
            remove(tx);
        }else if(type == "contract"){
            contract(tx);
        }
    }

};
