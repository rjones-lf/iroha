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
#include <service/peer_service.hpp>
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
            auto account = repository::account::find(tx.senderpubkey());
            if(!account.name().empty()){
                repository::asset::add(tx.senderpubkey(),asset.name(),asset);
                account.add_assets(asset.name());
                repository::account::update(tx.senderpubkey(), account);
            }
        } else if (tx.has_domain()) {
            // Add<Domain>
            // Domain will be supported by v1.0
        } else if (tx.has_account()) {
            // Add<Account>
            const auto account = tx.account();
            repository::account::add(account.publickey(), account);

            for(auto asset_name: account.assets()){
                // Add default asset
                auto asset = Api::Asset();
                auto base  = Api::BaseObject();
                base.set_valueint(0);
                asset.set_name(asset_name);
                asset.set_domain("default");
                (*asset.mutable_value())["value"] = base;
                logger::info("executor") << "add asset: " << asset.DebugString();
                repository::asset::add(tx.senderpubkey(),asset.name(),asset);
            }
            logger::info("executor") << "add account";

        } else if( tx.has_peer() ) {
            logger::info("executor") << "add peer";
            logger::info("executor") << "add peer";
            // Temporary - to operate peer service
            peer::Node query_peer(tx.peer().address(), tx.peer().publickey(),
                                  tx.peer().trust().value(), tx.peer().trust().isok());
            ::peer::transaction::executor::add(query_peer);
            if (::peer::myself::getIp() != query_peer.ip && ::peer::myself::isActive())
                ::peer::transaction::izanami::start(query_peer);
        }
    }

    // **********************************************************************************
    // * This is Transfer<Asset>'s logic. Tax send logic
    // **********************************************************************************
    namespace tax{

        void transfer(const Transaction& tx){
            const auto sender = tx.senderpubkey();
            const auto receiver = tx.receivepubkey();
            const auto assetName = tx.asset().name();

            const auto author =  tx.asset().value().find("author");
            const auto percent = tx.asset().value().find("percent");
            const auto value = tx.asset().value().find("value");

            if (author != tx.asset().value().end() &&
                percent != tx.asset().value().end() &&
                value != tx.asset().value().end()
            ) {
                auto senderAsset = repository::asset::find(sender, assetName);
                auto authorAsset = repository::asset::find((*author).second.valuestring(), assetName);
                auto receiverAsset = repository::asset::find(receiver, assetName);

                if(!senderAsset.name().empty() &&
                   !authorAsset.name().empty() &&
                   !receiverAsset.name().empty()
                ) {
                    auto senderValue   = senderAsset.value().find("value");
                    auto receiverValue = receiverAsset.value().find("value");
                    auto authorValue   = authorAsset.value().find("value");
                    if (
                            senderValue != senderAsset.value().end() &&
                            receiverValue != receiverAsset.value().end() &&
                            authorValue != authorAsset.value().end()
                        ) {
                        if ((*senderValue).second.valueint() >= (*value).second.valueint() ) {
                            (*senderAsset.mutable_value())["value"].set_valueint(
                                (google::protobuf::int64) (
                                    (*senderValue).second.valueint() - (*value).second.valueint()
                                )
                            );
                            (*receiverAsset.mutable_value())["value"].set_valueint(
                                (google::protobuf::int64) (
                                    (*receiverValue).second.valueint() + ((*value).second.valueint() * (1 - (*percent).second.valuedouble()))
                                )
                            );
                            (*authorAsset.mutable_value())["value"].set_valueint(
                                (google::protobuf::int64)(
                                    (*authorValue).second.valueint() + (*value).second.valueint() * (*percent).second.valuedouble()
                                )
                            );
                            repository::asset::update(sender, assetName, senderAsset);
                            repository::asset::update(receiver, assetName, receiverAsset);

                            repository::asset::update((*author).second.valuestring(), assetName, authorAsset);
                        }else{
                            logger::error("executor") << "Ops! sender does not have enough value";
                        }
                    }else{
                        logger::error("executor") << "sender or receiver's asset does not contain value";
                    }
                }else{
                    logger::error("executor") << "sender or receiver or author does not have asset " << assetName;
                }
            }else{
                logger::error("executor") << "Asset doesnot have author or percent or value";
            }
        }
    };

    // **********************************************************************************
    // * This is Transfer<Asset>'s logic. multi message chat
    // **********************************************************************************
    namespace multi_message{

        void transfer(const Transaction& tx){
            const auto sender = tx.senderpubkey();
            const auto receiver = tx.receivepubkey();
            const auto assetName = tx.asset().name();

            const auto targetName =  tx.asset().value().find("targetName");
            const auto value = tx.asset().value().find("value");

            if (targetName != tx.asset().value().end() &&
                value != tx.asset().value().end()
            ) {
                auto senderAsset    = repository::asset::find(sender, assetName);
                auto receiverAsset  = repository::asset::find(receiver, assetName);
                if(
                   !senderAsset.name().empty() &&
                   !receiverAsset.name().empty()
                ) {
                    auto senderHasNum    = senderAsset.value().find((*targetName).second.valuestring());
                    auto receiverHasNum  = receiverAsset.value().find((*targetName).second.valuestring());
                    if(
                        senderHasNum != senderAsset.value().end() &&
                        receiverHasNum != receiverAsset.value().end()
                    ){
                        (*senderAsset.mutable_value())[(*targetName).second.valuestring()].set_valueint(
                                (*senderHasNum).second.valueint() - (*value).second.valueint()
                        );
                        (*receiverAsset.mutable_value())[(*targetName).second.valuestring()].set_valueint(
                                (*receiverHasNum).second.valueint() + (*value).second.valueint()
                        );

                        repository::asset::update(sender, assetName, senderAsset);
                        repository::asset::update(receiver, assetName, receiverAsset);
                    }else{
                        logger::error("executor") << "sender or receiver does not have target "<< (*targetName).second.valuestring();
                    }
                }else{
                    logger::error("executor") << "sender or receiver does not have asset "<< assetName;
                }
            }else{
                logger::error("executor") << "Tx does not contain targetName ";
            }
        }
    };

    // **********************************************************************************
    // * This is Transfer<Asset>'s logic. virtual currency
    // **********************************************************************************
    namespace currency{
        void transfer(const Transaction& tx){
            const auto sender = tx.senderpubkey();
            const auto receiver = tx.receivepubkey();
            const auto assetName = tx.asset().name();

            const auto value = tx.asset().value().find("value");
            if (
                value != tx.asset().value().end()
            ) {
                auto senderAsset = repository::asset::find(sender, assetName);
                auto receiverAsset = repository::asset::find(receiver, assetName);
                if (
                    !senderAsset.name().empty() &&
                    !receiverAsset.name().empty()
                ){
                    auto senderValue = senderAsset.value().find("value");
                    auto receiverValue = receiverAsset.value().find("value");

                    if(
                        senderValue != senderAsset.value().end() &&
                        receiverValue != receiverAsset.value().end()
                    ){
                        if((*senderValue).second.valueint() >= (*value).second.valueint()) {
                            (*senderAsset.mutable_value())["value"].set_valueint(
                                (*senderValue).second.valueint() - (*value).second.valueint()
                            );
                            (*receiverAsset.mutable_value())["value"].set_valueint(
                                (*receiverValue).second.valueint() + (*value).second.valueint()
                            );
                            repository::asset::update(sender, assetName, senderAsset);
                            repository::asset::update(receiver, assetName, receiverAsset);
                        }
                    }
                }
            }
        }

    };

    void transfer(const Transaction& tx){
        if(tx.has_asset()){
            const auto type = tx.asset().value().find("type");
            if (type != tx.asset().value().end() && (*type).second.valuestring() == "tax") {
                logger::info("executor") << "tx type is tax ";
                tax::transfer(tx);
            }else if (type != tx.asset().value().end() && (*type).second.valuestring() == "multi_message") {
                logger::info("executor") << "tx type is multi message";
                multi_message::transfer(tx);
            }else{
                logger::info("executor") << "tx type is currency (default)";
                currency::transfer(tx);
            }
        }else if(tx.has_domain()){
            // Domain will be supported by v1.0
            // Transfer<Domain>
        }
    }

    void update(const Transaction& tx){
        if(tx.has_asset()){
            logger::info("executor") << "Update";
            const auto asset = tx.asset();
            const auto publicKey = tx.senderpubkey();
            const auto assetName = asset.name();
            auto senderAsset = repository::asset::find(publicKey, assetName);
            if (
                !senderAsset.name().empty()
            ){
                repository::asset::update(publicKey, assetName, senderAsset);
            }
        }else if(tx.has_domain()){
            // Domain will be supported by v1.0
        }else if(tx.has_account()){
            // Update<Account> will be supported by v1.0
        }else if(tx.has_peer()){
            // Temporary - to operate peer service
            // Update<Peer>
            // Temporary - to operate peer service
            peer::Node query_peer(tx.peer().address(), tx.peer().publickey(),
                                  tx.peer().trust().value(), tx.peer().trust().isok());
            ::peer::transaction::executor::update(query_peer.publicKey, query_peer);
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
            // Domain will be supported by v1.0
        }else if(tx.has_account()){
            // Remove<Account>
            const auto account = tx.account();
            repository::account::remove(account.publickey());
        }else if(tx.has_peer()){
            // Temporary - to operate peer service
            peer::Node query_peer(tx.peer().address(), tx.peer().publickey(),
                                  tx.peer().trust().value(), tx.peer().trust().isok());
            ::peer::transaction::executor::remove(query_peer.publicKey);
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
        }else{
            logger::info("executor") << "Uknowen command:" << tx.type();
        }
    }
};
