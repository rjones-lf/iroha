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
#include "repository_AssetRepository.h"
#include "../../core/repository/domain/asset_repository.hpp"
#include "../../core/util/serialized_type_switcher.hpp"
#include "../../core/util/logger.hpp"
#include <string>
#include <memory>
#include <vector>
#include <assert.h>

JNIEXPORT void JNICALL Java_repository_AssetRepository_add
    (JNIEnv *env, jclass cls, jstring publicKey_, jstring assetName_, jstring value_) {
    const char *publicKeyCString    = env->GetStringUTFChars(publicKey_, 0);
    const char *assetNameCString    = env->GetStringUTFChars(assetName_, 0);
    const char *valueCString        = env->GetStringUTFChars(value_, 0);

    const auto publicKey    = std::string(publicKeyCString);
    const auto assetName    = std::string(assetNameCString);
    const auto value        = std::string(valueCString);

    env->ReleaseStringUTFChars(publicKey_, publicKeyCString);
    env->ReleaseStringUTFChars(assetName_, assetNameCString);
    env->ReleaseStringUTFChars(value_, valueCString);

    repository::asset::add(publicKey, assetName, value);
}

JNIEXPORT jobject JNICALL Java_repository_AssetRepository_findOne
  (JNIEnv *env, jclass cls, jstring key_)
{
    const char *keyCString  = env->GetStringUTFChars(key_, 0);
    const auto key          = std::string(keyCString);

    env->ReleaseStringUTFChars(key_, keyCString);

    repository::asset::findOne(key);
}

JNIEXPORT void JNICALL Java_repository_AssetRepository_update
  (JNIEnv *env, jclass cls, jstring publicKey_, jstring assetName_, jstring newValue_)
{
    const char *publicKeyCString    = env->GetStringUTFChars(publicKey_, 0);
    const char *assetNameCString    = env->GetStringUTFChars(assetName_, 0);
    const char *newValueCString     = env->GetStringUTFChars(newValue_,  0);

    const auto publicKey    = std::string(publicKeyCString);
    const auto assetName    = std::string(assetNameCString);
    const auto newValue     = std::string(newValueCString);

    env->ReleaseStringUTFChars(publicKey_, publicKeyCString);
    env->ReleaseStringUTFChars(assetName_, assetNameCString);
    env->ReleaseStringUTFChars(newValue_, newValueCString);

    repository::asset::update(publicKey, assetName, newValue);
}

JNIEXPORT void JNICALL Java_repository_AssetRepository_remove
  (JNIEnv *env, jclass, jstring publicKey_, jstring assetName_)
{
    const char *publicKeyCString    = env->GetStringUTFChars(publicKey_, 0);
    const char *assetNameCString    = env->GetStringUTFChars(assetName_, 0);

    const auto publicKey    = std::string(publicKeyCString);
    const auto assetName    = std::string(assetNameCString);

    env->ReleaseStringUTFChars(publicKey_, publicKeyCString);
    env->ReleaseStringUTFChars(assetName_, assetNameCString);

    repository::asset::remove(publicKey, assetName);
}
