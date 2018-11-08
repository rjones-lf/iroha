/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/common_constants.hpp"

#include "cryptography/crypto_provider/crypto_defaults.hpp"

using namespace shared_model::crypto;

namespace common_constants {

  // user names
  const std::string kAdminName = "admin";
  const std::string kUser = "user";
  const std::string kSpectator = "spectator";

  // role names
  const std::string kAdminRole = "admin_role";
  const std::string kDefaultRole = "default_role";
  const std::string kRole = "user_role";

  // asset names
  const std::string kAssetName = "coin";

  // domain names
  const std::string kDomain = "domain";
  const std::string kSecondDomain = "domain2";

  // ids
  const std::string kAdminId = kAdminName + "@" + kDomain;
  const std::string kUserId = kUser + "@" + kDomain;
  const std::string kCloseSpectatorId = kSpectator + "@" + kDomain;
  const std::string kRemoteSpectatorId = kSpectator + "@" + kSecondDomain;
  const std::string kAssetId = kAssetName + "#" + kDomain;

  // keypairs
  const Keypair kAdminKeypair = DefaultCryptoAlgorithmType::generateKeypair();
  const Keypair kUserKeypair = DefaultCryptoAlgorithmType::generateKeypair();
  const Keypair kCloseSpectatorKeypair = DefaultCryptoAlgorithmType::generateKeypair();
  const Keypair kRemoteSpectatorKeypair = DefaultCryptoAlgorithmType::generateKeypair();
}
