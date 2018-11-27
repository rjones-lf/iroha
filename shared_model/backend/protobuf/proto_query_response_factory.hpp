/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_QUERY_RESPONSE_FACTORY_HPP
#define IROHA_PROTO_QUERY_RESPONSE_FACTORY_HPP

#include "interfaces/iroha_internal/query_response_factory.hpp"

namespace shared_model {
  namespace proto {

    class ProtoQueryResponseFactory : public interface::QueryResponseFactory {
     public:
      std::unique_ptr<interface::QueryResponse> createAccountAssetResponse(
          std::vector<std::tuple<interface::types::AccountIdType,
                                 interface::types::AssetIdType,
                                 shared_model::interface::Amount>> assets,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::QueryResponse> createAccountDetailResponse(
          interface::types::DetailType account_detail,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::QueryResponse> createAccountResponse(
          interface::types::AccountIdType account_id,
          interface::types::DomainIdType domain_id,
          interface::types::QuorumType quorum,
          interface::types::JsonType jsonData,
          std::vector<std::string> roles,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::QueryResponse> createErrorQueryResponse(
          ErrorQueryType error_type,
          std::string error_msg,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::QueryResponse> createSignatoriesResponse(
          std::vector<interface::types::PubkeyType> signatories,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::QueryResponse> createTransactionsResponse(
          std::vector<std::unique_ptr<shared_model::interface::Transaction>>
              transactions,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::QueryResponse> createTransactionsPageResponse(
          std::vector<std::unique_ptr<shared_model::interface::Transaction>>
              transactions,
          const crypto::Hash &next_tx_hash,
          interface::types::TransactionsNumberType all_transactions_size,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::QueryResponse> createAssetResponse(
          interface::types::AssetIdType asset_id,
          interface::types::DomainIdType domain_id,
          interface::types::PrecisionType precision,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::QueryResponse> createRolesResponse(
          std::vector<interface::types::RoleIdType> roles,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::QueryResponse> createRolePermissionsResponse(
          interface::RolePermissionSet role_permissions,
          const crypto::Hash &query_hash) const override;

      std::unique_ptr<interface::BlockQueryResponse> createBlockQueryResponse(
          std::unique_ptr<interface::Block> block) const override;

      std::unique_ptr<interface::BlockQueryResponse> createBlockQueryResponse(
          std::string error_message) const override;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_QUERY_RESPONSE_FACTORY_HPP
