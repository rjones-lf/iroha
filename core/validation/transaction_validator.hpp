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

#ifndef CORE_VALIDATION_TRANSACTIONVALIDATOR_HPP_
#define CORE_VALIDATION_TRANSACTIONVALIDATOR_HPP_

#include "../model/transaction.hpp"
#include <memory>
#include <type_traits>

namespace transaction_validator {

    template<
        typename T,
            std::enable_if_t<std::is_base_of<transaction::Transaction<command::Command>, T>::value, std::nullptr_t> = nullptr>
    bool isValid(const T& tx){
        return true;
    }

    template<typename T, std::enable_if_t<std::is_base_of<transaction::Transaction<command::Command>, T>::value, std::nullptr_t> = nullptr>
    bool signaturesAreValid(const T& tx){
        return true;
    }

    template<typename T, std::enable_if_t<std::is_base_of<transaction::Transaction<command::Command>, T>::value, std::nullptr_t> = nullptr>
    bool validForType(const T& tx){
        return true;
    }

};  // namespace transaction_validator

#endif  // CORE_VALIDATION_TRANSACTIONVALIDATOR_HPP_
