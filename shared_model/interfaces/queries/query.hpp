/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_QUERY_HPP
#define IROHA_SHARED_MODEL_QUERY_HPP

#include <boost/variant.hpp>

#include "interfaces/base/signable.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/queries/get_account.hpp"
#include "interfaces/queries/get_account_asset_transactions.hpp"
#include "interfaces/queries/get_account_assets.hpp"
#include "interfaces/queries/get_account_detail.hpp"
#include "interfaces/queries/get_account_transactions.hpp"
#include "interfaces/queries/get_asset_info.hpp"
#include "interfaces/queries/get_role_permissions.hpp"
#include "interfaces/queries/get_roles.hpp"
#include "interfaces/queries/get_signatories.hpp"
#include "interfaces/queries/get_transactions.hpp"
#include "interfaces/queries/get_pending_transactions.hpp"
#include "interfaces/queries/query_payload_meta.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Class Query provides container with one of concrete query available in
     * system.
     * General note: this class is container for queries but not a base class.
     */
    class Query : public Signable<Query> {
     private:
      /// Shortcut type for const reference
      template <typename... Value>
      using wrap = boost::variant<const Value &...>;

     public:
      /// Type of variant, that handle concrete query
      using QueryVariantType = wrap<GetAccount,
                                    GetSignatories,
                                    GetAccountTransactions,
                                    GetAccountAssetTransactions,
                                    GetTransactions,
                                    GetAccountAssets,
                                    GetAccountDetail,
                                    GetRoles,
                                    GetRolePermissions,
                                    GetAssetInfo,
                                    GetPendingTransactions>;

      /// Types of concrete commands, in attached variant
      using QueryListType = QueryVariantType::types;

      /**
       * @return reference to const variant with concrete command
       */
      virtual const QueryVariantType &get() const = 0;

      /**
       * @return id of query creator
       */
      virtual const types::AccountIdType &creatorAccountId() const = 0;

      /**
       * Query counter - incremental variable reflect for number of sent to
       * system queries plus 1. Required for preventing replay attacks.
       * @return attached query counter
       */
      virtual types::CounterType queryCounter() const = 0;

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override;

      bool operator==(const ModelType &rhs) const override;
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_QUERY_HPP
