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
        : protected std::bitset<static_cast<size_t>(Perm::COUNT)> {
     private:
      using Parent = std::bitset<static_cast<size_t>(Perm::COUNT)>;

     public:
      using Parent::all;
      using Parent::any;
      using Parent::none;
      using Parent::reset;
      using Parent::size;
      using Parent::operator==;
      using Parent::operator!=;

      PermissionSet(std::initializer_list<Perm> list) {
        append(list);
      }

      PermissionSet &append(std::initializer_list<Perm> list) {
        for (auto l : list) {
          set(l);
        }
        return *this;
      }

      PermissionSet &set(Perm p) {
        Parent::set(bit(p), true);
        return *this;
      }

      PermissionSet &unset(Perm p) {
        Parent::set(bit(p), false);
        return *this;
      }

      constexpr bool operator[](Perm p) const {
        return Parent::operator[](bit(p));
      }

      bool test(Perm p) const {
        return Parent::test(bit(p));
      }

     private:
      constexpr auto bit(Perm p) const {
        return static_cast<size_t>(p);
      }
    };

    template class PermissionSet<permissions::Role>;
    template class PermissionSet<permissions::Grantable>;
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_TRANSACTION_HPP
