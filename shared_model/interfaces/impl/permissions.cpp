/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/permissions.hpp"

using namespace shared_model::interface;

namespace shared_model {
  namespace interface {
    namespace permissions {

      Role permissionFor(Grantable g) {
        switch (g) {
          case Grantable::kAddMySignatory:
            return Role::kAddMySignatory;
          case Grantable::kRemoveMySignatory:
            return Role::kRemoveMySignatory;
          case Grantable::kSetMyQuorum:
            return Role::kSetMyQuorum;
          case Grantable::kSetMyAccountDetail:
            return Role::kSetMyAccountDetail;
          case Grantable::kTransferMyAssets:
            return Role::kTransferMyAssets;
          default:;
        }
        return Role::COUNT;
      }

      Grantable permissionOf(Role g) {
        switch (g) {
          case Role::kAddMySignatory:
            return Grantable::kAddMySignatory;
          case Role::kRemoveMySignatory:
            return Grantable::kRemoveMySignatory;
          case Role::kSetMyQuorum:
            return Grantable::kSetMyQuorum;
          case Role::kSetMyAccountDetail:
            return Grantable::kSetMyAccountDetail;
          case Role::kTransferMyAssets:
            return Grantable::kTransferMyAssets;
          default:;
        }
        return Grantable::COUNT;
      }
    }  // namespace permissions
  }    // namespace interface
}  // namespace shared_model

template <typename Perm>
PermissionSet<Perm>::PermissionSet(std::initializer_list<Perm> list) {
  append(list);
}

template <typename Perm>
PermissionSet<Perm> &PermissionSet<Perm>::append(
    std::initializer_list<Perm> list) {
  for (auto l : list) {
    set(l);
  }
  return *this;
}

template <typename Perm>
PermissionSet<Perm> &PermissionSet<Perm>::set(Perm p) {
  Parent::set(bit(p), true);
  return *this;
}

template <typename Perm>
PermissionSet<Perm> &PermissionSet<Perm>::unset(Perm p) {
  Parent::set(bit(p), false);
  return *this;
}

template <typename Perm>
bool PermissionSet<Perm>::operator[](Perm p) const {
  return Parent::operator[](bit(p));
}

template <typename Perm>
bool PermissionSet<Perm>::test(Perm p) const {
  return PermissionSet<Perm>::Parent::test(bit(p));
}

template <typename Perm>
bool PermissionSet<Perm>::isSubsetOf(const PermissionSet<Perm> &r) const {
  return (*this & r) == *this;
}

template <typename Perm>
bool PermissionSet<Perm>::operator==(const PermissionSet<Perm> &r) const {
  return Parent::operator==(r);
}

template <typename Perm>
bool PermissionSet<Perm>::operator!=(const PermissionSet<Perm> &r) const {
  return Parent::operator!=(r);
}

template <typename Perm>
PermissionSet<Perm> &PermissionSet<Perm>::operator&=(
    const PermissionSet<Perm> &r) {
  Parent::operator&=(r);
  return *this;
}

template <typename Perm>
PermissionSet<Perm> &PermissionSet<Perm>::operator|=(
    const PermissionSet<Perm> &r) {
  Parent::operator|=(r);
  return *this;
}

template <typename Perm>
PermissionSet<Perm> &PermissionSet<Perm>::operator^=(
    const PermissionSet<Perm> &r) {
  Parent::operator^=(r);
  return *this;
}

template <typename Perm>
void PermissionSet<Perm>::iterate(std::function<void(Perm)> f) const {
  for (size_t i = 0; i < size(); ++i) {
    auto perm = static_cast<Perm>(i);
    if (operator[](perm)) {
      f(perm);
    }
  }
}

template class shared_model::interface::PermissionSet<permissions::Role>;
template class shared_model::interface::PermissionSet<permissions::Grantable>;
