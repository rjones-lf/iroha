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

#ifndef CORE_DOMAIN_OBJECTS_ASSET_HPP_
#define CORE_DOMAIN_OBJECTS_ASSET_HPP_

#include <string>
#include "object.hpp"
#include "../../service/json_parse.hpp"

namespace asset {

class Asset : public Object{

    std::string domain;
    std::string name;
    unsigned long long value;
    unsigned int precision;

public:

    Asset(
        const std::string domain,
        const std::string name,
        const unsigned long long value,
        const unsigned int precision
    );

    json_parse::Object dump();
    static json_parse::Rule getJsonParseRule();

};

};  // namespace asset

#endif  // CORE_DOMAIN_OBJECTS_ASSET_HPP_

