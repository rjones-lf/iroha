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

#ifndef IROHA_ANSWER_HPP
#define IROHA_ANSWER_HPP

#include <boost/range/numeric.hpp>
#include <map>
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace validation {

    using ConcreteReasonType = std::string;
    using GroupedReasons = std::vector<ConcreteReasonType>;
    using ReasonsGroupName = std::string;
    using ReasonsGroupType = std::pair<ReasonsGroupName, GroupedReasons>;

    /**
     * Class which represents the answer to stateless validation: whether
     * validation is done right and if not it explains the reason
     */
    class Answer {
     public:
      operator bool() const { return not reasons_map_.empty(); }

      /**
       * @return string representation of errors
       */
      std::string reason() const {
        return boost::accumulate(
            reasons_map_,
            std::string{},
            [](auto &&acc, const auto &command_reasons) {
              acc += detail::PrettyStringBuilder()
                         .init(command_reasons.first)
                         .appendAll(command_reasons.second,
                                    [](auto &element) { return element; })
                         .finalize() + "\n";
              return std::forward<decltype(acc)>(acc);
            });
      }

      /**
       * Check if any error has been recorded to the answer
       * @return true if there are any errors, false otherwise
       */
      bool hasErrors() { return not reasons_map_.empty(); }

      /**
       * Adds error to map
       * @param reasons
       */
      void addReason(ReasonsGroupType &&reasons) {
        reasons_map_.insert(std::move(reasons));
      }

      std::map<ReasonsGroupName, GroupedReasons> getReasonsMap(){
        return reasons_map_;
      };

     private:
      std::map<ReasonsGroupName, GroupedReasons> reasons_map_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_ANSWER_HPP
