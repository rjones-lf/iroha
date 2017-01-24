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

#ifndef __CORE_UTIL_SERIALIZED_TYPE_SWITCHER_HPP__
#define __CORE_UTIL_SERIALIZED_TYPE_SWITCHER_HPP__

#include "logger.hpp"
#include <string>

namespace util { namespace type_switcher {
    enum class SerializedTypes {
        NoType,
        Int32,
        Int64,
        Uint32,
        Uint64,
        String,
        Double,
    };

    constexpr int TypeIDSize = 6;   // Length

    inline SerializedTypes getType(std::string const& s) {
        if (s == "Int32_")  return SerializedTypes::Int32;
        if (s == "Int64_")  return SerializedTypes::Int64;
        if (s == "Uint32")  return SerializedTypes::Uint32;
        if (s == "Uint64")  return SerializedTypes::Uint64;
        if (s == "String")  return SerializedTypes::String;
        if (s == "Double")  return SerializedTypes::Double;

        // TODO: IROHA_ASSERT
        logger::fatal("type_switcher") << "type mismatch.";
        std::runtime_error("error");
        return SerializedTypes::NoType;
    }
}}

#endif  // __CORE_UTIL_SERIALIZED_TYPE_SWITCHER_HPP__
