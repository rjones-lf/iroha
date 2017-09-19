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

#include "model/converters/pb_query_response_factory.hpp"
#include "model/converters/pb_transaction_factory.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      nonstd::optional<protocol::QueryResponse>
      PbQueryResponseFactory::serialize(
          const std::shared_ptr<QueryResponse> query_response) const {
        nonstd::optional<protocol::QueryResponse> response = nonstd::nullopt;
        if (instanceof <model::ErrorResponse>(*query_response)) {
          response = nonstd::make_optional<protocol::QueryResponse>();
          auto er = static_cast<model::ErrorResponse &>(*query_response);
          auto pb_er = serializeErrorResponse(er);
          response->mutable_error_response()->CopyFrom(pb_er);
        }
        if (instanceof <model::AccountAssetResponse>(*query_response)) {
          response = nonstd::make_optional<protocol::QueryResponse>();
          response->mutable_account_assets_response()->CopyFrom(
              serializeAccountAssetResponse(
                  static_cast<model::AccountAssetResponse &>(*query_response)));
        }
        if (instanceof <model::AccountResponse>(*query_response)) {
          response = nonstd::make_optional<protocol::QueryResponse>();
          response->mutable_account_response()->CopyFrom(
              serializeAccountResponse(
                  static_cast<model::AccountResponse &>(*query_response)));
        }
        if (instanceof <model::SignatoriesResponse>(*query_response)) {
          response = nonstd::make_optional<protocol::QueryResponse>();
          response->mutable_signatories_response()->CopyFrom(
              serializeSignatoriesResponse(
                  static_cast<model::SignatoriesResponse &>(*query_response)));
        }
        if (instanceof <model::TransactionsResponse>(*query_response)) {
          response = nonstd::make_optional<protocol::QueryResponse>();
          response->mutable_transactions_response()->CopyFrom(
              serializeTransactionsResponse(
                  static_cast<model::TransactionsResponse &>(*query_response)));
        }
        return response;
      }

      protocol::Account PbQueryResponseFactory::serializeAccount(
          const model::Account &account) const {
        protocol::Account pb_account;
        pb_account.set_quorum(account.quorum);
        pb_account.set_account_id(account.account_id);
        pb_account.set_domain_name(account.domain_name);

        auto permissions = pb_account.mutable_permissions();
        permissions->set_set_quorum(account.permissions.set_quorum);
        permissions->set_set_permissions(account.permissions.set_permissions);
        permissions->set_remove_signatory(account.permissions.remove_signatory);
        permissions->set_read_all_accounts(
            account.permissions.read_all_accounts);
        permissions->set_issue_assets(account.permissions.issue_assets);
        permissions->set_create_domains(account.permissions.create_domains);
        permissions->set_create_accounts(account.permissions.create_accounts);
        permissions->set_create_assets(account.permissions.create_assets);
        permissions->set_can_transfer(account.permissions.can_transfer);
        permissions->set_add_signatory(account.permissions.add_signatory);

        return pb_account;
      }

      model::Account PbQueryResponseFactory::deserializeAccount(
          const protocol::Account &pb_account) const {
        model::Account res;
        res.account_id = pb_account.account_id();
        res.quorum = pb_account.quorum();
        res.domain_name = pb_account.domain_name();

        res.permissions.add_signatory =
            pb_account.permissions().add_signatory();
        res.permissions.can_transfer = pb_account.permissions().can_transfer();
        res.permissions.create_assets =
            pb_account.permissions().create_assets();
        res.permissions.create_accounts =
            pb_account.permissions().create_accounts();
        res.permissions.create_domains =
            pb_account.permissions().create_domains();
        res.permissions.issue_assets = pb_account.permissions().issue_assets();
        res.permissions.read_all_accounts =
            pb_account.permissions().read_all_accounts();
        res.permissions.remove_signatory =
            pb_account.permissions().remove_signatory();
        res.permissions.set_permissions =
            pb_account.permissions().set_permissions();
        res.permissions.set_quorum = pb_account.permissions().set_quorum();

        return res;
      }

      protocol::AccountResponse
      PbQueryResponseFactory::serializeAccountResponse(
          const model::AccountResponse &accountResponse) const {
        protocol::AccountResponse pb_response;
        pb_response.mutable_account()->CopyFrom(
            serializeAccount(accountResponse.account));
        return pb_response;
      }

      model::AccountResponse PbQueryResponseFactory::deserializeAccountResponse(
          const protocol::AccountResponse pb_response) const {
        model::AccountResponse accountResponse;
        accountResponse.account = deserializeAccount(pb_response.account());
        return accountResponse;
      }

      protocol::AccountAsset PbQueryResponseFactory::serializeAccountAsset(
          const model::AccountAsset &account_asset) const {
        protocol::AccountAsset pb_account_asset;
        pb_account_asset.set_account_id(account_asset.account_id);
        pb_account_asset.set_asset_id(account_asset.asset_id);
        pb_account_asset.set_balance(account_asset.balance);
        return pb_account_asset;
      }

      model::AccountAsset PbQueryResponseFactory::deserializeAccountAsset(
          const protocol::AccountAsset &account_asset) const {
        model::AccountAsset res;
        res.account_id = account_asset.account_id();
        res.balance = account_asset.balance();
        res.asset_id = account_asset.asset_id();
        return res;
      }

      protocol::AccountAssetResponse
      PbQueryResponseFactory::serializeAccountAssetResponse(
          const model::AccountAssetResponse &accountAssetResponse) const {
        protocol::AccountAssetResponse pb_response;
        auto pb_account_asset = pb_response.mutable_account_asset();
        pb_account_asset->set_asset_id(
            accountAssetResponse.acct_asset.asset_id);
        pb_account_asset->set_account_id(
            accountAssetResponse.acct_asset.account_id);
        pb_account_asset->set_balance(accountAssetResponse.acct_asset.balance);
        return pb_response;
      }

      model::AccountAssetResponse
      PbQueryResponseFactory::deserializeAccountAssetResponse(
          const protocol::AccountAssetResponse &account_asset_response) const {
        model::AccountAssetResponse res;
        res.acct_asset.balance =
            account_asset_response.account_asset().balance();
        res.acct_asset.account_id =
            account_asset_response.account_asset().account_id();
        res.acct_asset.asset_id =
            account_asset_response.account_asset().asset_id();
        return res;
      }

      protocol::SignatoriesResponse
      PbQueryResponseFactory::serializeSignatoriesResponse(
          const model::SignatoriesResponse &signatoriesResponse) const {
        protocol::SignatoriesResponse pb_response;

        for (auto key : signatoriesResponse.keys) {
          pb_response.add_keys(key.data(), key.size());
        }
        return pb_response;
      }

      model::SignatoriesResponse
      PbQueryResponseFactory::deserializeSignatoriesResponse(
          const protocol::SignatoriesResponse &signatoriesResponse) const {
        model::SignatoriesResponse res{};
        for (const auto &key : signatoriesResponse.keys()) {
          ed25519::pubkey_t pubkey;
          std::copy(key.begin(), key.end(), pubkey.begin());
          res.keys.push_back(pubkey);
        }
        return res;
      }

      protocol::TransactionsResponse
      PbQueryResponseFactory::serializeTransactionsResponse(
          const model::TransactionsResponse &transactionsResponse) const {
        PbTransactionFactory pb_transaction_factory;

        // converting observable to the vector using reduce
        return transactionsResponse.transactions
            .reduce(protocol::TransactionsResponse(),
                    [&pb_transaction_factory](auto &&response, auto tx) {
                      response.add_transactions()->CopyFrom(
                          pb_transaction_factory.serialize(tx));
                      return response;
                    },
                    [](auto &&response) { return response; })
            .as_blocking()  // we need to wait when on_complete happens
            .first();
      }

      protocol::ErrorResponse PbQueryResponseFactory::serializeErrorResponse(
          const model::ErrorResponse &errorResponse) const {
        protocol::ErrorResponse pb_response;
        switch (errorResponse.reason) {
          case ErrorResponse::STATELESS_INVALID:
            pb_response.set_reason(protocol::ErrorResponse::STATELESS_INVALID);
            break;
          case ErrorResponse::STATEFUL_INVALID:
            pb_response.set_reason(protocol::ErrorResponse::STATEFUL_INVALID);
            break;
          case ErrorResponse::NO_ACCOUNT:
            pb_response.set_reason(protocol::ErrorResponse::NO_ACCOUNT);
            break;
          case ErrorResponse::NO_ACCOUNT_ASSETS:
            pb_response.set_reason(protocol::ErrorResponse::NO_ACCOUNT_ASSETS);
            break;
          case ErrorResponse::NO_SIGNATORIES:
            pb_response.set_reason(protocol::ErrorResponse::NO_SIGNATORIES);
            break;
          case ErrorResponse::NOT_SUPPORTED:
            pb_response.set_reason(protocol::ErrorResponse::NOT_SUPPORTED);
            break;
        }
        return pb_response;
      }
    }
  }
}
