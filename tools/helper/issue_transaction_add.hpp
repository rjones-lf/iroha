/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

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

//
// Created by Takumi Yamashita on 2017/03/10.
//

#ifndef TOOLS_ISSUE_TRANSACTION_ADD_HPP
#define TOOLS_ISSUE_TRANSACTION_ADD_HPP

#include <iostream>
#include <vector>
#include <stdexcept>
#include <util/logger.hpp>
#include <transaction_builder/transaction_builder.hpp>
#include <consensus/connection/connection.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <service/peer_service.hpp>



namespace tools {
    namespace issue_transaction {

        namespace add {
            namespace account {
                void issue_transaction(std::vector <std::string> &argv) {
                    try {
                        auto
                        tx = txbuilder::TransactionBuilder < type_signatures::Add < type_signatures::Account >> ()
                                .setSenderPublicKey(::peer::myself::getPublicKey())
//TODO                                txbuilder::createAccount( argv[0], argv[1], txbuilder::createTrust( stod(argv[2]), argv[3]=="true" ) )
                                .build();
                        connection::iroha::PeerService::Sumeragi::send(
                                ::peer::myself::getIp(),
                                tx
                        );
                    } catch (const std::out_of_range &oor) {
                        logger::error("issue_transaction") << "Not enough elements.";
                        logger::error("issue_transaction") << oor.what();
                        exit(1);
                    } catch (...) {
                        logger::error("issue_transaction") << "etc exception";
                        exit(1);
                    }
                }
            }
            namespace asset {
                void issue_transaction(std::vector <std::string> &argv) {
                    try {
                        auto
                        tx = txbuilder::TransactionBuilder < type_signatures::Add < type_signatures::Asset >> ()
                                .setSenderPublicKey(::peer::myself::getPublicKey())
//TODO                                txbuilder::createAsset( argv[0], argv[1], txbuilder::createTrust( stod(argv[2]), argv[3]=="true" ) )
                                .build();
                        connection::iroha::PeerService::Sumeragi::send(
                                ::peer::myself::getIp(),
                                tx
                        );
                    } catch (const std::out_of_range &oor) {
                        logger::error("issue_transaction") << "Not enough elements.";
                        logger::error("issue_transaction") << oor.what();
                        exit(1);
                    } catch (...) {
                        logger::error("issue_transaction") << "etc exception";
                        exit(1);
                    }
                }
            }
            namespace peer {
                void issue_transaction(std::vector <std::string> &argv) {
                    try {
                        auto
                        tx = txbuilder::TransactionBuilder < type_signatures::Add < type_signatures::Peer >> ()
                                .setSenderPublicKey(::peer::myself::getPublicKey())
                            .setPeer(
                                txbuilder::createPeer( argv.at(0), argv.at(1), txbuilder::createTrust( stod(argv.at(2)), argv.at(3)=="true" ) )
                            )
                                .build();
                        connection::iroha::PeerService::Sumeragi::send(
                                ::peer::myself::getIp(),
                                tx
                        );
                    } catch (const std::out_of_range &oor) {
                        logger::error("issue_transaction") << "Not enough elements.";
                        logger::error("issue_transaction") << oor.what();
                        exit(1);
                    } catch (...) {
                        logger::error("issue_transaction") << "etc exception";
                        exit(1);
                    }
                }
            }
            namespace domain {
                void issue_transaction(std::vector <std::string> &argv) {
                    try {
                        auto
                        tx = txbuilder::TransactionBuilder < type_signatures::Add < type_signatures::Domain >> ()
                                .setSenderPublicKey(::peer::myself::getPublicKey())
                            .setDomain(
                                txbuilder::createDomain( argv.at(0), argv.at(1) )
                            )
                                .build();
                        connection::iroha::PeerService::Sumeragi::send(
                                ::peer::myself::getIp(),
                                tx
                        );
                    } catch (const std::out_of_range &oor) {
                        logger::error("issue_transaction") << "Not enough elements.";
                        logger::error("issue_transaction") << oor.what();
                        exit(1);
                    } catch (...) {
                        logger::error("issue_transaction") << "etc exception";
                        exit(1);
                    }
                }
            }
        }
    }
}

#endif // TOOLS_ISSUE_TRANSACTION_ADD_HPP