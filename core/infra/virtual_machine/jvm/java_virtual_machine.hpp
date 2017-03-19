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

#ifndef _CORE_INFRA_VIRTUAL_MACHINE_JVM_JAVA_VIRTUAL_MACHINE_HPP
#define _CORE_INFRA_VIRTUAL_MACHINE_JVM_JAVA_VIRTUAL_MACHINE_HPP

#include <jni.h>
#include <memory>
#include <string>

#include <iostream>
#include <map>

namespace virtual_machine {
  namespace jvm {
    struct JavaContext {
      JNIEnv *env;
      JavaVM *jvm;
      jclass jClass;
      jobject jObject;
      std::string name;
      JavaVMInitArgs vmArgs;

      JavaContext(
          JNIEnv *aEnv,
          JavaVM *aJvm,
          JavaVMInitArgs aArgs,
          std::string aName,
          jclass cls,
          jobject obj
      ) :
          env(aEnv),
          jvm(aJvm),
          jClass(std::move(cls)),
          jObject(std::move(obj)),
          name(std::move(aName)),
          vmArgs(std::move(aArgs)) {}
    };


//    void Java_SmartContract_save(JNIEnv *env, jobject thiz, jstring key, jstring value);
    std::unique_ptr<JavaContext>
    initializeVM(const std::string &packageNameUnderInstances, const std::string &contractName);

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName,
        const std::map<std::string, std::string> &params
    );

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName,
        const std::map<std::string, std::string> &params,
        const std::map<std::string, std::string> &value
    );

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName,
        const std::map<std::string, std::string> &params,
        const std::map<std::string, std::map<std::string, std::string>> &value
    );

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName,
        const std::map<std::string, std::string> &params,
        const std::vector<std::string> &value
    );

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName
    );

  }
}

#endif //_CORE_INFRA_VIRTUAL_MACHINE_JVM_JAVA_VIRTUAL_MACHINE_HPP