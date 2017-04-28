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

#include <json.hpp>

#include <fstream>
#include <infra/config/iroha_config_with_json.hpp>
#include <consensus/connection/connection.hpp>
#include <thread>
#include <utils/ip_tools.hpp>

using Api::DiscoverRequest;
using Api::Peer;

int main(int argc, char* argv[]) {

    std::string discoveredFileName = "discovered.txt";
    if (argc == 2) discoveredFileName.assign(argv[1]);
    if (argc > 2) {
        std::cout << "Usage: make_hostdiscover [filename]" << std::endl;
        std::cout << "Discovered nodes will be saved into filename "
                "in <ip> <public key> format" << std::endl;
        return 1;
    }

    std::vector<std::string> defaultHosts {};
    auto trustedHosts = config::IrohaConfigManager::getInstance().getTrustedHosts(defaultHosts);
    if (trustedHosts.empty()) {
        std::cout << "Config section for trusted hosts was not found or empty." << std::endl;
        return 1;
    }

    std::ofstream hostsDiscovered(discoveredFileName);

    DiscoverRequest request;
    request.set_message("discovery");
    for (auto host: trustedHosts) {
        if (ip_tools::isIpValid(host)) {
            Peer peer = connection::iroha::HostDiscovery::getHostInfo::send(host, request);
            if (peer.publickey() != "") {
                hostsDiscovered << peer.address() << " " << peer.publickey() << std::endl;

            }
        } else {
            // maybe we have a netmask?
            auto range = ip_tools::getIpRangeByNetmask(host);
            for (uint32_t i = 0; i < range.second; ++i) {
                Peer peer = connection::iroha::HostDiscovery::getHostInfo::send(
                        ip_tools::uintIpToString(range.first++), request);
                if (peer.publickey() != "") {
                    hostsDiscovered << peer.address() << " " << peer.publickey() << std::endl;
                }
            }
        }
    }

    return 0;
}
