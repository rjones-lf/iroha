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
#include "../model/commands/add.hpp"
#include "../model/commands/transfer.hpp"

#include "../model/objects/domain.hpp"
#include "../model/objects/asset.hpp"
#include "../model/objects/message.hpp"

#include <memory>
#include <type_traits>

namespace transaction_validator {

    using transaction::Transaction;

    template<typename T>
    bool isValid(const std::unique_ptr<T>& tx);

    template<typename T>
    bool signaturesAreValid(const std::unique_ptr<T>& tx);

    template<typename T>
    bool validForType(const std::unique_ptr<T>& tx);

};  // namespace transaction_validator

#endif  // CORE_VALIDATION_TRANSACTIONVALIDATOR_HPP_
