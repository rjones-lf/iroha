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

#ifndef IROHA_VISITOR_APPLY_FOR_ALL_HPP
#define IROHA_VISITOR_APPLY_FOR_ALL_HPP

#include <boost/variant/static_visitor.hpp>
#include <type_traits>

namespace shared_model {
  namespace detail {

    /**
     * Class provides generic toString visitor for objects
     */
    class ToStringVisitor : public boost::static_visitor<std::string> {
     public:
      template <typename InputType>
      auto operator()(const InputType &operand) const
          -> decltype(operand.toString(), std::string()) {
        return operand.toString();
      }

      template <typename InputType>
      auto operator()(const InputType &operand) const
          -> decltype(operand->toString(), std::string()) {
        return operand->toString();
      }
    };


  }  // namespace detail
}  // namespace shared_model
#endif  // IROHA_VISITOR_APPLY_FOR_ALL_HPP
