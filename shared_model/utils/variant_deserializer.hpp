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

#ifndef IROHA_VARIANT_DESERIALIZER_HPP
#define IROHA_VARIANT_DESERIALIZER_HPP

#include <boost/serialization/variant.hpp>

#define NORETURN [[noreturn]]

namespace shared_model {
  namespace detail {
    /**
     * Helper for variant deserialization
     * Iterate through type list to use type specified by type index in list
     * @tparam S list of types
     */
    template <class S>
    struct variant_impl {
      /**
       * Dummy deserializer for empty list
       */
      struct load_null {
        /**
         * Dummy deserializer
         * @tparam V variant type for deserialization
         * @tparam T list of candidate types
         * @tparam Archive container type
         */
        template <class V, class T = typename V::types, class Archive>
        NORETURN static V invoke(Archive &&, int) {
          BOOST_ASSERT_MSG(false, "Required type not found");
          std::abort();
        }
      };

      /**
       * Deserializer implementation
       */
      struct load_impl {
        template <typename T>
        using FrontType = typename boost::mpl::front<T>::type;

        /**
         * Deserialize container in variant using type in list by specified
         * index
         * If type selector is 0, head type is required type, and it is used
         * Otherwise call helper without front element in type list
         * @tparam V variant type for deserialization
         * @tparam T list of candidate types
         * @tparam Archive container type
         * @param ar container to be deserialized
         * @param which type index in list
         * @param v result variant
         */
        template <class V, class T = typename V::types, class Archive>
        static auto invoke(Archive &&ar, int which) -> typename std::enable_if<
            not std::is_same<FrontType<S>, FrontType<T>>::value,
            V>::type {
          if (which == 0) {
            using head_type = FrontType<S>;
            using variant_head_type = FrontType<T>;
            // Given two variant type lists T and S, where T is list of abstract
            // types, and S is list of implementations, ensure that there is a
            // corresponding implementation for each abstract type
            // Probably there is a missing type either in interfaces/.h
            // abstract variant, or backend/type/.h implementation variant
            static_assert(
                std::is_base_of<typename variant_head_type::WrappedType,
                                typename head_type::WrappedType>::value,
                "variant_head_type is not base of head_type");
            return V(head_type(new typename head_type::WrappedType(
                std::forward<Archive>(ar))));
          } else {
            using type = typename boost::mpl::pop_front<S>::type;
            using variant_type = typename boost::mpl::pop_front<T>::type;
            return variant_impl<type>::template load<V, variant_type>(
                std::forward<Archive>(ar), which - 1);
          }
        }

        /**
         * Deserialize container in variant using type in list by specified
         * index
         * If type selector is 0, head type is required type, and it is used
         * Otherwise call helper without front element in type list
         * @tparam V variant type for deserialization
         * @tparam T list of candidate types
         * @tparam Archive container type
         * @param ar container to be deserialized
         * @param which type index in list
         * @param v result variant
         */
        template <class V, class T = typename V::types, class Archive>
        static auto invoke(Archive &&ar, int which) -> typename std::enable_if<
            std::is_same<FrontType<S>, FrontType<T>>::value,
            V>::type {
          if (which == 0) {
            using head_type = FrontType<S>;
            return V(head_type(std::forward<Archive>(ar)));
          } else {
            using type = typename boost::mpl::pop_front<S>::type;
            using variant_type = typename boost::mpl::pop_front<T>::type;
            return variant_impl<type>::template load<V, variant_type>(
                std::forward<Archive>(ar), which - 1);
          }
        }
      };

      /**
       * Deserialize container in variant using type in list by specified index
       * Choose dummy or concrete deserializer depending on type list size
       * @tparam V variant type for deserialization
       * @tparam T list of candidate types
       * @tparam Archive container type
       * @param ar container to be deserialized
       * @param which type index in list
       * @param v result variant
       */
      template <class V, class T = typename V::types, class Archive>
      static V load(Archive &&ar, int which) {
        using typex =
            typename boost::mpl::eval_if<boost::mpl::empty<S>,
                                         boost::mpl::identity<load_null>,
                                         boost::mpl::identity<load_impl>>::type;
        return typex::template invoke<V, T>(std::forward<Archive>(ar), which);
      }
    };
  }  // namespace detail
}  // namespace shared_model

#endif  // IROHA_VARIANT_DESERIALIZER_HPP
