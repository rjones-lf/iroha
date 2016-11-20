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
#include "asset.hpp"
#include <iostream>

namespace object {

using Rule = json_parse::Rule;
using Type = json_parse::Type;
using Object = json_parse::Object;

Asset::Asset(
    Object obj
):
    domain(obj.dictSub["domain"].str),
    name(obj.dictSub["name"].str),
    value(obj.dictSub["value"].integer),
    precision(obj.dictSub["precision"].integer)
{}

Asset::Asset(
    const std::string& domain,
    const std::string& name,
    const unsigned long long& value,
    const unsigned int& precision
):
    domain(domain),
    name(name),
    value(value),
    precision(precision)
{}

Asset::Asset(
    const std::string& name,
    const unsigned long long& value
):
    name(name),
    value(value)
{}

json_parse::Object Asset::dump() {
    json_parse::Object obj = Object(Type::DICT);
    obj.dictSub.insert( std::make_pair( "name", Object(Type::STR, name)));
    obj.dictSub.insert( std::make_pair( "domain", Object(Type::STR, domain)));
    obj.dictSub.insert( std::make_pair( "value",  Object(Type::INT, (int)value)));
    obj.dictSub.insert( std::make_pair( "precision", Object(Type::INT, (int)precision)));
    return obj;
}

Rule Asset::getJsonParseRule() {
    auto rule = Rule(Type::DICT);
    rule.dictSub.insert( std::make_pair( "name", Rule(Type::STR)));
    rule.dictSub.insert( std::make_pair( "domain", Rule(Type::STR)));
    rule.dictSub.insert( std::make_pair( "value", Rule(Type::INT)));
    rule.dictSub.insert( std::make_pair( "precision", Rule(Type::INT)));
    return rule;
}

};  // namespace asset
