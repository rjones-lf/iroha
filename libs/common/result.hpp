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

#ifndef IROHA_COMMON_RESULT_HPP_
#define IROHA_COMMON_RESULT_HPP_

#include <boost/variant.hpp>
#include "common/visitor.hpp"

namespace iroha {
  namespace result {

    /**
     * @struct Ok_t
     * @brief Type, which represents success of operation.
     * @tparam T any type.
     */
    template <typename T>
    struct Ok_t {
      Ok_t(T &&v) : value(std::forward<T>(v)) {}
      T value;
    };

    /**
     * @struct Error_t
     * @brief Type, which represents failed operation.
     * @tparam E any type.
     */
    template <typename E>
    struct Error_t {
      Error_t(E &&e) : reason(std::forward<E>(e)) {}
      E reason;
    };

    /**
     * @brief Okay indicator. Shortcut that is used to easily create Ok_t.
     *
     * @code
     * Result<int, std::string> f(){
     *   return Ok(5); // returns Ok_t<int>(5)
     * }
     * @endcode
     *
     * @tparam T any type.
     * @param e value.
     * @return Ok_t<T>(e)
     */
    template <typename T>
    constexpr auto Ok(T &&e) {
      return Ok_t<T>{std::forward<T>(e)};
    }

    /**
     * @brief Error indicator. Shortcut that is used to easily create Error_t.
     *
     * @code
     * Result<int, std::string> f() {
     *   return Error("reason"s); // returns Error_t<std::string>("reason"s);
     * }
     * @endcode
     *
     * @tparam E any movable or copyable type
     * @param e reason of error.
     * @return Error_t<E>(e)
     */
    template <typename E>
    constexpr auto Error(E &&e) {
      return Error_t<E>{std::forward<E>(e)};
    }

    /**
     * @brief Monadic bind operator for Result object.
     *
     * @code
     *  using R = iroha::Result<int, std::string>;
     *
     *  R increment_but_less_5(int a) {
     *    return a < 5 ? Ok(a + 1) : Error(":("s);
     *  }
     *
     *  void dothejob() {
     *    R a = 1;
     *    assert(a);  // true
     *    R b = a | increment_but_less_5 | increment_but_less_5;
     *    assert(a);  // true
     *    assert(b);  // true
     *    assert(*b == *a + 2);  //  true
     *  }
     * @endcode
     *
     * @tparam T monadic type.
     * @tparam Transform transformation, which accepts decltype(*t) as first
     * argument.
     * @param t instance of monadic type.
     * @param f instance of transformation.
     * @return applies transformation on t if t is true, returns t otherwise.
     */
    template <typename T, typename Transform>
    constexpr auto operator|(T t, Transform f) -> decltype(f(*t)) {
      if (t) {
        return f(*t);
      } else {
        return t;
      }
    }

    /**
     * @class Result
     * @brief Represents "Result of a function" type. Is either value, or error.
     * @tparam V value type.
     * @tparam E error type.
     */
    template <typename V, typename E>
    class Result final {
      /// shortcuts
      using Et = Error_t<E>;
      using Vt = Ok_t<V>;
      using R = Result<Vt, Et>;

     public:
      /**
       * @brief exception that is thrown by Result if user is trying to get
       * Value, while Result contains Error. And vice versa.
       */
      using bad_get = boost::bad_get;

      /**
       * Alias for V type
       */
      using VType = V;

      /**
       * Alias for E type
       */
      using EType = E;

      /**
       * Alias for Ok_t<V> type
       */
      using OkType = Vt;

      /**
       * Alias for Error_t<E> type
       */
      using ErrorType = Et;

      /// from Value (both lvalue and rvalue)
      Result(V &&v) : m(Ok(std::forward<V>(v))) {}

      /// from Ok_t<V> lvalue
      Result(const Vt &e) : m(e) {}

      /// from Ok_t<V> rvalue
      Result(Vt &&e) : m(std::move(e)) {}

      /// from Error_t<E> lvalue
      Result(const Et &e) : m(e) {}

      /// from Error_t<E> rvalue
      Result(Et &&e) : m(std::move(e)) {}

      /// from Result lvalue
      Result(const R &other) : m(other.m) {}

      /// from Result rvalue
      Result(R &&other) noexcept : m(std::move(other.m)) {}

      /// default destructor
      ~Result() = default;

      /**
       * @brief Operator bool. True if and only if Result contains Value type.
       *
       * @code
       * Result<int, double> f = 1;
       * if(f) {
       *   // f contains value
       * } else {
       *   // f contains error
       * }
       * @nocode
       *
       * @return
       */
      explicit operator bool() const noexcept { return m.which() == 0; }

      /**
       * Negation operator. True if and only if Result contains Error type.
       * @return
       */
      bool operator!() const noexcept { return m.which() == 1; }

      /**
       * @brief Dereference operator.
       *
       * @code
       * Result<int, double> f = 1;
       * int value = *f;  // one way to get value.
       * @nocode
       *
       * Semantics is the following: this operator is one of ways to get Value.
       * If Result contains Error, it throws Result::bad_get.
       *
       * @throws Result::bad_get if Result contains Error
       * @return
       */
      V &&operator*() && { return std::move(this->get_value()); }

      /**
       * @brief Dereference operator.
       * @throws Result::bad_get if Result contains Error
       * @return
       */
      V &operator*() & { return this->get_value(); }

      /**
       * @brief Dereference operator.
       * @throws Result::bad_get if Result contains Error
       * @return
       */
      V const &operator*() const & { return this->get_value(); }

      /**
       * @brief Arrow operator.
       *
       * @code
       * Result<std::string, double> f = "hello"s;
       * f->size();  // one more way to get access to the value.
       * @nocode
       *
       * Semantics is the following: this operator is one of ways to get Value.
       * If Result contains Error, it throws Result::bad_get
       *
       * @throws Result::bad_get if Result contains Error
       * @return
       */
      V *operator->() { return &this->get_value(); }
      // arrow operator
      const V *operator->() const { return &this->get_value(); }

      /**
       * @brief Accessor for the Error object.
       * @throws Result::bad_get if Result contains Value.
       * @return E
       */
      E error() { return this->get_error(); }

      /**
       * @brief Accessor for the Value object.
       * @throws Result::bad_get if Result contains Error.
       * @return V
       */
      V ok() { return this->get_value(); }

      /**
       * @brief Accessor for the Error object.
       * @throws Result::bad_get if Result contains Value.
       * @return const E&
       */
      const E &error() const { return this->get_error(); }

      /**
       * @brief Accessor for the Value object.
       * @throws Result::bad_get if Result contains Error.
       * @return const V&
       */
      const V &ok() const { return this->get_value(); }

      /// from Result lvalue
      Result &operator=(R const &rhs) {
        m = rhs;
        return *this;
      }

      /// from Result rvalue
      Result &operator=(R &&rhs) noexcept {
        m = std::move(rhs);
        return *this;
      }

      /// from Value (both lvalue and rvalue)
      Result &operator=(V &&val) {
        m = Ok(std::forward<V>(val));
        return *this;
      }

      /// from Ok_t<V> lvalue
      Result &operator=(Vt const &val) {
        m = val;
        return *this;
      }

      /// from Ok_t<V> rvalue
      Result &operator=(Vt &&val) {
        m = std::move(val);
        return *this;
      }

      /// from Error_t<E> lvalue
      Result &operator=(Et const &val) {
        m = val;
        return *this;
      }

      /// from Error_t<E> rvalue
      Result &operator=(Et &&val) {
        m = std::move(val);
        return *this;
      }

      /**
       * @brief Used for pattern matching. Fast, type-safe, exception-safe sway
       * to get value from Result.
       * @tparam Fs types of visitor functions
       * @param fs visitor functions
       * @return arbitrary value that visitor returns.
       */
      template <typename... Fs>
      constexpr auto match(Fs &&... fs) {
        return boost::apply_visitor(make_visitor(std::forward<Fs>(fs)...), m);
      }

      /**
       * @brief Used for pattern matching. Fast, type-safe, exception-safe way
       * to get value from Result.
       * @tparam Visitor visitor object, which has overloaded operator() for
       * Ok_t<V> const& and Error_t<E> const&
       * @param v visitor instance.
       * @return arbitrary value that visitor returns.
       */
      template <typename Visitor>
      constexpr auto match(Visitor v) {
        return boost::apply_visitor(v, m);
      }

     private:
      V &get_value() { return boost::get<Vt>(m).value; }
      E &get_error() { return boost::get<Et>(m).reason; }

      const V &get_value() const { return std::cref(boost::get<Vt>(m).value); }

      const E &get_error() const { return std::cref(boost::get<Et>(m).reason); }

      boost::variant<Vt, Et> m;
    };
  }

}  // namespace iroha

#endif  // IROHA_COMMON_RESULT_HPP_
