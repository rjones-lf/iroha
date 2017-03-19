/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <assert.h>
#include <iostream>
#include <map>
#include <vector>

#include <infra/protobuf/api.pb.h>
#include <infra/virtual_machine/jvm/java_data_structure.hpp>
#include <repository/domain/account_repository.hpp>
#include <repository/domain/asset_repository.hpp>
#include <repository/domain/domain_repository.hpp>
#include <repository/domain/simple_asset_repository.hpp>
#include <repository/domain/peer_repository.hpp>
//#include <util/convert_string.hpp>
#include <util/logger.hpp>

#include "jni_constants.hpp"
#include "repository_Repository.h"

const std::string NameSpaceID = "domain repo jni";

using virtual_machine::jvm::JavaMakeBoolean;
using virtual_machine::jvm::JavaMakeMap;
using virtual_machine::jvm::convertJavaStringArrayRelease;
using virtual_machine::jvm::convertJavaHashMapValueString;
using virtual_machine::jvm::convertJavaHashMapValueHashMap;
using virtual_machine::jvm::convertAssetValueHashMap;
using virtual_machine::jvm::convertSimpleAssetValueHashMap;
using virtual_machine::jvm::convertBaseObjectToMapString;
using virtual_machine::jvm::convertMapStringToTrust;
using virtual_machine::jvm::convertTrustToMapString;
using virtual_machine::jvm::JavaMakeStringArray;

namespace tag = jni_constants;

/***************************************************************************************
 * Account
 ***************************************************************************************/

JNIEXPORT jstring JNICALL Java_repository_Repository_accountAdd
    (JNIEnv *env, jclass, jobject mMap_, jobjectArray assets_) {

  const auto mMap = convertJavaHashMapValueString(env, mMap_);
  const std::vector<std::string> assets = convertJavaStringArrayRelease(env, assets_);

  IROHA_ASSERT_FALSE(mMap.find(tag::PublicKey) == mMap.end());
  const std::string publicKey = mMap.find(tag::PublicKey)->second;

  IROHA_ASSERT_FALSE(mMap.find(tag::AccountName) == mMap.end());
  const std::string name = mMap.find(tag::AccountName)->second;

  logger::debug(NameSpaceID + "::accountAdd")
      << "pubkey: " << publicKey << " name: " << name;
//      << " assets: " << convert_string::to_string(assets);

  Api::Account account;

  for (auto asset: assets)
    account.add_assets(asset);

  account.set_name(name);
  account.set_publickey(publicKey);

  const auto ret = repository::account::add(publicKey, account);
  return env->NewStringUTF("OK");
}

// accountAttach is deprecated
JNIEXPORT jobject JNICALL Java_repository_Repository_accountAttach
    (JNIEnv *env, jclass, jobject mMap_) {

  const auto mMap = convertJavaHashMapValueString(env, mMap_);

  IROHA_ASSERT_FALSE(mMap.find(tag::Uuid) == mMap.end());
  const auto uuid = mMap.find(tag::Uuid)->second;

  IROHA_ASSERT_FALSE(mMap.find(tag::AttachedAssetUuid) == mMap.end());
  const auto assetUuid = mMap.find(tag::AttachedAssetUuid)->second;

  logger::debug(NameSpaceID + "::accountAttach") << "uuid: " << uuid << ", assetUuid: " << assetUuid;

  return JavaMakeBoolean(env, false);
}


// accountAttach is deprecated
JNIEXPORT jobject JNICALL Java_repository_Repository_accountUpdate
    (JNIEnv *env, jclass, jobject mMap_, jobjectArray assets_) {

  const auto mMap = convertJavaHashMapValueString(env, mMap_);
  const auto assets = convertJavaStringArrayRelease(env, assets_);

  IROHA_ASSERT_FALSE(mMap.find(tag::PublicKey) == mMap.end());
  const auto publicKey = mMap.find(tag::PublicKey)->second;

  IROHA_ASSERT_FALSE(mMap.find(tag::AccountName) == mMap.end());
  const auto name = mMap.find(tag::AccountName)->second;

  logger::debug(NameSpaceID + "::accountUpdate")
      << " publicKey: " << publicKey << ", name: " << name;
//      << ", assets: " << convert_string::to_string(assets);

  Api::Account account;

  for (auto asset: assets)
    account.add_assets(asset);

  account.set_name(name);
  account.set_publickey(publicKey);

  return JavaMakeBoolean(env, (jboolean) repository::account::update(publicKey, account));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_accountRemove
    (JNIEnv *env, jclass, jobject mMap_) {
  const auto mMap = convertJavaHashMapValueString(env, mMap_);

  IROHA_ASSERT_FALSE(mMap.find(tag::Uuid) == mMap.end());
  const auto publicKey = mMap.find(tag::PublicKey)->second;

  logger::debug(NameSpaceID + "::accountRemove") << " publicKey: " << publicKey;
  return JavaMakeBoolean(env, repository::account::remove(publicKey));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_accountInfoFindByUuid
    (JNIEnv *env, jclass, jobject mMap_) {
  const auto mMap = convertJavaHashMapValueString(env, mMap_);

  IROHA_ASSERT_FALSE(mMap.find(tag::PublicKey) == mMap.end());
  const auto publicKey = mMap.find(tag::PublicKey)->second;

  Api::Account account = repository::account::find(publicKey);

  std::map<std::string, std::string> params;
  {
    params[tag::PublicKey] = account.publickey();
    params[tag::AccountName] = account.name();
  }

  logger::debug(NameSpaceID + "::accountInfoFindByUuid")
      << "params[tag::PublicKey]: " << params[tag::PublicKey] << ", "
      << "params[tag::AccountName]: " << params[tag::AccountName];

  return JavaMakeMap(env, params);
}

JNIEXPORT jobjectArray JNICALL Java_repository_Repository_accountValueFindByUuid
    (JNIEnv *env, jclass, jobject mMap_) {
  const auto mMap = convertJavaHashMapValueString(env, mMap_);

  IROHA_ASSERT_FALSE(mMap.find(tag::Uuid) == mMap.end());
  const auto publicKey = mMap.find(tag::PublicKey)->second;

  Api::Account account = repository::account::find(publicKey);

  const auto assets = txbuilder::createStandardVector(account.assets());

  logger::debug(NameSpaceID + "::accountValueFindByUuid");
//      << "value: " << convert_string::to_string(assets);

  return JavaMakeStringArray(env, assets);
}

JNIEXPORT jobject JNICALL Java_repository_Repository_accountExists
    (JNIEnv *env, jclass, jobject mMap_) {
  const auto mMap = convertJavaHashMapValueString(env, mMap_);

  IROHA_ASSERT_FALSE(mMap.find(tag::Uuid) == mMap.end());
  const auto publicKey = mMap.find(tag::PublicKey)->second;

  logger::debug(NameSpaceID + "::accountExists") << " uuid: " << publicKey;
  return JavaMakeBoolean(env, repository::account::exists(publicKey));
}

/***************************************************************************************
 * Domain
 ***************************************************************************************/
JNIEXPORT jstring JNICALL Java_repository_Repository_domainAdd
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::OwnerPublicKey) == params.end());
  const auto ownerPublicKey = params.find(tag::OwnerPublicKey)->second;

  IROHA_ASSERT_FALSE(params.find(tag::DomainName) == params.end());
  const auto name = params.find(tag::DomainName)->second;

  logger::debug(NameSpaceID + "::domainAdd")
      << "ownerPublicKey: " << ownerPublicKey << ", name: " << name;

  const auto ret = repository::domain::add(ownerPublicKey, name);

  return env->NewStringUTF(ret.c_str());
}

JNIEXPORT jobject JNICALL Java_repository_Repository_domainUpdate
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  IROHA_ASSERT_FALSE(params.find(tag::DomainName) == params.end());
  const auto name = params.find(tag::DomainName)->second;

  logger::debug(NameSpaceID + "::domainUpdate") << "uuid: " << uuid
                                                << ", name: " << name;

  return JavaMakeBoolean(env, (jboolean) repository::domain::update(uuid, name));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_domainRemove
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::domainRemove") << "uuid: " << uuid;
  return JavaMakeBoolean(env, (jboolean) repository::domain::remove(uuid));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_domainFindByUuid
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::domainFindByUuid") << "uuid: " << uuid;

  auto domain = repository::domain::findByUuid(uuid);
  std::map<std::string, std::string> domainMap;
  {
    domainMap[tag::OwnerPublicKey] = domain.ownerpublickey();
    domainMap[tag::DomainName] = domain.name();
  }
  return JavaMakeMap(env, domainMap);
}

JNIEXPORT jobject JNICALL Java_repository_Repository_domainExists
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::domainExists") << " uuid: " << uuid;
  return JavaMakeBoolean(env, (jboolean) repository::domain::exists(params.find(tag::Uuid)->second));
}

/***************************************************************************************
 * Asset
 ***************************************************************************************/

JNIEXPORT jstring JNICALL Java_repository_Repository_assetAdd
    (JNIEnv *env, jclass, jobject params_, jobject value_) {

  const auto params = convertJavaHashMapValueString(env, params_);


  IROHA_ASSERT_FALSE(params.find(tag::PublicKey) == params.end());
  const auto publicKey = params.find(tag::PublicKey)->second;

  IROHA_ASSERT_FALSE(params.find(tag::DomainId) == params.end());
  const auto domain = params.find(tag::DomainId)->second;

  IROHA_ASSERT_FALSE(params.find(tag::AssetName) == params.end());
  const auto name = params.find(tag::AssetName)->second;


  const auto value = convertAssetValueHashMap(env, value_);

  logger::debug(NameSpaceID + "::assetAdd")
      << "domainId: " << domain << ", assetName: " << name;

  Api::Asset asset;
  asset.set_name(name);
  asset.set_domain(domain);

  const auto ret =
      repository::asset::add(publicKey, name, asset);

  return env->NewStringUTF("OK");
}

JNIEXPORT jobject JNICALL Java_repository_Repository_assetUpdate
    (JNIEnv *env, jclass, jobject params_, jobject value_) {
/*
  const auto params = convertJavaHashMapValueString(env, params_);
  IROHA_ASSERT_FALSE(params.find(tag::DomainId) == params.end());
  const auto domain = params.find(tag::DomainId)->second;

  IROHA_ASSERT_FALSE(params.find(tag::DomainName) == params.end());
  const auto name   = params.find(tag::DomainName)->second;

  IROHA_ASSERT_FALSE(params.find(tag::SmartContractName) == params.end());
  const auto smartContractName = params.find(tag::SmartContractName)->second;
*/
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  const auto value = convertAssetValueHashMap(env, value_);
  // WIP
  return JavaMakeBoolean(env, (jboolean) false);  // Updates value only.
}

JNIEXPORT jobject JNICALL Java_repository_Repository_assetRemove
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::PublicKey) == params.end());
  const auto publicKey = params.find(tag::PublicKey)->second;

  IROHA_ASSERT_FALSE(params.find(tag::AssetName) == params.end());
  const auto assetName = params.find(tag::AssetName)->second;

  // WIP
  return JavaMakeBoolean(env, (jboolean) repository::asset::remove(publicKey, assetName));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_assetInfoFindByUuid
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::PublicKey) == params.end());
  const auto publicKey = params.find(tag::PublicKey)->second;

  IROHA_ASSERT_FALSE(params.find(tag::AssetName) == params.end());
  const auto assetName = params.find(tag::AssetName)->second;

  logger::debug(NameSpaceID + "::assetInfoFindByUuid") << "publicKey: " << publicKey;

  auto asset = repository::asset::find(publicKey, assetName);

  std::map<std::string, std::string> assetMap;
  {
    assetMap[tag::DomainId] = asset.domain();
    assetMap[tag::AssetName] = asset.name();
    assetMap[tag::SmartContractName] = "none";
  }

  logger::debug(NameSpaceID + "::assetInfoFindByUuid")
      << "assetMap[tag::DomainId]: \"value\":" << assetMap[tag::DomainId] << ", "
      << "assetMap[tag::AssetName]: \"value\":" << assetMap[tag::AssetName] << ", "
      << "assetMap[tag::SmartContractName]: \"value\":"
      << assetMap[tag::SmartContractName];

  return JavaMakeMap(env, assetMap);
}

JNIEXPORT jobject JNICALL Java_repository_Repository_assetValueFindByUuid
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::PublicKey) == params.end());
  const auto publicKey = params.find(tag::PublicKey)->second;

  IROHA_ASSERT_FALSE(params.find(tag::AssetName) == params.end());
  const auto assetName = params.find(tag::AssetName)->second;

  logger::debug(NameSpaceID + "::assetValueFindByUuid") << "publicKey: " << publicKey;

  auto asset = repository::asset::find(publicKey, assetName);

  ::txbuilder::Map value(asset.value().begin(), asset.value().end());
  // std::map<std::string, Api::BaseObject>
  // -> std::map<std::string, std::map<std::string, std::string>>
  std::map<std::string, std::map<std::string, std::string>> assetValue;
  for (auto &&e : value) {
    assetValue.emplace(e.first, convertBaseObjectToMapString(e.second));
  }

  std::string paramsStr = "{";
  for (auto &&e : assetValue) {
    paramsStr += "{" + e.first + ",";
    for (auto &&u : e.second) {
      paramsStr += "{" + u.first + "," + u.second + "},";
    }
    paramsStr += "},";
  }
  paramsStr += "}";

  logger::debug(NameSpaceID + "::assetValueFindByUuid") << "value: "
                                                        << paramsStr;

  return JavaMakeMap(env, assetValue);
}

JNIEXPORT jobject JNICALL Java_repository_Repository_assetExists
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::AssetName) == params.end());
  const auto assetName = params.find(tag::AssetName)->second;

  IROHA_ASSERT_FALSE(params.find(tag::PublicKey) == params.end());
  const auto publicKey = params.find(tag::PublicKey)->second;

  logger::debug(NameSpaceID + "::assetExists") << " publicKey: " << publicKey;
  return JavaMakeBoolean(env, (jboolean) repository::asset::exists(publicKey, assetName));
}

/***************************************************************************************
 * SimpleAsset
 ***************************************************************************************/
JNIEXPORT jstring JNICALL Java_repository_Repository_simpleAssetAdd
    (JNIEnv *env, jclass, jobject params_, jobject value_) {
  const auto params = convertJavaHashMapValueString(env, params_);
  IROHA_ASSERT_FALSE(params.find(tag::DomainId) == params.end());
  const auto domain = params.find(tag::DomainId)->second;

  IROHA_ASSERT_FALSE(params.find(tag::SimpleAssetName) == params.end());
  const auto name = params.find(tag::SimpleAssetName)->second;

  IROHA_ASSERT_FALSE(params.find(tag::SmartContractName) == params.end());
  const auto smartContractName = params.find(tag::SmartContractName)->second;

  const Api::BaseObject value = convertSimpleAssetValueHashMap(env, value_);

  logger::debug(NameSpaceID + "::simpleAssetAdd")
      << "domainId: " << domain << ", name: " << name
      << ", smartContractName: " << smartContractName;

  const auto ret =
      repository::simple_asset::add(domain, name, value, smartContractName);

  return env->NewStringUTF(ret.c_str());
}

JNIEXPORT jobject JNICALL Java_repository_Repository_simpleAssetUpdate
    (JNIEnv *env, jclass, jobject params_, jobject value_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  const auto value = convertSimpleAssetValueHashMap(env, value_);

  return JavaMakeBoolean(env, repository::simple_asset::update(uuid, value));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_simpleAssetRemove
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID) << "::simpleAssetRemove()\n";

  return JavaMakeBoolean(env, repository::simple_asset::remove(uuid));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_simpleAssetInfoFindByUuid
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::simpleAssetInfoFindByUuid") << "uuid: "
                                                             << uuid;

  auto simpleAsset = repository::simple_asset::findByUuid(uuid);

  std::map<std::string, std::string> simpleAssetInfo;
  {
    simpleAssetInfo[tag::DomainId] = simpleAsset.domain();
    simpleAssetInfo[tag::AssetName] = simpleAsset.name();
    simpleAssetInfo[tag::SmartContractName] = "";
  }

  logger::debug(NameSpaceID + "::simpleAssetInfoFindByUuid")
      << "simpleAssetInfo[tag::DomainId]: \"value\":" << simpleAssetInfo[tag::DomainId] << ", "
      << "simpleAssetInfo[tag::AssetName]: \"value\":" << simpleAssetInfo[tag::AssetName] << ", "
      << "simpleAssetInfo[tag::SmartContractName]: \"value\":"
      << simpleAssetInfo[tag::SmartContractName];

  return JavaMakeMap(env, simpleAssetInfo);
}

JNIEXPORT jobject JNICALL Java_repository_Repository_simpleAssetValueFindByUuid
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::simpleAssetValueFindByUuid") << "uuid: "
                                                              << uuid;

  auto simpleAsset = repository::simple_asset::findByUuid(uuid);
  return JavaMakeMap(env, convertBaseObjectToMapString(simpleAsset.value()));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_simpleAssetExists
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::simpleAssetExists") << " uuid: " << uuid;
  return JavaMakeBoolean(env, repository::simple_asset::exists(uuid));
}

/***************************************************************************************
 * Peer
 ***************************************************************************************/
JNIEXPORT jstring JNICALL Java_repository_Repository_peerAdd
    (JNIEnv *env, jclass, jobject params_, jobject trust_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::PublicKey) == params.end());
  const auto publicKey = params.find(tag::PublicKey)->second;

  IROHA_ASSERT_FALSE(params.find(tag::PeerAddress) == params.end());
  const auto address = params.find(tag::PeerAddress)->second;

  const auto trust = convertMapStringToTrust(convertJavaHashMapValueString(env, trust_));

  logger::debug(NameSpaceID + "::peerAdd")
      << "publicKey: " << publicKey << ", address: " << address;

  const auto ret = repository::peer::add(publicKey, address, trust);

  return env->NewStringUTF(ret.c_str());
}

JNIEXPORT jobject JNICALL Java_repository_Repository_peerUpdate
    (JNIEnv *env, jclass, jobject params_, jobject trust_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  IROHA_ASSERT_FALSE(params.find(tag::PeerAddress) == params.end());
  const auto address = params.find(tag::PeerAddress)->second;

  const auto trust = convertMapStringToTrust(convertJavaHashMapValueString(env, trust_));

  logger::debug(NameSpaceID + "::peerUpdate") << "uuid: " << uuid
                                              << ", address: " << address
                                              << ", trust value: " << trust.value()
                                              << ", trust isOk: " << (trust.isok() ? "true" : "false");

  return JavaMakeBoolean(env, repository::peer::update(uuid, address, trust));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_peerRemove
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::peerRemove") << "uuid: " << uuid;
  return JavaMakeBoolean(env, repository::domain::remove(uuid));
}

JNIEXPORT jobject JNICALL Java_repository_Repository_peerInfoFindByUuid
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::peerInfoFindByUuid") << "uuid: " << uuid;

  auto peer = repository::peer::findByUuid(uuid);
  std::map<std::string, std::string> peerMap;
  {
    peerMap[tag::PublicKey] = peer.publickey();
    peerMap[tag::PeerAddress] = peer.address();
  }
  return JavaMakeMap(env, peerMap);
}

JNIEXPORT jobject JNICALL Java_repository_Repository_peerTrustFindByUuid
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::peerTrustFindByUuid") << "uuid: " << uuid;
  Api::Peer peer = repository::peer::findByUuid(uuid);
  auto trsutMap = convertTrustToMapString(peer.trust());
  return JavaMakeMap(env, trsutMap);
}

JNIEXPORT jobject JNICALL Java_repository_Repository_peerExists
    (JNIEnv *env, jclass, jobject params_) {
  const auto params = convertJavaHashMapValueString(env, params_);

  IROHA_ASSERT_FALSE(params.find(tag::Uuid) == params.end());
  const auto uuid = params.find(tag::Uuid)->second;

  logger::debug(NameSpaceID + "::peerExists") << " uuid: " << uuid;
  return JavaMakeBoolean(env, repository::domain::exists(params.find(tag::Uuid)->second));
}