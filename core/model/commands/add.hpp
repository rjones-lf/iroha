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

#ifndef CORE_DOMAIN_COMMANDS_ADD_HPP_
#define CORE_DOMAIN_COMMANDS_ADD_HPP_

#include "../../service/json_parse.hpp"

#include "../objects/domain.hpp"
#include "../objects/asset.hpp"
#include "../objects/message.hpp"

#include <string>

namespace command {

template <typename T>
class Add: public T {
  public:

    using Object = json_parse::Object;
    using Rule = json_parse::Rule;
    using Type = json_parse::Type;

    Add(
        const std::string& domain,
        const std::string& name,
        const unsigned long long& value,
        const unsigned int& precision
    );

    Add(const std::string& ownerPublicKey,const std::string& name);

    Add(Object obj);

    std::string getCommandName() const{
        return "Add";
    }

    Object dump() {
        Object obj = Object(Type::DICT);
        obj.dictSub.insert( std::make_pair( "name", Object(Type::STR, getCommandName())));
        obj.dictSub.insert( std::make_pair( "object", T::dump()));
        return obj;
    }

    static Rule getJsonParseRule() {
        auto rule = Rule(Type::DICT);
        rule.dictSub.insert(std::make_pair("name", Rule(Type::STR)));
        rule.dictSub.insert(std::make_pair("object", T::getJsonParseRule()));
        return rule;
    }
};
};  // namespace command

#endif  // CORE_DOMAIN_TRANSACTIONS_TRANSFERTRANSACTION_HPP_

