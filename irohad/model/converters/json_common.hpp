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

#ifndef IROHA_JSON_COMMON_HPP
#define IROHA_JSON_COMMON_HPP

#include <numeric>
#include <string>
#include <unordered_map>

// Enable std::string support in rapidjson
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <nonstd/optional.hpp>

#include "common/byteutils.hpp"
#include "model/block.hpp"
#include "model/common.hpp"
#include "model/signature.hpp"
#include "model/queries/pager.hpp"

namespace iroha {
  namespace model {
    namespace converters {
      /**
       * Convert functor which specifies output type
       * @tparam V - output type
       */
      template <typename V>
      struct Convert {
        /**
         * Convert input type to defined type
         * @tparam T - input type
         * @param x - input value
         * @return optional of output type from input value
         */
        template <typename T>
        auto operator()(T &&x) {
          return nonstd::optional<V>(x);
        }
      };

      template <size_t size>
      struct Convert<blob_t<size>> {
        template <typename T>
        auto operator()(T &&x) {
          return hexstringToArray<size>(x);
        }
      };

      /**
       * Deserialize field from given document with given type
       * @tparam T - getter return type
       * @tparam D - document type
       * @param document - document value for deserialization
       * @param field - field name for deserialization
       * @return deserialized field on success, nullopt otherwise
       */
      template <typename T, typename D>
      nonstd::optional<T> deserializeField(const D &document,
                                           const std::string &field) {
        if (document.HasMember(field.c_str())
            and document[field.c_str()].template Is<T>()) {
          return document[field.c_str()].template Get<T>();
        }
        return nonstd::nullopt;
      }

      /**
       * Functor for deserialization from given document
       * @tparam D - document type
       */
      template <typename D>
      struct FieldDeserializer {
        /**
         * @param document - document for field deserialization
         */
        explicit FieldDeserializer(const D &document) : document(document) {}

        /**
         * Create function, which deserializes document field,
         * transforms the value to required type, and
         * assigns it to block member
         * @tparam T - getter return type
         * @tparam V - block member type
         * @tparam B - block type
         * @tparam Convert - transform function type
         * @param member - pointer to member in block
         * @param field - field name for deserialization
         * @param transform - transform function from T to V
         * @return function, which takes block, returns block with deserialized
         * member on success, nullopt otherwise
         */
        template <typename T, typename V, typename B,
                  typename Convert = Convert<V>>
        auto deserialize(V B::*member, const std::string &field,
                         Convert transform = Convert()) {
          return [this, member, field, transform](auto block) {
            return deserializeField<T>(document, field) | transform
                | assignObjectField(block, member);
          };
        }

        /**
         * Deserialize field of Uint type to given member field of block
         * @tparam V - block member type
         * @tparam B - block type
         * @param member - pointer to member in block
         * @param field - field name for deserialization
         * @return @see deserialize
         */
        template <typename V, typename B>
        auto Uint(V B::*member, const std::string &field) {
          return deserialize<uint32_t>(member, field);
        }

        /**
         * Deserialize field of Uint64 type to given member field of block
         * @tparam V - block member type
         * @tparam B - block type
         * @param member - pointer to member in block
         * @param field - field name for deserialization
         * @return @see deserialize
         */
        template <typename V, typename B>
        auto Uint64(V B::*member, const std::string &field) {
          return deserialize<uint64_t>(member, field);
        }

        /**
         * Deserialize field of Bool type to given member field of block
         * @tparam V - block member type
         * @tparam B - block type
         * @param member - pointer to member in block
         * @param field - field name for deserialization
         * @return @see deserialize
         */
        template <typename V, typename B>
        auto Bool(V B::*member, const std::string &field) {
          return deserialize<bool>(member, field);
        }

        /**
         * Deserialize field of String type to given member field of block
         * @tparam V - block member type
         * @tparam B - block type
         * @param member - pointer to member in block
         * @param field - field name for deserialization
         * @return @see deserialize
         */
        template <typename V, typename B>
        auto String(V B::*member, const std::string &field) {
          return deserialize<std::string>(member, field);
        }

        /**
         * Deserialize field of String type
         * @param field - field name for deserialization
         * @return deserialized field on success, nullopt otherwise
         */
        auto String(const std::string &field) {
          return deserializeField<std::string>(document, field);
        }

        /**
         * Deserialize field of Array type to given member field of block,
         * using provided transform function
         * @tparam V - block member type
         * @tparam B - block type
         * @tparam Convert - transform function type
         * @param member - pointer to member in block
         * @param field - field name for deserialization
         * @param transform - transform function from Array to V
         * @return @see deserialize
         */
        template <typename V, typename B, typename Convert = Convert<V>>
        auto Array(V B::*member, const std::string &field,
                   Convert transform = Convert()) {
          return deserialize<rapidjson::Value::ConstArray>(member, field,
                                                           transform);
        }

        /**
         * Deserialize field of Object type to given member field of block
         * @tparam V - block member type
         * @tparam B - block type
         * @param member - pointer to member in block
         * @param field - field name for deserialization
         * @return @see deserialize
         */
        template <typename V, typename B, typename Convert = Convert<V>>
        auto Object(V B::*member, const std::string &field,
                    Convert transform = Convert()) {
          return deserialize<rapidjson::Value::ConstObject>(member, field,
                                                            transform);
        }

        // document for deserialization
        const D &document;
      };

      /**
       * Factory method for FieldDeserializer functor
       * @tparam D - document type
       * @param document - document for deserialization
       * @return FieldDeserializer instance for given arguments
       */
      template <typename D>
      auto makeFieldDeserializer(const D &document) {
        return FieldDeserializer<D>(document);
      }

      template <>
      struct Convert<Pager> {
        template <typename T>
        auto operator()(T &&x) {
          auto des = makeFieldDeserializer(x);
          return nonstd::make_optional<Pager>()
                 | des.String(&Pager::tx_hash, "tx_hash")
                 | des.Uint(&Pager::limit, "limit");
        }
      };

      template <>
      struct Convert<Signature> {
        template <typename T>
        auto operator()(T &&x) {
          auto des = makeFieldDeserializer(x);
          return nonstd::make_optional<Signature>()
              | des.String(&Signature::pubkey, "pubkey")
              | des.String(&Signature::signature, "signature");
        }
      };

      template <>
      struct Convert<Block::SignaturesType> {
        template <typename T>
        auto operator()(T &&x) {
          auto acc_signatures = [](auto init, auto &x) {
            return init | [&x](auto signatures) {
              return Convert<Signature>()(x) | [&signatures](auto signature) {
                signatures.push_back(signature);
                return nonstd::make_optional(signatures);
              };
            };
          };
          return std::accumulate(x.begin(), x.end(),
                                 nonstd::make_optional<Block::SignaturesType>(),
                                 acc_signatures);
        }
      };

      /**
       * Serialize pager to JSON with given allocator
       * @param pager - pager for restricting to get transactions
       * @param allocator - allocator for JSON value
       * @return JSON value with pager
       */
      rapidjson::Value serializePager(
        const Pager &pager, rapidjson::Document::AllocatorType &allocator);

      /**
       * Serialize signature to JSON with given allocator
       * @param signature - signature for serialization
       * @param allocator - allocator for JSON value
       * @return JSON value with signature
       */
      rapidjson::Value serializeSignature(
          const Signature &signature,
          rapidjson::Document::AllocatorType &allocator);

      /**
       * Try to parse JSON from string
       * @param string - string for parsing
       * @return JSON document on success, nullopt otherwise
       */
      nonstd::optional<rapidjson::Document> stringToJson(
          const std::string &string);

      /**
       * Pretty print JSON document to string
       * @param document - document for printing
       * @return pretty printed JSON document
       */
      std::string jsonToString(const rapidjson::Document &document);
    }  // namespace converters
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_JSON_COMMON_HPP
