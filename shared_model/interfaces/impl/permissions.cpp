/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/permissions.hpp"

using namespace shared_model::interface;

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

template class shared_model::interface::PermissionSet<permissions::Role>;
template class shared_model::interface::PermissionSet<permissions::Grantable>;
