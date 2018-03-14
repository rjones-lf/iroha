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

#include <gtest/gtest.h>

#include "builders/protobuf/builder_templates/query_response_template.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "interfaces/query_responses/error_responses/stateless_failed_error_response.hpp"

const auto account_id = "test@domain";
const auto asset_id = "bit#domain";
// const auto domain_id = "domain";
// const auto precision = 2;
const auto amount = "100.0";
boost::multiprecision::uint256_t valid_value = 1000;
auto valid_precision = 1;
const shared_model::interface::types::DetailType account_detail =
    "account-detail";

const auto query_hash = shared_model::interface::types::HashType("hashhash");

template <class T>
using w = shared_model::detail::PolymorphicWrapper<T>;

TEST(QueryResponseBuilderTest, AccountAssetResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .accountAssetResponse(asset_id, account_id, amount)
          .build();

  const auto tmp = boost::get<w<shared_model::interface::AccountAssetResponse>>(
      query_response.get());
  const auto &asset_response = tmp->accountAsset();

  shared_model::proto::AmountBuilder amountBuilder;
  auto amount =
      amountBuilder.intValue(valid_value).precision(valid_precision).build();

  ASSERT_EQ(asset_response.assetId(), asset_id);
  ASSERT_EQ(asset_response.accountId(), account_id);
  ASSERT_EQ(asset_response.balance(), amount);
  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, AccountDetailResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .accountDetailResponse(account_detail)
          .build();

  const auto account_detail_response =
      boost::get<w<shared_model::interface::AccountDetailResponse>>(
          query_response.get());

  ASSERT_EQ(account_detail_response->detail(), account_detail);
  ASSERT_EQ(query_response.queryHash(), query_hash);
}

TEST(QueryResponseBuilderTest, ErrorQueryResponse) {
  shared_model::proto::TemplateQueryResponseBuilder<> builder;
  shared_model::proto::QueryResponse query_response =
      builder.queryHash(query_hash)
          .errorQueryResponse<
              shared_model::interface::StatelessFailedErrorResponse>()
          .build();

  const auto error_response =
      boost::get<w<shared_model::interface::ErrorQueryResponse>>(
          query_response.get());

  ASSERT_NO_THROW(
      boost::get<w<shared_model::interface::StatelessFailedErrorResponse>>(
          error_response->get()));
}
