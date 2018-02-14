/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_SHARED_MODEL_PROTO_CONCRETE_ERROR_QUERY_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_CONCRETE_ERROR_QUERY_RESPONSE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/error_responses/no_account_assets_error_response.hpp"
#include "interfaces/query_responses/error_responses/no_account_detail_error_response.hpp"
#include "interfaces/query_responses/error_responses/no_account_error_response.hpp"
#include "interfaces/query_responses/error_responses/no_asset_error_response.hpp"
#include "interfaces/query_responses/error_responses/no_roles_error_response.hpp"
#include "interfaces/query_responses/error_responses/no_signatories_error_response.hpp"
#include "interfaces/query_responses/error_responses/not_supported_error_response.hpp"
#include "interfaces/query_responses/error_responses/stateful_failed_error_response.hpp"
#include "interfaces/query_responses/error_responses/stateless_failed_error_response.hpp"
#include "responses.pb.h"

namespace shared_model {
  namespace proto {
    using StatelessFailedErrorResponse =
        TrivialProto<interface::StatelessFailedErrorResponse,
                     iroha::protocol::ErrorResponse>;
    using StatefulFailedErrorResponse =
        TrivialProto<interface::StatefulFailedErrorResponse,
                     iroha::protocol::ErrorResponse>;
    using NoAccountErrorResponse =
        TrivialProto<interface::NoAccountErrorResponse,
                     iroha::protocol::ErrorResponse>;
    using NoAccountAssetsErrorResponse =
        TrivialProto<interface::NoAccountAssetsErrorResponse,
                     iroha::protocol::ErrorResponse>;
    using NoAccountDetailErrorResponse =
        TrivialProto<interface::NoAccountDetailErrorResponse,
                     iroha::protocol::ErrorResponse>;
    using NoSignatoriesErrorResponse =
        TrivialProto<interface::NoSignatoriesErrorResponse,
                     iroha::protocol::ErrorResponse>;
    using NotSupportedErrorResponse =
        TrivialProto<interface::NotSupportedErrorResponse,
                     iroha::protocol::ErrorResponse>;
    using NoAssetErrorResponse = TrivialProto<interface::NoAssetErrorResponse,
                                              iroha::protocol::ErrorResponse>;
    using NoRolesErrorResponse = TrivialProto<interface::NoRolesErrorResponse,
                                              iroha::protocol::ErrorResponse>;
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_CONCRETE_ERROR_QUERY_RESPONSE_HPP
