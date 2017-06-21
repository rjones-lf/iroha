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

#include <logger/logger.hpp>
#include <gtest/gtest.h>
#include <iostream>

#include <vector>
#include <string>

std::vector<std::string> split(std::string str, const std::string& delim) {
    std::vector<std::string> res;
    int delim_size = delim.size();
    while(str.find(delim.c_str()) != std::string::npos){
        res.push_back(str.substr(0,str.find(delim.c_str())));
        str = str.substr(str.find(delim.c_str()) + delim_size);
    }
    res.push_back(str);
    return res;
}

TEST( Logger, basic) {
    auto log = logger::Logger("sample");
    testing::internal::CaptureStdout();
    log.info("Is order the rabbit?? {:08d}", 12345);
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_STREQ(split(output," [sample] ")[1].c_str(), "[info] Is order the rabbit?? 00012345\n\x1B[00m");
}
