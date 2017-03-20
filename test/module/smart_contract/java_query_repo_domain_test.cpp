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

#include "../../../smart_contract/repository/jni_constants.hpp"
#include <infra/virtual_machine/jvm/java_data_structure.hpp>
#include <repository/domain/domain_repository.hpp>
#include <virtual_machine/virtual_machine.hpp>

const std::string PackageName = "test";
const std::string ContractName = "TestDomain";

namespace tag = jni_constants;

/*********************************************************************************************************
 * Test Account
 *********************************************************************************************************/
TEST(JavaQueryRepoDomain, initializeVM) {
  virtual_machine::initializeVM(PackageName, ContractName);
}

TEST(JavaQueryRepoDomain, invokeAddDomain) {

  /*****************************************************************
   * Remove chache
   *****************************************************************/
  const auto uuid =
      "eeeada754cb39bff9f229bca75c4eb8e743f0a77649bfedcc47513452c9324f5";

  if (repository::domain::exists(uuid)) {
    repository::domain::remove(uuid);
  }

  /*****************************************************************
   * Invoke Java method
   *****************************************************************/
  const std::string FunctionName = "testAddDomain";

  std::map<std::string, std::string> params;
  {
    params[tag::OwnerPublicKey] =
        "MPTt3ULszCLGQqAqRgHj2gQHVnxn/DuNlRXR/iLMAn4=";
    params[tag::DomainName] = "domain name";
  }

  virtual_machine::invokeFunction(PackageName, ContractName, FunctionName,
                                  params);

  const auto domain = repository::domain::findByUuid(uuid);

  ASSERT_STREQ(params[tag::OwnerPublicKey].c_str(),
               domain.ownerpublickey().c_str());
  ASSERT_STREQ(params[tag::DomainName].c_str(), domain.name().c_str());

  // Remove cache again
  ASSERT_TRUE(repository::domain::remove(uuid));
}

TEST(JavaQueryRepoDomain, invokeUpdateDomain) {

  /*****************************************************************
   * Remove cache & initialize
   *****************************************************************/
  const auto uuid =
      "eeeada754cb39bff9f229bca75c4eb8e743f0a77649bfedcc47513452c9324f5";

  if (repository::domain::exists(uuid)) {
    repository::domain::remove(uuid);
  }

  repository::domain::add("MPTt3ULszCLGQqAqRgHj2gQHVnxn/DuNlRXR/iLMAn4=",
                          "domain name");

  ASSERT_TRUE(repository::domain::exists(uuid));

  /*****************************************************************
   * Invoke Java method
   *****************************************************************/
  const std::string FunctionName = "testUpdateDomain";

  std::map<std::string, std::string> params;
  {
    params[tag::Uuid] = uuid;
    params[tag::DomainName] = "Updated Domain Name";
  }

  virtual_machine::invokeFunction(PackageName, ContractName, FunctionName,
                                  params);

  const auto domain = repository::domain::findByUuid(uuid);

  ASSERT_STREQ("MPTt3ULszCLGQqAqRgHj2gQHVnxn/DuNlRXR/iLMAn4=",
               domain.ownerpublickey().c_str());
  ASSERT_STREQ(params[tag::DomainName].c_str(), domain.name().c_str());

  // Remove chache again
  repository::domain::remove(uuid);
}

TEST(JavaQueryRepoDomain, invokeRemoveDomain) {

  /*****************************************************************
  * Remove cache & initialize
  *****************************************************************/
  const std::string uuid =
      "eeeada754cb39bff9f229bca75c4eb8e743f0a77649bfedcc47513452c9324f5";

  if (repository::domain::exists(uuid)) {
    repository::domain::remove(uuid);
  }

  repository::domain::add("MPTt3ULszCLGQqAqRgHj2gQHVnxn/DuNlRXR/iLMAn4=",
                          "domain name");

  ASSERT_TRUE(repository::domain::exists(uuid));

  /*****************************************************************
   * Invoke Java method
   *****************************************************************/
  const std::string FunctionName = "testRemoveDomain";

  std::map<std::string, std::string> params;
  { params[tag::Uuid] = uuid; }

  virtual_machine::invokeFunction(PackageName, ContractName, FunctionName,
                                  params);

  ASSERT_FALSE(repository::domain::exists(uuid));
}

TEST(JavaQueryRepoDomain, finishVM) {
  virtual_machine::finishVM(PackageName, ContractName);
}
