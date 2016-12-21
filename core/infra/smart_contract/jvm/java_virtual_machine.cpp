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

#include "java_virtual_machine.hpp"
#include <algorithm>

namespace smart_contract {

    void Java_SmartContract_save(JNIEnv *env, jobject thiz, jstring key, jstring value) {
        const char *keyChar = env->GetStringUTFChars(key, 0);
        const char *valueChar = env->GetStringUTFChars(value, 0);

        std::cout << "[" << keyChar << ", " << valueChar << std::endl;

        env->ReleaseStringUTFChars(key, keyChar);
        env->ReleaseStringUTFChars(value, valueChar);
    }

    std::unique_ptr<JavaContext> initializeVM(std::string contractName) {
        JNIEnv *env;
        JavaVM *jvm;

        if (getenv("IROHA_HOME") == nullptr) {
            std::cout << "You must set IROHA_HOME!" << std::endl;
            return nullptr;
        }

        std::cout << "-Djava.class.path=" + (std::string) getenv("IROHA_HOME") +
                     "/smart_contract/" + contractName <<"/ "<< contractName.c_str() << std::endl;

        JavaVMOption options[3];
        //options[0].optionString = (char*)"-Djava.security.manager -Djava.security.policy=policy.txt -Djava.class.path=./contract/";
        options[1].optionString = (char *) "-Djava.security.manager";
        options[2].optionString = (char *) "-Djava.security.policy=policy.txt";
        options[0].optionString = (char *) ("-Djava.class.path=" + (std::string) getenv("IROHA_HOME") +
                                            "/smart_contract").c_str();

        JavaVMInitArgs vm_args;
        vm_args.version = JNI_VERSION_1_6;
        vm_args.options = options;
        vm_args.nOptions = 3;
        //vm_args.ignoreUnrecognized = true;

        int res = JNI_CreateJavaVM(&jvm, (void **) &env, &vm_args);
        if (res) {
            std::cout << "cannot run JavaVM : " << res << std::endl;
            return nullptr;
        }
        jclass cls = env->FindClass("SampleCurrency/SampleCurrency");// (contractName+"/"+contractName).c_str());
        if (cls == 0) {
            std::cout << "could not found class : " << contractName << std::endl;
            return nullptr;
        }

        jmethodID cns = env->GetMethodID(cls, "<init>", "()V");
        if (cns == NULL) {
            std::cout << "could not get <init> method." << std::endl;
            return nullptr;
        }
        jobject obj = env->NewObject(cls, cns);

        std::unique_ptr<JavaContext> ptr(new JavaContext(
            env,
            jvm,
            vm_args,
            std::move(contractName),
            cls,
            obj
        ));
        return ptr;
    }

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        std::string functionName,
        std::unordered_map<std::string,
        std::string> params
    ) {

        jmethodID mid = context->env->GetStaticMethodID(context->jClass, functionName.c_str(), "(Ljava/util/Map;)V");
        if (mid == NULL) {
            std::cout << "could not get method : " << functionName << std::endl;
            return;
        }
        context->env->CallVoidMethod(context->jObject, mid);

        auto res = context->jvm->DestroyJavaVM();
        if (res) {
            std::cout << "could not destroy JavaVM : " << res << std::endl;
        }
    }

};