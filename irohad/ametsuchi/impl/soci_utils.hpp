/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_POSTGRES_WSV_COMMON_HPP
#define IROHA_POSTGRES_WSV_COMMON_HPP

#include <soci/soci.h>
#include <boost/optional.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/tuple/tuple.hpp>
#include "common/types.hpp"

namespace iroha {
  namespace ametsuchi {

    template <typename ParamType, typename Function>
    inline void processSoci(soci::statement &st,
                            soci::indicator &ind,
                            ParamType &row,
                            Function f) {
      while (st.fetch()) {
        switch (ind) {
          case soci::i_ok:
            f(row);
          case soci::i_null:
          case soci::i_truncated:
            break;
        }
      }
    }

    /// tuple length shortcut
    template <typename T>
    constexpr std::size_t length_v = boost::tuples::length<T>::value;

    /// index sequence helper for index_apply
    template <typename F, std::size_t... Is>
    constexpr decltype(auto) index_apply_impl(F &&f,
                                              std::index_sequence<Is...>) {
      return std::forward<F>(f)(std::integral_constant<std::size_t, Is>{}...);
    }

    /// apply F to an integer sequence [0, N)
    template <size_t N, typename F>
    constexpr decltype(auto) index_apply(F &&f) {
      return index_apply_impl(std::forward<F>(f),
                              std::make_index_sequence<N>{});
    }

    /// apply F to Tuple
    template <typename Tuple, typename F>
    constexpr decltype(auto) apply(Tuple &&t, F &&f) {
      return index_apply<length_v<std::decay_t<Tuple>>>(
          [&](auto... Is) -> decltype(auto) {
            return std::forward<F>(f)(
                boost::get<Is>(std::forward<Tuple>(t))...);
          });
    }

    template <typename C, typename T, typename F>
    auto mapValues(T &t, F &&f) {
      return t | [&](auto &st) -> boost::optional<C> {
        return boost::copy_range<C>(
            st | boost::adaptors::transformed([&](auto &t) {
              return apply(t, std::forward<F>(f));
            }));
      };
    }

    template <typename C, typename T, typename F>
    auto flatMapValues(T &t, F &&f) {
      return t | [&](auto &st) -> boost::optional<C> {
        return boost::copy_range<C>(
            st | boost::adaptors::transformed([&](auto &t) {
              return apply(t, std::forward<F>(f));
            })
            | boost::adaptors::filtered(
                  [](auto &r) { return static_cast<bool>(r); })
            | boost::adaptors::transformed([](auto &&r) { return *r; }));
      };
    }

    template <typename T, typename F>
    auto flatMapValue(T &t, F &&f) {
      return t | [&](auto &st) -> decltype(
                                   apply(boost::make_iterator_range(st).front(),
                                         std::forward<F>(f))) {
        auto range = boost::make_iterator_range(st);

        if (range.empty()) {
          return boost::none;
        }

        return apply(range.front(), std::forward<F>(f));
      };
    }

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_WSV_COMMON_HPP
