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

#ifndef IROHA_SHARED_MODEL_STRING_BUILDER_HPP
#define IROHA_SHARED_MODEL_STRING_BUILDER_HPP

#include <algorithm>
#include <string>

namespace shared_model {
  namespace detail {
    /**
     * A simple string builder class for building pretty looking strings
     */
    class PrettyStringBuilder {
     public:
      /**
       * Initializes new string with a provided name
       * @param name - name to initialize
       */
      PrettyStringBuilder &init(const std::string &name) {
        result_.append(name);
        result_.append(initSeparator);
        result_.append(spaceSeparator);
        result_.append(beginBlockMarker);
        return *this;
      }

      /**
       * Inserts new level marker
       */
      PrettyStringBuilder &insertLevel() {
        result_.append(beginBlockMarker);
        return *this;
      }

      /**
       * Closes new level marker
       */
      PrettyStringBuilder &removeLevel() {
        result_.append(endBlockMarker);
        return *this;
      }

      /**
       * Appends new field to string as a "name=value" pair
       * @param name - field name to append
       * @param value - field value
       */
      PrettyStringBuilder &append(const std::string &name,
                                  const std::string &value) {
        result_.append(name);
        result_.append(keyValueSeparator);
        result_.append(value);
        result_.append(singleFieldsSeparator);
        result_.append(spaceSeparator);
        return *this;
      }

      /**
       * Appends new single value to string
       * @param value - value to append
       */
      PrettyStringBuilder &append(const std::string &value) {
        result_.append(value);
        result_.append(spaceSeparator);
        return *this;
      }

      /**
       * Appends a new collection to string
       * @tparam Collection - type of collection
       * @tparam Transform - type of transformation function
       * @param c - collection to append
       * @param t - transformation function
       */
      template <typename Collection, typename Transform>
      PrettyStringBuilder &appendAll(Collection &&c, Transform &&t) {
        insertLevel();
        std::for_each(c.begin(), c.end(), [this, &t](auto &val) {
          this->append(t(val));
        });
        removeLevel();
        return *this;
      }

      /**
       * Finalizes appending and returns constructed string.
       * @return resulted string
       */
      std::string finalize() {
        result_.append(endBlockMarker);
        return result_;
      }

     private:
      std::string result_;
      const std::string beginBlockMarker = "[";
      const std::string endBlockMarker = "]";
      const std::string keyValueSeparator = "=";
      const std::string singleFieldsSeparator = ",";
      const std::string initSeparator = ":";
      const std::string spaceSeparator = " ";
    };
  }
}

#endif  // IROHA_SHARED_MODEL_STRING_BUILDER_HPP
