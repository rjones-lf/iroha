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

#ifndef IROHA_LAZY_INITIALIZER_HPP
#define IROHA_LAZY_INITIALIZER_HPP

#include <boost/optional.hpp>
#include <functional>

namespace shared_model {
  namespace detail {

    /**
     * Lazy class for lazy converting one type to another
     * @tparam Target - output type
     */
    template <typename Target>
    class LazyInitializer {
     private:
      /// Type of generator function
      using GeneratorType = std::function<Target()>;

     public:
      /**
       * Default constructor prevents compilation error in pre 7.2 gcc.
       *
       * Short explanation: gcc needs default constructor
       * when the constructor is inherited (via using Base::Base)
       * if the inherited class has default initalizers for any of its members.
       * This can be found in shared_model/backend/protobuf/protobuf/block.hpp
       * where Lazy fields are initialized via default initializers.
       *
       * Note that this does not result in default constructor being called, so
       * the resulting code is still correct. This is an issue of gcc compiler
       * which is resolved in 7.2
       *
       * For more details: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67054
       */
      LazyInitializer();

      template <typename T>
      explicit LazyInitializer(T &&generator)
          : generator_(std::forward<T>(generator)) {}

      using PointerType = typename std::add_pointer_t<Target>;

      const Target &operator*() const {
        return *ptr();
      }

      const PointerType ptr() const {
        if (target_value_ == boost::none) {
          // Use type move constructor with emplace
          // since Target copy assignment operator could be deleted

          target_value_.emplace(generator_());
        }
        return target_value_.get_ptr();
      }

      const PointerType operator->() const {
        return ptr();
      }

      /**
       * Remove generated value. Next ptr() call will generate new value
       */
      void invalidate() const {
        target_value_ = boost::none;
      }

     private:
      GeneratorType generator_;
      mutable boost::optional<Target> target_value_;
    };

    /**
     * Function for creating lazy object
     * @tparam Generator - type of generator
     * @param generator - instance of Generator
     * @return initialized lazy value
     */
    template <typename Generator>
    auto makeLazyInitializer(Generator &&generator) {
      using targetType = decltype(generator());
      return LazyInitializer<targetType>(std::forward<Generator>(generator));
    }
  }  // namespace detail
}  // namespace shared_model
#endif  // IROHA_LAZY_INITIALIZER_HPP
