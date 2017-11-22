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
     * Structure designed to contain pointer or value depending on called ctor
     * @tparam T
     * @tparam V
     */
    template <typename T, typename V = const T &>
    class ReferenceHolder {
     private:
      using MapperType = std::function<const V &(const T &)>;

      //  T_T*
      using VariantType = boost::variant<T, const T *>;

      static const V &identity(const T &x) { return x; }

     public:
      ReferenceHolder(T &&value, const MapperType &mapper = identity)
          : ReferenceHolder(VariantType(std::move(value)), mapper) {}

      ReferenceHolder(const T &ref, const MapperType &mapper = identity)
          : ReferenceHolder(VariantType(&ref), mapper) {}

      using PointerType = typename std::add_pointer_t<V>;

      const V &operator*() const { return *ptr(); }

      const PointerType ptr() const { return value_.ptr(); }

      const PointerType operator->() const { return ptr(); }

     private:
      ReferenceHolder(VariantType &&variant, const MapperType &mapper)
          : variant_(std::move(variant)), value_([mapper, this]() -> const V & {
              return mapper(boost::apply_visitor(getter_visitor_, variant_));
            }) {}

      class GetterVisitor : public boost::static_visitor<const T &> {
       public:
        const T &operator()(const T &value) const { return value; }
        const T &operator()(const T *ref) const { return *ref; }
      } getter_visitor_;

      VariantType variant_;

      LazyInitializer<const V &> value_;
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
