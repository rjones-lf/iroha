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

#include <iostream>
#include <vector>
#include <stdexcept>
#include <util/logger.hpp>
#include <transaction_builder/transaction_builder.hpp>
#include <consensus/connection/connection.hpp>
#include <infra/config/peer_service_with_json.hpp>

#ifndef TOOLS_ISSUE_TRANSACTION_CONTRACT_HPP
#define TOOLS_ISSUE_TRANSACTION_CONTRACT_HPP

namespace tools {
    namespace issue_transaction {
        namespace contract {
            namespace account {
                void issue_transaction( std::vector<std::string>& argv ) {
                    auto tx = txbuilder::TransactionBuilder<Contract<Account>>()
                            .setSenderPublicKey(config::PeerServiceConfig::getMyPublicKey())
//TODO                                txbuilder::createAccount( argv[0], argv[1], txbuilder::createTrust( stod(argv[2]), argv[3]=="true" ) )
                            .build();
                    connection::iroha::PeerService::Sumeragi::send(
                            config::PeerServiceConfig::getInstance().getMyIp(),
                            tx
                    )
                } catch (const out_of_range& oor) {
                    logger::error("issue_transaction") << "Not enough elements." << endl;
                    logger::error("issue_transaction") << oor.what() << endl;
                    exit(1);
                } catch( ... ) {
                    logger::error("issue_transaction") << "etc error" << endl;
                    exit(1);
                }
            }
            namespace asset {
                void issue_transaction( std::vector<std::string>& argv ) {
                    auto tx = txbuilder::TransactionBuilder<Contract<Asset>>()
                            .setSenderPublicKey(config::PeerServiceConfig::getMyPublicKey())
//TODO                                txbuilder::createAsset( argv[0], argv[1], txbuilder::createTrust( stod(argv[2]), argv[3]=="true" ) )
                            .build();
                    connection::iroha::PeerService::Sumeragi::send(
                            config::PeerServiceConfig::getInstance().getMyIp(),
                            tx
                    )
                } catch (const out_of_range& oor) {
                    logger::error("issue_transaction") << "Not enough elements." << endl;
                    logger::error("issue_transaction") << oor.what() << endl;
                    exit(1);
                } catch( ... ) {
                    logger::error("issue_transaction") << "etc error" << endl;
                    exit(1);
                }
            }
            namespace peer {
                void issue_transaction( std::vector<std::string>& argv ) {
                    auto tx = txbuilder::TransactionBuilder<Contract<Peer>>()
                            .setSenderPublicKey(config::PeerServiceConfig::getMyPublicKey())
                            .setPeer(
                                txbuilder::createPeer( argv[0], argv[1], txbuilder::createTrust( stod(argv[2]), argv[3]=="true" ) )
                            )
                            .build();
                    connection::iroha::PeerService::Sumeragi::send(
                            config::PeerServiceConfig::getInstance().getMyIp(),
                            tx
                    )
                } catch (const out_of_range& oor) {
                    logger::error("issue_transaction") << "Not enough elements." << endl;
                    logger::error("issue_transaction") << oor.what() << endl;
                    exit(1);
                } catch( ... ) {
                    logger::error("issue_transaction") << "etc error" << endl;
                    exit(1);
                }
            }
            }
        }
    }
}

#endif // TOOLS_ISSUE_TRANSACTION_CONTRACT_HPP