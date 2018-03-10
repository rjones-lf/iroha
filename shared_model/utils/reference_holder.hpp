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

#ifndef IROHA_REFERENCE_HOLDER_HPP
#define IROHA_REFERENCE_HOLDER_HPP

#include <boost/variant.hpp>
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace detail {
    /**
     * Container designed to store reference or value depending on called ctor
     * @tparam T type of stored value
     */
    template <typename T>
    class ReferenceHolder {
     private:
      using VariantType = boost::variant<T, T &>;

     public:
      template <typename V>
      explicit ReferenceHolder(V &&value) : variant_(std::forward<V>(value)) {}

      using ReferenceType = typename std::add_lvalue_reference_t<T>;
      using PointerType = typename std::add_pointer_t<T>;

      using ConstReferenceType = typename std::add_lvalue_reference_t<const T>;
      using ConstPointerType = typename std::add_pointer_t<const T>;

      ReferenceType operator*() {
        return *ptr();
      }

      PointerType operator->() {
        return ptr();
      }

      PointerType ptr() {
        return &boost::apply_visitor(
            [](auto &value) -> decltype(auto) { return (value); }, variant_);
      }

      ConstReferenceType operator*() const {
        return *ptr();
      }

      ConstPointerType operator->() const {
        return ptr();
      }

      ConstPointerType ptr() const {
        return &boost::apply_visitor(
            [](const auto &value) -> decltype(auto) { return (value); },
            variant_);
      }

     private:
      VariantType variant_;
    };
  }  // namespace detail
}  // namespace shared_model

#endif  // IROHA_REFERENCE_HOLDER_HPP
