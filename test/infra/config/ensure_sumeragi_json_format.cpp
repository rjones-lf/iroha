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
#include <dirent.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <fstream>
#include "../../../core/infra/config/peer_service_with_json.hpp"

const std::string IrohaHome = getenv("IROHA_HOME");

TEST(ensure_sumeragi_json_format, normal_sumeragi_json) {
    std::ifstream ifs(IrohaHome + "/test/infra/config/inputs/normal_sumeragi.json");
    std::istreambuf_iterator<char> it(ifs);
    const auto jsonString = std::string(it, std::istreambuf_iterator<char>());
    ASSERT_TRUE(config::PeerServiceConfig::getInstance().ensureConfigFormat(jsonString));
}

TEST(ensure_sumeragi_json_format, bad_json) {
    std::ifstream ifs(IrohaHome + "/test/infra/config/inputs/bad_json.json");
    std::istreambuf_iterator<char> it(ifs);
    const auto jsonString = std::string(it, std::istreambuf_iterator<char>());
//    ASSERT_THROW(config::PeerServiceConfig::getInstance().ensureConfigFormat(jsonString), std::exception);
    ASSERT_EXIT(
        config::PeerServiceConfig::getInstance().ensureConfigFormat(jsonString),
        ::testing::ExitedWithCode(EXIT_FAILURE),
        ""//".*ERROR (-A-) [peer service with json] Bad json.*"
    );
}

TEST(ensure_sumeragi_json_format, bad_ip) {

    std::vector<std::string> fnames = {
      "bad_ip.json",
      "bad_ip2.json",
    };

    for (const auto& fname: fnames) {
        std::ifstream ifs(IrohaHome + "/test/infra/config/inputs/" + fname);
        std::istreambuf_iterator<char> it(ifs);
        const auto jsonString = std::string(it, std::istreambuf_iterator<char>());
        ASSERT_FALSE(config::PeerServiceConfig::getInstance().ensureConfigFormat(jsonString));
    }
}

TEST(ensure_sumeragi_json_format, missing_key) {
/*
    const auto directoryPath = IrohaHome + "/test/infra/config/inputs/";
    std::unique_ptr<DIR, decltype(&closedir)> dp(opendir(directoryPath.c_str()), closedir);
    if (not dp) {
        exit(EXIT_FAILURE);
    }

    std::vector<std::string> filepaths;
//    std::unique_ptr<dirent> entry(nullptr);
    struct dirent* entry;
    do {
        std::cout << "ok" << std::endl;
        entry = readdir(dp.get());
        if (entry) {
            std::cout << entry->d_name << std::endl;

            const auto fname = std::string(entry->d_name);
            if (fname.size() >= 3 && fname.substr(0, 3) == "no_") {
                filepaths.push_back(directoryPath + fname);
            }
        }
    } while (not entry);
*/

    const auto filepaths = {
        IrohaHome + "/test/infra/config/inputs/no_group_ip.json",
        IrohaHome + "/test/infra/config/inputs/no_group_name.json",
        IrohaHome + "/test/infra/config/inputs/no_group_publicKey.json",
        IrohaHome + "/test/infra/config/inputs/no_me_ip.json",
        IrohaHome + "/test/infra/config/inputs/no_me_name.json",
        IrohaHome + "/test/infra/config/inputs/no_me_privateKey.json",
        IrohaHome + "/test/infra/config/inputs/no_me_publicKey.json",
    };

    for (const auto& fpath: filepaths) {

        std::cout << "\n" << fpath << std::endl;

        std::ifstream ifs(fpath);
        std::istreambuf_iterator<char> it(ifs);
        const auto jsonString = std::string(it, std::istreambuf_iterator<char>());
        ASSERT_FALSE(config::PeerServiceConfig::getInstance().ensureConfigFormat(jsonString));
    }
}

TEST(ensure_sumeragi_json_format, useless_key) {

    const auto filepaths = {
        IrohaHome + "/test/infra/config/inputs/useless_me_key.json",
        IrohaHome + "/test/infra/config/inputs/useless_group_key.json",
    };

    for (const auto& fpath: filepaths) {

        std::cout << "\n" << fpath << std::endl;

        std::ifstream ifs(fpath);
        std::istreambuf_iterator<char> it(ifs);
        const auto jsonString = std::string(it, std::istreambuf_iterator<char>());
        ASSERT_FALSE(config::PeerServiceConfig::getInstance().ensureConfigFormat(jsonString));
    }
}
