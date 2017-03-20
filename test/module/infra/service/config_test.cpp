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
#include <gtest/gtest.h>

#include <string>
#include <fstream>
#include <regex>

#include <json.hpp>

using json = nlohmann::json;

void isIpValid(const std::string &ip) {
    std::regex ipRegex("\"((([0-1]?\\d\\d?)|((2[0-4]\\d)|(25[0-5]))).){3}(([0-1]?\\d\\d?)|((2[0-4]\\d)|(25[0-5])))\"");
    ASSERT_TRUE(std::regex_match(ip, ipRegex)) << "IP " << ip << " looks like not a valid ip.";
}

TEST(config, isSystemConfigValid) {

    const auto irohaHome = []() -> std::string {
        auto p = getenv("IROHA_HOME");
        return p == nullptr ? "" : std::string(p);
    }();

    ASSERT_FALSE(irohaHome.empty()) << "Warning: IROHA_HOME is not set.";
    
    std::string configFileName = irohaHome + "/config/sumeragi.json";
    std::ifstream ifs(configFileName);
    ASSERT_FALSE(ifs.fail()) << "Can't open " << configFileName << " file.";

    std::istreambuf_iterator<char> it(ifs);
    std::istreambuf_iterator<char> last;
    std::string res(it, last);
    ASSERT_FALSE(res == "") << "Config " << configFileName << " is empty or can't be read.";

    json configData = json::parse(res);
    std::string myip = configData["me"]["ip"].dump();
    isIpValid(myip);

    ASSERT_FALSE(configData["me"]["name"].dump() == "\"\"") << "Your name is empty.";
    ASSERT_FALSE(configData["me"]["publicKey"].dump() == "\"\"") << "Your public key is empty.";
    ASSERT_FALSE(configData["me"]["privateKey"].dump() == "\"\"") << "Your private key is empty.";


    std::set<std::string> ips;
    for (auto peer: configData["group"]) {
        auto prevSize = ips.size();
        ips.insert(peer["ip"].dump());
        ASSERT_TRUE(ips.size() == prevSize+1) << "IP duplicate found: " << peer["ip"];

        if (peer["ip"].dump() == myip) {
            ASSERT_TRUE(peer["name"] == configData["me"]["name"])
                    << "My name differs from a name of the node with same IP as me.";
            ASSERT_TRUE(peer["publicKey"] == configData["me"]["publicKey"])
                    << "My publicKey differs from a name of the node with same IP as me.";
        }

        isIpValid(peer["ip"].dump());
        ASSERT_FALSE(peer["name"].dump() == "\"\"")
                << "Name of node " << peer["ip"].dump() << " is empty";
        ASSERT_FALSE(peer["publicKey"].dump() == "\"\"")
                << "Public key of node " << peer["ip"].dump() << " is empty";
        ASSERT_TRUE(peer["privateKey"].dump() == "null")
                << "Private key of node " << peer["ip"].dump() << " is not empty";
    }
}
