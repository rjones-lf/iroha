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

#ifndef IROHA_BYTEUTILS_H
#define IROHA_BYTEUTILS_H

#include <algorithm>
#include <boost/optional.hpp>
#include <string>

#include "common/types.hpp"
namespace iroha {

  /**
   * Create blob_t from string of specified size
   * @tparam size - size of blob_t, expected size of string
   * @param s - string to convert
   * @return blob, if conversion was successful, otherwise nullopt
   */
  template <size_t size>
  boost::optional<blob_t<size>> stringToBlob(const std::string &string) {
    if (size != string.size()) {
      return boost::none;
    }
    blob_t<size> array;
    std::copy(string.begin(), string.end(), array.begin());
    return array;
  }

  /**
   * Convert string of raw bytes to printable hex string
   * @param str - raw bytes string to convert
   * @return - converted hex string
   */
  inline std::string bytestringToHexstring(const std::string &str) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto &c : str) {
      ss << std::setw(2) << (static_cast<int>(c) & 0xff);
    }
    return ss.str();
  }

  /**
   * Convert printable hex string to string of raw bytes
   * @param str - hex string to convert
   * @return - raw bytes converted string or boost::noneif provided string
   * was not a correct hex string
   */
  inline boost::optional<std::string> hexstringToBytestring(
      const std::string &str) {
    if (str.empty() or str.size() % 2 != 0) {
      return boost::none;
    }
    std::string result(str.size() / 2, 0);
    for (size_t i = 0; i < result.length(); ++i) {
      std::string byte = str.substr(i * 2, 2);
      try {
        result.at(i) = std::stoul(byte, nullptr, 16);
      } catch (const std::invalid_argument &e) {
        return boost::none;
      } catch (const std::out_of_range &e) {
        return boost::none;
      }
    }
    return result;
  }

  /**
   * Convert hexstring to array of given size
   * @tparam size - output array size
   * @param string - input string for transform
   * @return array of given size if size matches, nullopt otherwise
   */
  template <size_t size>
  boost::optional<blob_t<size>> hexstringToArray(const std::string &string) {
    return hexstringToBytestring(string) | stringToBlob<size>;
  }

}  // namespace iroha

#endif  // IROHA_BYTEUTILS_H
