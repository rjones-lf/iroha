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

#ifndef _JAVA_DATA_STRUCTURE_HPP
#define _JAVA_DATA_STRUCTURE_HPP

#include <jni.h>
#include <string>
#include <map>
#include <vector>
#include <infra/protobuf/api.pb.h>
#include <transaction_builder/helper/create_objects_helper.hpp>

namespace virtual_machine {
  namespace jvm {

    JNIEXPORT jobject JNICALL JavaMakeBoolean(JNIEnv *env, jboolean value);

    JNIEXPORT jobject JNICALL JavaMakeMap(JNIEnv *env, std::map<std::string, std::string> mMap);

    JNIEXPORT jobject JNICALL JavaMakeMap(JNIEnv *env, std::map<std::string, std::map<std::string, std::string>> mMap);

    JNIEXPORT jobject JNICALL JavaMakeAssetValueMap(JNIEnv *env, const txbuilder::Map &value);

    std::vector<std::string> convertJavaStringArrayRelease(JNIEnv *env, jobjectArray javaArray_);

    JNIEXPORT jobjectArray JNICALL JavaMakeStringArray(JNIEnv *env, const std::vector<std::string> &vec);

    std::map<std::string, std::string> convertJavaHashMapValueString(JNIEnv *env, jobject hashMapObj_);

    std::map<std::string, std::map<std::string, std::string>>
    convertJavaHashMapValueHashMap(JNIEnv *env, jobject hashMapObj_);

    txbuilder::Map convertAssetValueHashMap(JNIEnv *env, jobject value_);

    Api::BaseObject convertSimpleAssetValueHashMap(JNIEnv *env, jobject value_);

    std::map<std::string, std::string> convertBaseObjectToMapString(const Api::BaseObject &value);
//Api::BaseObject convertMapStringToBaseObject(const std::map<std::string, std::string> &value);

    std::map<std::string, std::string> convertTrustToMapString(const Api::Trust &trust);

    Api::Trust convertMapStringToTrust(const std::map<std::string, std::string> &trustMap);

  }
}

#endif