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

namespace asset {

    Asset::Asset(
            const std::string domain,
            const std::string name,
            const unsigned long long value,
            const unsigned int precision
    ):
        domain(domain),
        name(name),
        value(value),
        precision(precision)
    {}

    using Rule = json_parse::Rule;
    using Type = json_parse::Type;
    json_parse::Object Asset::dump() {
        json_parse::Object obj = json_parse::Object(Type::DICT);
//        obj.dictSub["name"] = json_parse::Object(Type::STR, name);
//        obj.dictSub["domain"] =  json_parse::Object(Type::STR, domain);
//        obj.dictSub["value"] =  json_parse::Object(Type::INT, (int)value);
//        obj.dictSub["precision"] =  json_parse::Object(Type::INT, (int)precision);
        return obj;
    }

    Rule Asset::getJsonParseRule() {
        Rule obj = Rule(Type::DICT);
//        obj.dictSub["name"] = Rule(Type::STR);
//        obj.dictSub["domain"] =  Rule(Type::STR);
//        obj.dictSub["value"] = Rule(Type::INT);
//        obj.dictSub["precision"] = Rule(Type::INT);
        return obj;
    }

};  // namespace asset
