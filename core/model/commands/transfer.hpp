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

#ifndef CORE_DOMAIN_TRANSACTIONS_TRANSFERTRANSACTION_HPP_
#define CORE_DOMAIN_TRANSACTIONS_TRANSFERTRANSACTION_HPP_

#include "transaction.hpp"
#include "command.hpp"
#include "objects.hpp"

#include <string>
#include <type_traits>

#include "../../service/json_parse.hpp"

namespace command {

    template<typename T,
        std::enable_if_t<
            std::is_base_of<Objects, T>::value,std::nullptr_t
        > = nullptr
    >
    class Transfer : public Command{
        T object;
    public:
        Transfer(T object):
            object(object)
        {}

        std::string getAsJson() const{
            object.getAsJson();
        }

        std::string getCommandName() const{
            return "Transfer";
        }

        using Object = json_parse::Object;
        using Rule = json_parse::Rule;
        using Type = json_parse::Type;

        Object dump() {
            Object obj = Object(Object::Type::DICT);
            obj.dictSub["command"] = Object(Object::Type::STR, getCommandName());
            obj.dictSub["object"] = object.getJsonParseRule();
            return obj;
        }

        static Rule getJsonParseRule() {
            Rule obj = Rule(Type::DICT);
            obj.dictSub["command"] = Rule(Type::STR);
            obj.dictSub["object"] = T::getJsonParseRule();
            return obj;
        }
    };

};  // namespace command

#endif  // CORE_DOMAIN_TRANSACTIONS_TRANSFERTRANSACTION_HPP_

