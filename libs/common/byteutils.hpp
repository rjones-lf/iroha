/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BYTEUTILS_H
#define IROHA_BYTEUTILS_H

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <boost/optional.hpp>

#include "common/bind.hpp"
#include "common/blob.hpp"

namespace iroha {
  /**
   * Convert string to blob vector
   * @param source - string for conversion
   * @return vector<blob>
   */
  inline std::vector<uint8_t> stringToBytes(const std::string &source) {
    return std::vector<uint8_t>(source.begin(), source.end());
  }

  /**
   * blob vector to string
   * @param source - vector for conversion
   * @return result string
   */
  inline std::string bytesToString(const std::vector<uint8_t> &source) {
    return std::string(source.begin(), source.end());
  }

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
        result.at(i) =
            static_cast<std::string::value_type>(std::stoul(byte, nullptr, 16));
      } catch (const std::invalid_argument &) {
        return boost::none;
      } catch (const std::out_of_range &) {
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
