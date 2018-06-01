/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PERMISSIONS_HPP
#define IROHA_SHARED_MODEL_PERMISSIONS_HPP

#include <bitset>
#include <initializer_list>

namespace shared_model {
  namespace interface {
    namespace permissions {
      enum class Role {
        kAppendRole,
        kCreateRole,
        kDetachRole,
        kAddAssetQty,
        kSubtractAssetQty,
        kAddPeer,
        kAddSignatory,
        kRemoveSignatory,
        kSetQuorum,
        kCreateAccount,
        kSetDetail,
        kCreateAsset,
        kTransfer,
        kReceive,
        kCreateDomain,
        kReadAssets,
        kGetRoles,
        kGetMyAccount,
        kGetAllAccounts,
        kGetDomainAccounts,
        kGetMySignatories,
        kGetAllSignatories,
        kGetDomainSignatories,
        kGetMyAccAst,
        kGetAllAccAst,
        kGetDomainAccAst,
        kGetMyAccDetail,
        kGetAllAccDetail,
        kGetDomainAccDetail,
        kGetMyAccTxs,
        kGetAllAccTxs,
        kGetDomainAccTxs,
        kGetMyAccAstTxs,
        kGetAllAccAstTxs,
        kGetDomainAccAstTxs,
        kGetMyTxs,
        kGetAllTxs,
        kSetMyQuorum,
        kAddMySignatory,
        kRemoveMySignatory,
        kTransferMyAssets,
        kSetMyAccountDetail,
        kGetBlocks,

        COUNT
      };

      enum class Grantable {
        kAddMySignatory,
        kRemoveMySignatory,
        kSetMyQuorum,
        kSetMyAccountDetail,
        kTransferMyAssets,

        COUNT
      };
    }  // namespace permissions

    template <typename Perm>
    class PermissionSet
        : private std::bitset<static_cast<size_t>(Perm::COUNT)> {
     private:
      using Parent = std::bitset<static_cast<size_t>(Perm::COUNT)>;

     public:
      using Parent::Parent;
      using Parent::reset;
      using Parent::size;
      explicit PermissionSet(std::initializer_list<Perm> list);

      PermissionSet &append(std::initializer_list<Perm> list);

      PermissionSet &set(Perm p);
      PermissionSet &unset(Perm p);

      bool operator[](Perm p) const;
      bool test(Perm p) const;

      bool isSubsetOf(const PermissionSet<Perm> &r) const;

      bool operator==(const PermissionSet<Perm> &r) const;
      bool operator!=(const PermissionSet<Perm> &r) const;
      PermissionSet<Perm> &operator&=(const PermissionSet<Perm> &r);
      PermissionSet<Perm> &operator|=(const PermissionSet<Perm> &r);
      PermissionSet<Perm> &operator^=(const PermissionSet<Perm> &r);

     private:
      constexpr auto bit(Perm p) const {
        return static_cast<size_t>(p);
      }
    };

    extern template class PermissionSet<permissions::Role>;
    extern template class PermissionSet<permissions::Grantable>;

    using RolePermissionSet = PermissionSet<permissions::Role>;
    using GrantablePermissionSet = PermissionSet<permissions::Grantable>;
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_TRANSACTION_HPP
