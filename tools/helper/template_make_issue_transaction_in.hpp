            namespace DATAMODEL_NAME_A {
                void issue_transaction(std::vector <std::string> &argv) {
                    try {
                        auto
                        tx = txbuilder::TransactionBuilder < type_signatures::COMMAND_NAME_B < type_signatures::DATAMODEL_NAME_B >> ()
                                .setSenderPublicKey(config::PeerServiceConfig::getInstance().getMyPublicKey())
                        ##CODE
                                .build();
                        connection::iroha::PeerService::Sumeragi::send(
                                config::PeerServiceConfig::getInstance().getMyIp(),
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
