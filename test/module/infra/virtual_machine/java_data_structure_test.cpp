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
#include <map>

#include <infra/virtual_machine/jvm/java_data_structure.hpp>

JNIEnv *env;
JavaVM *jvm;

TEST(java_data_structure_test, initializeJavaVM) {
  JavaVMInitArgs vm_args;
  vm_args.version = JNI_VERSION_1_8;
  //  vm_args.options  = options;
  vm_args.nOptions = 0;
  vm_args.ignoreUnrecognized = JNI_FALSE;

  int res = JNI_CreateJavaVM(&jvm, (void **)&env, &vm_args);
  if (res) {
    throw "cannot run JavaVM";
  }
}

TEST(java_data_structure_test, JavaMakeMap) {
  std::map<std::string, std::string> params = {
      {"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"},
  };

  auto javaHashMap = virtual_machine::jvm::JavaMakeMap(env, params);
  auto cppMap =
      virtual_machine::jvm::convertJavaHashMapValueString(env, javaHashMap);

  for (auto &&e : cppMap) {
    std::cout << e.first << ", " << e.second << std::endl;
  }

  ASSERT_TRUE(params == cppMap);
}

TEST(java_data_structure_test, JavaMakeMapInMap) {

  std::map<std::string, std::string> params1 = {
      {"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"},
  };

  std::map<std::string, std::string> params2 = {
      {"Key1", "Value1"}, {"Key2", "Value2"}, {"Key3", "Value3"},
  };

  std::map<std::string, std::map<std::string, std::string>> paramsInParams;
  paramsInParams.emplace("KEY1", params1);
  paramsInParams.emplace("KEY2", params2);

  auto javaHashMapInHashMap =
      virtual_machine::jvm::JavaMakeMap(env, paramsInParams);
  auto cppMapInMap = virtual_machine::jvm::convertJavaHashMapValueHashMap(
      env, javaHashMapInHashMap);

  for (auto &&e : cppMapInMap) {
    std::cout << e.first << ",\n";
    for (auto &&u : e.second) {
      std::cout << "  " << u.first << ", " << u.second << std::endl;
    }
  }

  ASSERT_TRUE(paramsInParams == cppMapInMap);
}