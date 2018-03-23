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

#include <string>
#include <vector>

#include "interfaces/base/model_primitive.hpp"
#include "utils/lazy_initializer.hpp"
#include "utils/swig_keyword_hider.hpp"

namespace shared_model {
  namespace crypto {

    class Blob;
    std::string toBinaryString(const Blob &b);

    /**
     * Blob class present user-friendly blob for working with low-level
     * binary stuff. Its length is not fixed in compile time.
     */
    class Blob : public interface::ModelPrimitive<Blob> {
     public:
      using Bytes = std::vector<uint8_t>;

      Blob() = default;
      /**
       * Create blob from a string
       * @param blob - string to create blob from
       */
      explicit Blob(const std::string &blob);

      /**
       * Create blob from a vector
       * @param blob - vector to create blob from
       */
      explicit Blob(const Bytes &blob);

      explicit Blob(Bytes &&blob) noexcept;

      /**
       * Creates new Blob object from provided hex string
       * @param hex - string in hex format to create Blob from
       * @return Blob from provided hex string if it was correct or
       * Blob from empty string if provided string was not a correct hex string
       */
      static Blob fromHexString(const std::string &hex);

      /**
       * @return provides raw representation of blob
       */
      virtual const Bytes &blob() const;

      /**
       * @return provides human-readable representation of blob without leading
       * 0x
       */
      virtual const std::string &hex() const;

      /**
       * @return size of raw representation of blob
       */
      virtual size_t size() const;

      std::string toString() const override;

      bool operator==(const Blob &rhs) const override;

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

     protected:
      Blob *clone() const override;

     private:
      // TODO: 17/11/2017 luckychess use improved Lazy with references support
      Bytes blob_;
      std::string hex_;
    };


  }  // namespace crypto
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_BLOB_HPP
