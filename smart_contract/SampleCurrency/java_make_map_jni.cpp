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
#include "utility_JavaMakeMap.h"
#include "../../core/repository/domain/domain_repository.hpp"
#include "../../core/util/logger.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <assert.h>

JNIEXPORT jobject JNICALL JavaMakeMap(JNIEnv *env, std::unordered_map<std::string,std::string> mMap) {
    env->PushLocalFrame(256); // fix for local references
    jclass hashMapClass= env->FindClass( "java/util/HashMap" );
    jmethodID hashMapInit = env->GetMethodID( hashMapClass, "<init>", "(I)V");
    jobject hashMapObj = env->NewObject( hashMapClass, hashMapInit, mMap.size());
    jmethodID hashMapOut = env->GetMethodID( hashMapClass, "put",
                "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    for (auto it : mMap)
    {
        env->CallObjectMethod( hashMapObj, hashMapOut,
             env->NewStringUTF(it.first.c_str()),
             env->NewStringUTF(it.second.c_str()));
    }

    env->PopLocalFrame(hashMapObj);
    return hashMapObj;
}