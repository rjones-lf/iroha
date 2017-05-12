/*
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * http://soramitsu.co.jp
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_UTILS_EXCEPTION_HPP_
#define IROHA_UTILS_EXCEPTION_HPP_

#include "exception.hpp"

class UnexpectedType {
 public:
  explicit UnexpectedType(std::exception_ptr&& excptr) noexcept;
  const std::exception_ptr& excptr() const noexcept;

 private:
  std::exception_ptr excptr_;
};

template <typename ExceptionType>
UnexpectedType makeUnexpected(
    ExceptionType&& exc,
    typename std::enable_if<
        std::is_base_of<std::exception, ExceptionType>::value>::type* =
        nullptr) noexcept {
  return UnexpectedType(std::make_exception_ptr(exc));
}

template <typename ExceptionPtrType>
UnexpectedType makeUnexpected(
    ExceptionPtrType&& excptr,
    typename std::enable_if<
        std::is_same<std::exception_ptr, ExceptionPtrType>::value>::type* =
        nullptr) noexcept {
  return UnexpectedType(std::forward<ExceptionPtrType>(excptr));
}

template <typename T>
class Expected {
 public:
  Expected(
      T&& value,
      typename std::enable_if<!std::is_same<T, UnexpectedType>::value>::type* =
          0) noexcept
      : value_(std::forward<T>(value)),
        excptr_(std::make_exception_ptr(exception::None())),
        valid_(true) {}

  Expected(const UnexpectedType& exc) noexcept
      : value_(T()), excptr_(exc.excptr()), valid_(false) {}

  bool valid() const noexcept { return valid_; }

  explicit operator bool() const noexcept { return valid(); }

  const T& value() const & {
    if (!valid()) std::rethrow_exception(excptr());
    return value_;
  }

  T&& value_move() {
    if (!valid()) std::rethrow_exception(excptr());
      return std::move(value_);
  }


  const T& operator*() const { return value(); }

  void move_value(T& to) {
    T temp(std::move(value_));
    value_ = std::move(to);
    to = std::move(temp);
  }

  std::exception_ptr excptr() const noexcept { return excptr_; }

  std::string error() const {
    try {
      std::rethrow_exception(excptr_);
    } catch (const exception::IrohaException& e) {
      return e.message();
    }
  }

 private:
  T value_;
  std::exception_ptr excptr_;
  bool valid_;
};

class VoidHandler {
 public:
  VoidHandler() noexcept;

  template <typename T>
  VoidHandler(
      T&& value,
      typename std::enable_if<!std::is_same<T, UnexpectedType>::value>::type* =
          0) noexcept
      : excptr_(std::make_exception_ptr(exception::None())), valid_(true) {}

  VoidHandler(const UnexpectedType& exc) noexcept;

  bool valid() const noexcept { return valid_; }
  explicit operator bool() const noexcept { return valid(); }

  std::exception_ptr excptr() const noexcept { return excptr_; }

  std::string error() const;

 private:
  std::exception_ptr excptr_;
  bool valid_;
};

#endif  // IROHA_UTILS_EXCEPTION_HPP_
