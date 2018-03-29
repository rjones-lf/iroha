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

#ifndef IROHA_DEFAULT_BUILDERS_HPP
#define IROHA_DEFAULT_BUILDERS_HPP

#include "builders/common_objects/account_asset_builder.hpp"
#include "builders/common_objects/account_builder.hpp"
#include "builders/common_objects/amount_builder.hpp"
#include "builders/common_objects/asset_builder.hpp"
#include "builders/common_objects/domain_builder.hpp"
#include "builders/common_objects/peer_builder.hpp"
#include "builders/common_objects/signature_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "builders/protobuf/common_objects/proto_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_domain_builder.hpp"
#include "builders/protobuf/common_objects/proto_peer_builder.hpp"
#include "builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "builders/protobuf/transaction_responses/proto_transaction_status_builder.hpp"
#include "builders/transaction_responses/transaction_status_builder.hpp"
#include "validators/amount_true_validator.hpp"
#include "validators/field_validator.hpp"

namespace shared_model {
  namespace builder {
    using DefaultAccountBuilder = shared_model::builder::AccountBuilder<
        shared_model::proto::AccountBuilder,
        shared_model::validation::FieldValidator>;

    using DefaultAssetBuilder = shared_model::builder::AssetBuilder<
        shared_model::proto::AssetBuilder,
        shared_model::validation::FieldValidator>;

    using DefaultAccountAssetBuilder =
        shared_model::builder::AccountAssetBuilder<
            shared_model::proto::AccountAssetBuilder,
            shared_model::validation::FieldValidator>;

    using DefaultPeerBuilder = shared_model::builder::PeerBuilder<
        shared_model::proto::PeerBuilder,
        shared_model::validation::FieldValidator>;

    using DefaultAmountBuilder = shared_model::builder::AmountBuilder<
        shared_model::proto::AmountBuilder,
        shared_model::validation::FieldValidator>;

    using DefaultDomainBuilder = shared_model::builder::DomainBuilder<
        shared_model::proto::DomainBuilder,
        shared_model::validation::FieldValidator>;

    using DefaultTransactionStatusBuilder =
        shared_model::builder::TransactionStatusBuilder<
            shared_model::proto::TransactionStatusBuilder>;

    using DefaultSignatureBuilder = shared_model::builder::SignatureBuilder<
        shared_model::proto::SignatureBuilder,
        shared_model::validation::FieldValidator>;

    using AmountBuilderWithoutValidator = shared_model::builder::AmountBuilder<
        shared_model::proto::AmountBuilder,
        shared_model::validation::AmountTrueValidator>;
  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_DEFAULT_BUILDERS_HPP
