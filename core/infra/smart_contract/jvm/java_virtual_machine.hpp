//
// Created by 五十嵐太清 on 2016/12/15.
//

#ifndef IROHA_JAVA_VIRTUAL_MACHINE_HPP_HPP
#define IROHA_JAVA_VIRTUAL_MACHINE_HPP_HPP

#include <jni.h>
#include <memory>
#include <string>

#include <iostream>
#include <unordered_map>

namespace smart_contract {

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
                env(std::move(aEnv)),
                jvm(std::move(aJvm)),
                jClass(std::move(cls)),
                jObject(std::move(obj)),
                name(std::move(aName)),
                vmArgs(std::move(aArgs)) {}
    };


    void Java_SmartContract_save(JNIEnv *env, jobject thiz, jstring key, jstring value);
    std::unique_ptr<JavaContext> initializeVM(std::string contractName);

    void execFunction(
        const std::unique_ptr<JavaContext> &context,
        std::string functionName,
        std::unordered_map<std::string,
        std::string> params
    );

}

#endif //IROHA_JAVA_VIRTUAL_MACHINE_HPP_HPP
