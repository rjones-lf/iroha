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

#ifndef IROHA_SHARED_MODEL_TYPES_HPP
#define IROHA_SHARED_MODEL_TYPES_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "common/types.hpp"
#include "cryptography/public_key.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "utils/polymorphic_wrapper.hpp"

namespace shared_model {

  namespace interface {
    namespace types {
      /// Type of account id
      using AccountIdType = std::string;
      /// Type of precision
      using PrecisionType = uint8_t;
      /// Type of height (for Block, Proposal etc)
      using HeightType = uint64_t;
      /// Type of public key
      using PubkeyType = crypto::PublicKey;
      /// Type of public keys' collection
      using PublicKeyCollectionType =
          std::vector<detail::PolymorphicWrapper<PubkeyType>>;
      /// Type of role (i.e admin, user)
      using RoleIdType = std::string;
      /// Iroha domain id type
      using DomainIdType = std::string;
      /// Type of asset id
      using AssetIdType = std::string;
      /// Type of asset id collection
      using AssetIdCollectionType = std::vector<AssetIdType>;
      /// Permission type used in permission commands
      using PermissionNameType = std::string;
      /// Type of Quorum used in transaction and set quorum
      using QuorumType = uint32_t;
      /// Type of transaction signature
      using SignatureType = detail::PolymorphicWrapper<Signature>;
      /// Type of timestamp
      using TimestampType = uint64_t;
      /// Type of peer address
      using AddressType = std::string;
      /// Type of counter
      using CounterType = uint64_t;
      /// Type of account name
      using AccountNameType = std::string;
      /// Type of asset name
      using AssetNameType = std::string;
      /// Type of asset name
      using DetailType = std::string;
      /// Type of JSON data
      using JsonType = std::string;
    }  // namespace types
  }    // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_TYPES_HPP
