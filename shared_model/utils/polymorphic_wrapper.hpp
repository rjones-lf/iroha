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

#ifndef IROHA_POLYMORPHIC_WRAPPER_HPP
#define IROHA_POLYMORPHIC_WRAPPER_HPP

#include <memory>
#include <utility>

namespace shared_model {
  namespace detail {

    /**
     * Wrapper for polymorphic types for correct working with constructors
     * @tparam T - type of wrapped object
     */
    template <class T>
    class PolymorphicWrapper final {
      template <typename>
      friend class PolymorphicWrapper;

     public:
      /// Type of wrapped object
      using WrappedType = T;

      PolymorphicWrapper() = delete;

      PolymorphicWrapper(std::shared_ptr<T> shp) : ptr_(shp) {}

      /**
       * Copy constructor that performs deep copy
       * @param rhs - another wrapped value
       */
      PolymorphicWrapper(const PolymorphicWrapper &rhs)
          : ptr_(std::move(clone(*rhs.ptr_))) {}

      /**
       * Move constructor
       * @param rhs - wrapped temporary value
       */
      PolymorphicWrapper(PolymorphicWrapper &&rhs) noexcept : ptr_(nullptr) {
        std::swap(this->ptr_, rhs.ptr_);
      }

      /**
       * Value constructor
       * @param value - pointer for wrapping
       */
      template <typename Y,
                typename = std::enable_if_t<std::is_base_of<T, Y>::value>>
      explicit PolymorphicWrapper(Y *value) : ptr_(value) {}

      template <typename Y,
                typename = std::enable_if_t<std::is_base_of<T, Y>::value>>
      PolymorphicWrapper(const PolymorphicWrapper<Y> &rhs)
          : ptr_(std::move(clone(*rhs.ptr_))) {}

      template <typename Y,
                typename = std::enable_if_t<std::is_base_of<T, Y>::value>>
      PolymorphicWrapper(PolymorphicWrapper<Y> &&rhs) noexcept
          : ptr_(std::move(rhs.ptr_)) {
        rhs.ptr_ = nullptr;
      }

      /**
       * Copy operator=
       * @param rhs - another wrapped value
       * @return *this
       */
      PolymorphicWrapper &operator=(const PolymorphicWrapper &rhs) {
        ptr_ = std::move(clone(*rhs.ptr_));
        return *this;
      }

      /**
       * Move operator=
       * @param rhs - another temporary wrapped value
       * @return *this
       */
      PolymorphicWrapper &operator=(PolymorphicWrapper &&rhs) noexcept {
        std::swap(this->ptr_, rhs.ptr_);
        return *this;
      }

      /**
       * Checks equality of objects inside
       * @param rhs - other wrapped value
       * @return true, if wrapped objects are same
       */
      bool operator==(const PolymorphicWrapper &rhs) const {
        return *ptr_ == *rhs.ptr_;
      }

      /**
       * Mutable wrapped object pointer
       * @return pointer for wrapped object
       */
      WrappedType *operator->() {
        return ptr_.get();
      }

      /**
       * Immutable wrapped object pointer
       * @return pointer for wrapped object
       */
      const WrappedType *operator->() const {
        return ptr_.get();
      }

      /**
       * Mutable wrapped object
       * @return pointer for wrapped object
       */
      WrappedType &operator*() {
        return *ptr_;
      }

      /**
       * Immutable wraped object
       * @return pointer for wrapped object
       */
      const WrappedType &operator*() const {
        return *ptr_;
      }

     private:
      /// pointer with wrapped value
      std::shared_ptr<WrappedType> ptr_;
    };

    template <class T, class... Args>
    PolymorphicWrapper<T> makePolymorphic(Args &&... args) {
      return PolymorphicWrapper<T>(
          std::make_shared<T>(std::forward<Args>(args)...));
    }

  }  // namespace detail
}  // namespace shared_model

#endif  // IROHA_POLYMORPHIC_WRAPPER_HPP
