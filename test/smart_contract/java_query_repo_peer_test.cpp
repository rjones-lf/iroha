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

#include <gtest/gtest.h>

#include <cmath>
#include <../smart_contract/repository/jni_constants.hpp>
#include <infra/protobuf/api.pb.h>
#include <infra/virtual_machine/jvm/java_data_structure.hpp>
#include <repository/domain/peer_repository.hpp>
#include <repository/world_state_repository.hpp>
#include <transaction_builder/transaction_builder.hpp>
#include <virtual_machine/virtual_machine.hpp>

const std::string PackageName = "test";
const std::string ContractName = "TestPeer";

namespace tag = jni_constants;

/*********************************************************************************************************
 * Test Account
 *********************************************************************************************************/
TEST(JavaQueryRepoPeer, initializeVM) {
  virtual_machine::initializeVM(PackageName, ContractName);
}

TEST(JavaQueryRepoPeer, invokeAddPeer) {

  /*****************************************************************
   * Remove chache
   *****************************************************************/
  const auto uuid =
      "eeeada754cb39bff9f229bca75c4eb8e743f0a77649bfedcc47513452c9324f5";

  if (repository::peer::exists(uuid)) {
    repository::peer::remove(uuid);
  }

  /*****************************************************************
   * Invoke Java method
   *****************************************************************/
  const std::string FunctionName = "testAddPeer";

  std::map<std::string, std::string> params;
  {
    params[tag::PublicKey] = "MPTt3ULszCLGQqAqRgHj2gQHVnxn/DuNlRXR/iLMAn4=";
    params[tag::PeerAddress] = "this is address";
  }

  auto trust = txbuilder::createTrust(1.234567890987654321, true);

  virtual_machine::invokeFunction(PackageName, ContractName, FunctionName,
                                  params, virtual_machine::jvm::convertTrustToMapString(trust));

  const std::string strPeer = repository::world_state_repository::find(uuid);

  Api::Peer peer;
  peer.ParseFromString(strPeer);

  ASSERT_STREQ(params[tag::PublicKey].c_str(), peer.publickey().c_str());
  ASSERT_STREQ(params[tag::PeerAddress].c_str(), peer.address().c_str());

  constexpr double Eps = 1e-5;
  ASSERT_TRUE(std::abs(trust.value() - peer.trust().value()) < Eps);
  ASSERT_TRUE(trust.isok() == true);

  // Remove cache again
  ASSERT_TRUE(repository::peer::remove(uuid));
}

TEST(JavaQueryRepoPeer, invokeUpdatePeer) {

  /*****************************************************************
   * Remove cache & initialize
   *****************************************************************/
  const auto uuid =
      "eeeada754cb39bff9f229bca75c4eb8e743f0a77649bfedcc47513452c9324f5";

  if (repository::peer::exists(uuid)) {
    repository::peer::remove(uuid);
  }

  repository::peer::add("MPTt3ULszCLGQqAqRgHj2gQHVnxn/DuNlRXR/iLMAn4=",
                        "this is address",
                        txbuilder::createTrust(1.234567890987654321, true));

  ASSERT_TRUE(repository::peer::exists(uuid));

  /*****************************************************************
   * Invoke Java method
   *****************************************************************/
  const std::string FunctionName = "testUpdatePeer";

  std::map<std::string, std::string> params;
  {
    params[tag::Uuid] = uuid;
    params[tag::PeerAddress] = "Updated Peer Address";
  }

  auto trust = txbuilder::createTrust(98765.432123456789, false);

  virtual_machine::invokeFunction(PackageName, ContractName, FunctionName,
                                  params, virtual_machine::jvm::convertTrustToMapString(trust));

  const std::string strPeer = repository::world_state_repository::find(uuid);

  Api::Peer peer;
  peer.ParseFromString(strPeer);

  ASSERT_STREQ("MPTt3ULszCLGQqAqRgHj2gQHVnxn/DuNlRXR/iLMAn4=",
               peer.publickey().c_str());
  ASSERT_STREQ(params[tag::PeerAddress].c_str(), peer.address().c_str());
  constexpr double Eps = 1e-5;
  ASSERT_TRUE(std::abs(trust.value() - peer.trust().value()) < Eps);

  // Remove chache again
  repository::peer::remove(uuid);
}

TEST(JavaQueryRepoPeer, invokeRemovePeer) {

  /*****************************************************************
  * Remove cache & initialize
  *****************************************************************/
  const std::string uuid =
      "eeeada754cb39bff9f229bca75c4eb8e743f0a77649bfedcc47513452c9324f5";

  if (repository::peer::exists(uuid)) {
    repository::peer::remove(uuid);
  }

  repository::peer::add("MPTt3ULszCLGQqAqRgHj2gQHVnxn/DuNlRXR/iLMAn4=",
                        "this is peer address",
                        txbuilder::createTrust(0, true));

  ASSERT_TRUE(repository::peer::exists(uuid));

  /*****************************************************************
   * Invoke Java method
   *****************************************************************/
  const std::string FunctionName = "testRemovePeer";

  std::map<std::string, std::string> params;
  { params[tag::Uuid] = uuid; }

  virtual_machine::invokeFunction(PackageName, ContractName, FunctionName,
                                  params);

  ASSERT_TRUE(!repository::peer::exists(uuid));
}

TEST(JavaQueryRepoDomain, finishVM) {
  virtual_machine::finishVM(PackageName, ContractName);
}
