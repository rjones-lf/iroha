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

#pragma once

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include "visitor.hpp"

namespace iroha {

  namespace result {

    template <typename T>
    struct Ok_t {
      Ok_t(T&& v) : value{std::forward<T>(v)} {}

      Ok_t<T>& operator=(T&& v) {
        value = std::forward<T>(v);
        return *this;
      }

      T value;
    };

    template <typename E>
    struct Error_t {
      Error_t(E&& e) : reason{std::forward<E>(e)} {}

      Error_t<E>& operator=(E&& e) {
        reason = std::forward<E>(e);
        return *this;
      }

      E reason;
    };

    template <typename T>
    constexpr auto Ok(T&& e) {
      return Ok_t<T>{std::forward<T>(e)};
    }

    template <typename E>
    constexpr auto Error(E&& e) {
      return Error_t<E>{std::forward<E>(e)};
    }

    template <typename T, typename Transform>
    constexpr auto operator|(T t, Transform f) -> decltype(f(*t)) {
      return t ? f(*t) : t;
    }

    /**
     * Represents "result of a function" type. Is either value, or error.
     * @tparam V value type
     * @tparam E error type
     */
    template <typename V, typename E>
    class result final {
     public:
      /// basic types
      using pointer_type = V*;
      using pointer_const_type = const V*;
      using Et = Error_t<E>;
      using Vt = Ok_t<V>;
      using R = result<Vt, Et>;
      template <typename T>
      using Ok_f = std::function<T(Vt const&)>;
      template <typename T>
      using Err_f = std::function<T(Et const&)>;
      using bad_get = boost::bad_get;

      /// initializing constructor
      // construct from value
      result(const V& v) : result(Ok(v)) {}
      result(V&& v) : result(Ok(std::forward<V>(v))) {}
      // construct from Ok
      result(const Vt& e) : _(e) {}
      result(Vt&& e) : _(std::move(e)) {}
      // construct from Error
      result(const Et& e) : _(e) {}
      result(Et&& e) : _(std::move(e)) {}

      /// copy constructor
      result(R const& other) { _ = other._; }
      result(R&& other) noexcept { _ = std::move(other._); }

      /// default destructor
      ~result() = default;

      /// boolean operators
      explicit operator bool() const noexcept { return _.which() == 0; }
      bool operator!() const noexcept { return _.which() == 1; }

      /// dereference operator
      V&& operator*() && { return std::move(this->get_value()); }
      V& operator*() & { return this->get_value(); }
      V const& operator*() const & { return this->get_value(); }

      /// arrow operator
      pointer_type operator->() { return std::ref(this->get_value()); }
      pointer_const_type operator->() const {
        return std::cref(this->get_value());
      }

      /// access operators
      // throw result::bad_get if you're trying to get Error while result has
      // Value, and vice versa
      E error() { return this->get_error(); }
      V ok() { return this->get_value(); }
      const V& error() const { return this->get_error(); }
      const V& ok() const { return this->get_value(); }

      /// operator=
      result& operator=(R const& rhs) {
        _ = rhs;
        return *this;
      }
      result& operator=(R&& rhs) noexcept {
        _ = std::move(rhs);
        return *this;
      }
      // from Value
      result& operator=(V const& val) {
        _ = Ok(val);
        return *this;
      }
      result& operator=(V&& val) {
        _ = Ok(std::move(val));
        return *this;
      }
      // from Ok(Value)
      result& operator=(Vt const& val) {
        _ = val;
        return *this;
      }
      result& operator=(Vt&& val) {
        _ = std::move(val);
        return *this;
      }
      // from Error
      result& operator=(Et const& val) {
        _ = val;
        return *this;
      }
      result& operator=(Et&& val) {
        _ = std::move(val);
        return *this;
      }

      /// visitor
      // first OK, then Error
      template <typename T>
      constexpr T match(Ok_f<T> f1, Err_f<T> f2) {
        return boost::apply_visitor(make_visitor(f1, f2), _);
      }
      // first Error, then OK
      template <typename T>
      constexpr T match(Err_f<T> f1, Ok_f<T> f2) {
        return boost::apply_visitor(make_visitor(f2, f1), _);
      }

     private:
      inline V& get_value() { return boost::get<Vt>(_).value; }
      inline E& get_error() { return boost::get<Et>(_).reason; }

      inline V const& get_value() const {
        return std::cref(boost::get<Vt>(_).value);
      }

      inline E const& get_error() const {
        return std::cref(boost::get<Et>(_).reason);
      }

      boost::variant<Vt, Et> _;
    };
  }

}  // namespace iroha
