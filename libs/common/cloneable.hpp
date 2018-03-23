/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_CLONEABLE_HPP
#define IROHA_CLONEABLE_HPP

#include <memory>

/**
 * Functions and interface for creation cloneable classes with
 * smart pointers.
 *
 * Usage:
 * struct Base : object::cloneable<Base> {
 *   // ... other methods
 * };
 *
 * struct Derived : Base {
 *   // ... other methods
 * protected:
 *   Derived* clone() const override {
 *     return new Derived(*this);
 *   }
 * };
 *
 * Derived derived;
 * auto c1 = clone(derived);
 */

/**
 * Function to clone from Cloneable.
 * @tparam T - derived from Cloneable
 * @param object - object to clone
 * @return clone of object
 */
template <typename T>
std::unique_ptr<T> clone(const T &object) {
  using base_type = typename T::base_type;
  static_assert(std::is_base_of<base_type, T>::value,
                "T object has to derived from T::base_type");
  auto ptr = static_cast<const base_type &>(object).clone();
  return std::unique_ptr<T>(static_cast<T *>(ptr));
}

/**
 * Helper function to copy from pointer to Cloneable.
 * @tparam T - derived from Cloneable
 * @param object - object to clone
 * @return clone of object
 */
template <typename T>
auto clone(T *object) {
  return clone(*object);
}

/**
 * Interface for cloneable classes.
 * @tparam T
 */
template <typename T>
class Cloneable {
 public:
  using base_type = T;

  virtual ~Cloneable() = default;

 protected:
  /**
   * Polymorphic clone constructor.
   * Method guarantees deep-copy.
   * @return pointer to cloned object
   */
  virtual T *clone() const = 0;

  template <typename X>
  friend std::unique_ptr<X> clone(const X &);
};

#endif  // IROHA_CLONEABLE_HPP
