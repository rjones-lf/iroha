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
#include <vector>
#include <json.hpp>
#include <service/http_client.hpp>

using nlohmann::json;

std::vector<std::string> split(const std::string& str, char splitter = '\n', unsigned int time = 0) {
    std::vector<std::string> res;
    std::stringstream ss(str);
    std::string tmp;
    unsigned int count = 0;
    while(getline(ss, tmp, splitter)) {
        if(!tmp.empty()){
            res.push_back(tmp);
        }
        if(time != 0 && count >= time){
            res.push_back(ss.str());
            break;
        }
        count++;
    }
    return res;
}

TEST(config, isSystemConfigValid) {

    auto req = http_client::Request("GET","/", "");
    req.addHeader("Accept", "*/*");

    auto res = http_client::request( "example.com", 443, req);
    ASSERT_EQ( std::get<0>(res), 0);
    ASSERT_STREQ( split(std::get<1>(res)).at(0).c_str(), "HTTP/1.1 200 OK\r");

}
