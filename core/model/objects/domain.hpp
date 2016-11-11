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

#ifndef CORE_DOMAIN_OBJECTS_DOMAIN_HPP_
#define CORE_DOMAIN_OBJECTS_DOMAIN_HPP_

#include <string>
#include "object.hpp"
#include "../../service/json_parse.hpp"

namespace domain {

class Domain : public AbsObject{
    std::string ownerPublicKey;
    std::string name;

public:
    Domain(
        std::string ownerPublicKey,
        std::string name
    );

    virtual json_parse::Object dump() override;
    virtual json_parse::Rule getJsonParseRule() override;
};

};  // namespace domain

#endif  // CORE_DOMAIN_OBJECTS_DOMAIN_HPP_

