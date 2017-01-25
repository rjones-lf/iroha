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
#include "repository_DomainRepository.h"
#include "../../core/repository/domain/domain_repository.hpp"
#include "../../core/util/logger.hpp"
#include <string>
#include <memory>
#include <vector>
#include <assert.h>

JNIEXPORT void JNICALL Java_repository_DomainRepository_add__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jclass cls, jstring key_, jstring value_)
{
    const char *keyCString      = env->GetStringUTFChars(key_, 0);
    const char *valueCString    = env->GetStringUTFChars(value_, 0);

    const auto key              = std::string(keyCString);
    const auto value            = std::string(valueCString);

    env->ReleaseStringUTFChars(key_,    keyCString);
    env->ReleaseStringUTFChars(value_,  valueCString);

    logger::debug("domain repo jni") << key << ", " << value;

    repository::domain::add(key, value);
}

JNIEXPORT void JNICALL Java_repository_DomainRepository_add__Ljava_lang_String_2I
  (JNIEnv *env, jclass cls, jstring key_, jint value_)
{
    const char *keyCString      = env->GetStringUTFChars(key_, 0);

    const auto key              = std::string(keyCString);
    const auto value            = static_cast<int32_t>(value_);

    env->ReleaseStringUTFChars(key_,    keyCString);

    logger::debug("domain repo jni") << key << ", " << value;

    repository::domain::add(key, value);
}

JNIEXPORT void JNICALL Java_repository_DomainRepository_add__Ljava_lang_String_2J
  (JNIEnv *env, jclass cls, jstring key_, jlong value_)
{
    const char *keyCString      = env->GetStringUTFChars(key_, 0);

    const auto key              = std::string(keyCString);
    const auto value            = static_cast<int64_t>(value_);

    env->ReleaseStringUTFChars(key_,    keyCString);

    logger::debug("domain repo jni") << key << ", " << value;

    repository::domain::add(key, value);
}

JNIEXPORT void JNICALL Java_repository_DomainRepository_add__Ljava_lang_String_2D
  (JNIEnv *env, jclass cls, jstring key_, jdouble value_)
{
    const char *keyCString      = env->GetStringUTFChars(key_, 0);

    const auto key              = std::string(keyCString);
    const auto value            = static_cast<double>(value_);

    env->ReleaseStringUTFChars(key_,    keyCString);

    logger::debug("domain repo jni") << key << ", " << value;

    repository::domain::add(key, value);
}

JNIEXPORT jobject JNICALL Java_repository_AssetRepository_findOne
  (JNIEnv *env, jclass cls, jstring key_)
{
    const char *keyCString = env->GetStringUTFChars(key_, 0);
    const auto key = std::string(keyCString);

    env->ReleaseStringUTFChars(key_, keyCString);

    logger::debug("domain repo jni :: findOne") << key;

    // TODO: Specify template type.
//    auto p = repository::domain::findOne<  >(key);

    // TODO: Serialize to string for Java to read domain data here.



    // make jstring from std::string
    // http://stackoverflow.com/questions/6989740/conversion-from-basic-string-to-jstring
    // but, compile error has ocurred.
    // compiler says signature is as follows: jstring NewStringUTF(const char *utf) {
    const auto ret = std::string("serialized_stirng");
    return env->NewStringUTF(ret.c_str());
}

JNIEXPORT void JNICALL Java_repository_DomainRepository_update
  (JNIEnv *env, jclass cls, jstring key_, jstring newValue_)
{
    const char *keyCString      = env->GetStringUTFChars(key_, 0);
    const char *newValueCString = env->GetStringUTFChars(newValue_, 0);

    const auto key              = std::string(keyCString);
    const auto newValue         = std::string(newValueCString);

    env->ReleaseStringUTFChars(key_,    keyCString);
    env->ReleaseStringUTFChars(newValue_,  newValueCString);

    logger::debug("domain repo jni :: update") << key << ", " << newValue;

    repository::domain::update(key, newValue);
}

JNIEXPORT void JNICALL Java_repository_DomainRepository_remove
  (JNIEnv *env, jclass cls, jstring key_)
{
    const char *keyCString      = env->GetStringUTFChars(key_, 0);

    const auto key              = std::string(keyCString);

    env->ReleaseStringUTFChars(key_,    keyCString);

    logger::debug("domain repo jni :: remove") << key;

    // TODO: Specify template type.
//    repository::domain::remove(key);
}
