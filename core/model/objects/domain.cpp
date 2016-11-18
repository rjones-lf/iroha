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

#include "domain.hpp"

namespace object {

using Rule = json_parse::Rule;
using Type = json_parse::Type;
using Object = json_parse::Object;

Domain::Domain(
    Object obj
){
    ownerPublicKey = obj.dictSub["ownerPublicKey"].str;
    name = obj.dictSub["name"].str;
}

Domain::Domain(
    const std::string& ownerPublicKey,
    const std::string& name
):
    ownerPublicKey(ownerPublicKey),
    name(name)
{}

json_parse::Object Domain::dump() {
    json_parse::Object obj = json_parse::Object(Type::DICT);
    obj.dictSub.insert( std::make_pair("ownerPublicKey",  json_parse::Object(Type::STR, ownerPublicKey)));
    obj.dictSub.insert( std::make_pair("name", json_parse::Object(Type::STR, name)));
    return obj;
}

Rule Domain::getJsonParseRule() {
    auto rule = Rule(Type::DICT);
    rule.dictSub.insert( std::make_pair("ownerPublicKey",  Rule(Type::STR)));
    rule.dictSub.insert( std::make_pair("name", Rule(Type::STR)));
    return rule;
}

};  // namespace domain
