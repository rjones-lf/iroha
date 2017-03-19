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
#include <algorithm>
#include <array>
#include <util/logger.hpp>
#include <infra/config/iroha_config_with_json.hpp>

#include "java_virtual_machine.hpp"
#include "java_data_structure.hpp"

namespace virtual_machine {
  namespace jvm {

/*
    void Java_SmartContract_save(JNIEnv *env, jobject thiz, jstring key, jstring value) {
        const char *keyChar = env->GetStringUTFChars(key, 0);
        const char *valueChar = env->GetStringUTFChars(value, 0);

        std::cout << "[" << keyChar << ", " << valueChar << std::endl;

        env->ReleaseStringUTFChars(key, keyChar);
        env->ReleaseStringUTFChars(value, valueChar);
    }
*/
    std::unique_ptr<JavaContext>
    initializeVM(const std::string &packageNameUnderInstances, const std::string &contractName) {

      const auto IrohaHome = []() {
        const auto p = getenv("IROHA_HOME");
        if (p == nullptr) {
          throw "[FATAL] Set environment $IROHA_HOME";
        }
        return std::string(p);
      }();

      const auto packageName = "instances." + packageNameUnderInstances;

      // paths are hard coding here...
      std::vector<std::string> java_args = {
          "-Djava.class.path=" + IrohaHome + "/" +
          config::IrohaConfigManager::getInstance().getJavaClassPath("java_tests") + ":"
          + IrohaHome + "/" +
          config::IrohaConfigManager::getInstance().getJavaClassPathLocal("smart_contract/java_tests"),
          "-Djava.library.path=" + IrohaHome + "/" +
          config::IrohaConfigManager::getInstance().getJavaLibraryPath("lib") + ":"
          + IrohaHome + "/" + config::IrohaConfigManager::getInstance().getJavaLibraryPathLocal("build/lib"),
          "-Djava.security.policy=" + IrohaHome + "/" +
          config::IrohaConfigManager::getInstance().getJavaPolicyPath("jvm/java.policy.txt"),
          "-Djava.security.manager"
      };

      const int OptionSize = java_args.size();

      JavaVMOption options[OptionSize];
      for (int i = 0; i < OptionSize; i++) {
        options[i].optionString = const_cast<char *>( java_args[i].c_str());
      }

      {
        for (int i = 0; i < OptionSize; i++) {
          std::cout << options[i].optionString << " ";
        }
        std::cout << packageName + "." + contractName << std::endl;
      }

      JavaVMInitArgs vm_args;
      vm_args.version = JNI_VERSION_1_8;
      vm_args.options = options;
      vm_args.nOptions = OptionSize;
      vm_args.ignoreUnrecognized = JNI_FALSE;

      JNIEnv *env;
      JavaVM *jvm;

      int res = JNI_CreateJavaVM(&jvm, (void **) &env, &vm_args);
      if (res) {
        std::cout << "cannot run JavaVM : " << res << std::endl;
        return nullptr;
      }

      auto slashPackageName = packageName;
      std::transform(slashPackageName.begin(), slashPackageName.end(), slashPackageName.begin(), [](const char a) {
        return a == '.' ? '/' : a;
      });

      jclass cls = env->FindClass((slashPackageName + "/" + contractName).c_str());
      if (cls == nullptr) {
        std::cout << "could not found class : " << packageName << "." << contractName << std::endl;
        return nullptr;
      }

      jmethodID cns = env->GetMethodID(cls, "<init>", "()V");
      if (cns == nullptr) {
        std::cout << "could not get <init> method." << std::endl;
        return nullptr;
      }

      jobject obj = env->NewObject(cls, cns);

      return std::make_unique<JavaContext>(
          env,
          jvm,
          vm_args,
          packageName + "." + contractName,
          cls,
          obj
      );
    }

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName,
        const std::map<std::string, std::string> &params
    ) {
      jmethodID mid = context->env->GetStaticMethodID(context->jClass, functionName.c_str(), "(Ljava/util/HashMap;)V");
      if (mid == nullptr) {
        std::cout << "could not get method : " << functionName << std::endl;
        return;
      }

      jobject jmap = JavaMakeMap(context->env, params);

      context->env->CallVoidMethod(context->jObject, mid, jmap);

      context->env->DeleteLocalRef(jmap);
    }

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName,
        const std::map<std::string, std::string> &params,
        const std::map<std::string, std::string> &value
    ) {
      jmethodID mid = context->env->GetStaticMethodID(context->jClass, functionName.c_str(),
                                                      "(Ljava/util/HashMap;Ljava/util/HashMap;)V");
      if (mid == nullptr) {
        std::cout << "could not get method : " << functionName << std::endl;
        return;
      }

      jobject jmap1 = JavaMakeMap(context->env, params);
      jobject jmap2 = JavaMakeMap(context->env, value);

      context->env->CallVoidMethod(context->jObject, mid, jmap1, jmap2);

      context->env->DeleteLocalRef(jmap1);
      context->env->DeleteLocalRef(jmap2);
    }

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName,
        const std::map<std::string, std::string> &params,
        const std::map<std::string, std::map<std::string, std::string>> &value
    ) {
      jmethodID mid = context->env->GetStaticMethodID(context->jClass, functionName.c_str(),
                                                      "(Ljava/util/HashMap;Ljava/util/HashMap;)V");
      if (mid == nullptr) {
        std::cout << "could not get method : " << functionName << std::endl;
        return;
      }

      jobject jmap = JavaMakeMap(context->env, params);
      jobject jmapInMap = JavaMakeMap(context->env, value);

      context->env->CallVoidMethod(context->jObject, mid, jmap, jmapInMap);

      context->env->DeleteLocalRef(jmap);
      context->env->DeleteLocalRef(jmapInMap);
    }

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName,
        const std::map<std::string, std::string> &params,
        const std::vector<std::string> &value
    ) {
      jmethodID mid = context->env->GetStaticMethodID(context->jClass, functionName.c_str(),
                                                      "(Ljava/util/HashMap;[Ljava/lang/String;)V");
      if (mid == nullptr) {
        std::cout << "could not get method : " << functionName << std::endl;
        return;
      }


      jobject jmap = JavaMakeMap(context->env, params);
      jobject jarr = JavaMakeStringArray(context->env, value);

      context->env->CallVoidMethod(context->jObject, mid, jmap, jarr);

      context->env->DeleteLocalRef(jmap);
      context->env->DeleteLocalRef(jarr);
    }

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        const std::string &functionName
    ) {
      jmethodID mid = context->env->GetStaticMethodID(context->jClass, functionName.c_str(), "()V");
      if (mid == nullptr) {
        std::cout << "could not get method : " << functionName << std::endl;
        return;
      }

      context->env->CallVoidMethod(context->jObject, mid);
    }
  }
}
