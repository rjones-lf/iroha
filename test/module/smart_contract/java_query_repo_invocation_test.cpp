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

#include <infra/protobuf/api.pb.h>
#include <virtual_machine/virtual_machine.hpp>
#include <infra/virtual_machine/jvm/java_data_structure.hpp>

const std::string PackageName = "test";
const std::string ContractName = "TestInvocation";

/*********************************************************************************************************
 * Test Invocation
 *********************************************************************************************************/
TEST(JavaQueryRepoInvoke, InitializeVM) {
  virtual_machine::initializeVM(PackageName, ContractName);
}

TEST(JavaQueryRepoInvoke, Invoke_JAVA_function) {
  const std::string FunctionName = "test1";
  virtual_machine::invokeFunction(PackageName, ContractName, FunctionName);
}

TEST(JavaQueryRepoInvoke, Invoke_JAVA_function_map_argv) {

  const std::string FunctionName = "test2";

  std::map<std::string, std::string> params;
  {
    params["key1"] = "Mizuki";
    params["key2"] = "Sonoko";
  }

  virtual_machine::invokeFunction(PackageName, ContractName, FunctionName,
                                  params);
}

TEST(JavaQueryRepoInvoke, Invoke_JAVA_function_map_utf_8) {

  const std::string FunctionName = "test3";

  std::map<std::string, std::string> params;
  {
    params["key1"] = "水樹";
    params["key2"] = "素子";
  }

  virtual_machine::invokeFunction(PackageName, ContractName, FunctionName,
                                  params);
}

TEST(JavaQueryRepoInvoke, FinishVM) {
  virtual_machine::finishVM(PackageName, ContractName);
}