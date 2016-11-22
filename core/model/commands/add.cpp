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
#include "add.hpp"

namespace command {

using Object = json_parse::Object;
using Rule = json_parse::Rule;
using Type = json_parse::Type;

template <>
Add<object::Domain>::Add(
    Object obj
):Domain(obj){}

template <>
Add<object::Asset>::Add(
    Object obj
):Asset(obj){}

template <>
Add<object::Domain>::Add(
        const std::string& ownerPublicKey,
        const std::string& name
):
    object::Domain(
        ownerPublicKey,
        name
    )
{}

template <>
Add<object::Asset>::Add(
    const std::string& domain,
    const std::string& name,
    const unsigned long long& value,
    const unsigned int& precision
):
    object::Asset(
        domain,
        name,
        value,
        precision
    )
{}

}