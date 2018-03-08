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

#ifndef IROHA_SHARED_MODEL_BLOB_HPP
#define IROHA_SHARED_MODEL_BLOB_HPP

#include <iomanip>
#include <sstream>
#include <vector>
#include "common/byteutils.hpp"
#include "interfaces/base/model_primitive.hpp"
#include "utils/lazy_initializer.hpp"
#include "utils/string_builder.hpp"
#include "utils/swig_keyword_hider.hpp"

namespace shared_model {
  namespace crypto {

    class Blob;
    static inline std::string toBinaryString(const Blob &b);
    /**
     * Blob class present user-friendly blob for working with low-level
     * binary stuff. Its length is not fixed in compile time.
     */
    class Blob : public interface::ModelPrimitive<Blob> {
     public:
      using Bytes = std::vector<uint8_t>;

      Blob() : blob_() {}
      /**
       * Create blob from a string
       * @param blob - string to create blob from
       */
      explicit Blob(const std::string &blob)
          : Blob(Bytes(blob.begin(), blob.end())) {}

      /**
       * Create blob from a vector
       * @param blob - vector to create blob from
       */
      explicit Blob(const Bytes &blob) : Blob(Bytes(blob)) {}
      explicit Blob(Bytes &&blob) : blob_(std::move(blob)) {
        hex_ = iroha::bytestringToHexstring(toBinaryString(*this));
      }

      /**
       * Creates new Blob object from provided hex string
       * @param hex - string in hex format to create Blob from
       * @return Blob from provided hex string if it was correct or
       * Blob from empty string if provided string was not a correct hex string
       */
      static Blob fromHexString(const std::string &hex) {
        using iroha::operator|;
        Blob b("");
        iroha::hexstringToBytestring(hex) | [&](auto &&s) { b = Blob(s); };
        return b;
      }

      /**
       * @return provides raw representation of blob
       */
      virtual const Bytes &blob() const {
        return blob_;
      }

      /**
       * @return provides human-readable representation of blob without leading
       * 0x
       */
      virtual const std::string &hex() const {
        return hex_;
      }

      /**
       * @return size of raw representation of blob
       */
      virtual size_t size() const {
        return blob_.size();
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Blob")
            .append(hex())
            .finalize();
      }

      bool operator==(const Blob &rhs) const override {
        return blob() == rhs.blob();
      }

      Blob *copy() const override {
        return new Blob(blob());
      };

#ifndef DISABLE_BACKWARD
      /**
       * Method perform transforming object to old-fashion blob_t format
       * @tparam BlobType - type of blob
       * @return blob_t array with own data
       * Design note: this method is deprecated and should be removed after
       * migration to shared model in whole project
       */

      template <typename BlobType>
      DEPRECATED BlobType makeOldModel() const {
        return BlobType::from_string(toBinaryString(*this));
      }
#endif

     private:
      // TODO: 17/11/2017 luckychess use improved Lazy with references support
      Bytes blob_;
      std::string hex_;
    };

    static inline std::string   toBinaryString(const Blob &b) {
      return std::string(b.blob().begin(), b.blob().end());
    }
  }  // namespace crypto
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_BLOB_HPP
