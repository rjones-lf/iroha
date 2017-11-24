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

#include "model/converters/json_common.hpp"

using namespace rapidjson;

namespace iroha {
  namespace model {
    namespace converters {
      Value serializeSignature(const Signature& signature,
                               Document::AllocatorType& allocator) {
        Value document;
        document.SetObject();

        document.AddMember("pubkey", signature.pubkey.to_hexstring(),
                           allocator);
        document.AddMember("signature", signature.signature.to_hexstring(),
                           allocator);

        return document;
      }

      Value serializePager(const Pager& pager,
                           Document::AllocatorType& allocator) {
        Value document;
        document.SetObject();

        document.AddMember("tx_hash", pager.tx_hash.to_hexstring(), allocator);
        document.AddMember("limit", pager.limit, allocator);

        return document;
      }

      nonstd::optional<Document> stringToJson(const std::string& string) {
        Document document;
        document.Parse(string);
        if (document.HasParseError()) {
          return nonstd::nullopt;
        }
        return nonstd::make_optional(std::move(document));
      }

      std::string jsonToString(const Document& document) {
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        document.Accept(writer);
        return sb.GetString();
      }
    }  // namespace converters
  }    // namespace model
}  // namespace iroha
