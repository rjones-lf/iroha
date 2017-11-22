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
     * Structure designed to contain reference to const
     * or value depending on called ctor
     * @tparam T type of stored value
     */
    template <typename T>
    class ReferenceHolder {
     private:
      using VariantType = boost::variant<T, const T &>;

     public:
      template <typename V>
      explicit ReferenceHolder(V &&value) : variant_(std::forward<V>(value)) {}

      using ReferenceType = typename std::add_lvalue_reference_t<const T>;
      using PointerType = typename std::add_pointer_t<const T>;

      ReferenceType operator*() const { return *ptr(); }

      PointerType operator->() const { return ptr(); }

      PointerType ptr() const {
        return &boost::apply_visitor(getter_visitor_, variant_);
      }

      VariantType &variant() { return variant_; }

     private:
      class GetterVisitor : public boost::static_visitor<const T &> {
       public:
        const T &operator()(const T &value) const { return value; }
      } getter_visitor_;

      VariantType variant_;
    };

    /**
     * Create function which will return ref-to-const member by calling
     * corresponding member function
     * @tparam T - member function type
     * @param member pointer-to-member function
     * @return ref-to-const member
     */
    template <typename T>
    inline auto makeReferenceGetter(T member) {
      // capture by value since member temporary will be destroyed after return
      return [=](const auto &i) -> const auto & { return (i.*member)(); };
    }
  }  // namespace detail
}  // namespace shared_model

#endif  // IROHA_REFERENCE_HOLDER_HPP
