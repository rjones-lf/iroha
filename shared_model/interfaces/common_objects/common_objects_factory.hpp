/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_COMMON_OBJECTS_FACTORY_HPP
#define IROHA_COMMON_OBJECTS_FACTORY_HPP

#include <memory>

#include "common/result.hpp"
#include "interfaces/common_objects/account.hpp"
#include "interfaces/common_objects/peer.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    /**
     * CommonObjectsFactory provides methods to construct simple objects
     * such as peer, account etc.
     */
    class CommonObjectsFactory {
     public:
      template <typename T>
      using FactoryResult = iroha::expected::Result<T, std::string>;

      /**
       * Create peer instance
       */
      virtual FactoryResult<std::unique_ptr<Peer>> createPeer(
          const types::AddressType &address,
          const types::PubkeyType &public_key) = 0;

      /**
       * Create account instance
       */
      virtual FactoryResult<std::unique_ptr<Account>> createAccount(
          const types::AccountIdType &account_id,
          const types::DomainIdType &domain_id,
          types::QuorumType quorum,
          const types::JsonType &jsonData) = 0;

      /**
       * Create account asset instance
       */
      virtual FactoryResult<std::unique_ptr<AccountAsset>> createAccountAsset(
          const types::AccountIdType &account_id,
          const types::AssetIdType &asset_id,
          const Amount &balance) = 0;

      /**
       * Create amount instance
       *
       * @param value integer will be divided by 10 * precision,
       * so value 123 with precision 2 will become Amount of 1.23
       */
      virtual FactoryResult<std::unique_ptr<Amount>> createAmount(
          boost::multiprecision::uint256_t value,
          types::PrecisionType precision) = 0;

      virtual ~CommonObjectsFactory() = default;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_COMMONOBJECTSFACTORY_HPP
