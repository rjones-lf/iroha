/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_TEST_TRANSACTION_BUILDER_HPP
#define IROHA_TEST_TRANSACTION_BUILDER_HPP

#include "builders/protobuf/builder_templates/transaction_template.hpp"
#include "module/shared_model/validators/validators.hpp"

/**
 * Builder alias, to build shared model proto transaction object avoiding
 * validation and "required fields" check
 */
using TestTransactionBuilder = shared_model::proto::TemplateTransactionBuilder<
    (1 << shared_model::proto::TemplateTransactionBuilder<>::total) - 1,
    shared_model::validation::TransactionAlwaysValidValidator,
    shared_model::proto::Transaction>;

using TestUnsignedTransactionBuilder =
    shared_model::proto::TemplateTransactionBuilder<
        (1 << shared_model::proto::TemplateTransactionBuilder<>::total) - 1,
        shared_model::validation::TransactionAlwaysValidValidator,
        shared_model::proto::UnsignedWrapper<shared_model::proto::Transaction>>;

#endif  // IROHA_TEST_TRANSACTION_BUILDER_HPP
