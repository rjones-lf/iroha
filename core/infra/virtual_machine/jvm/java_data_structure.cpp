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

#include <../smart_contract/repository/jni_constants.hpp>
#include <iostream>
#include <map>
#include <transaction_builder/helper/create_objects_helper.hpp>
#include <util/exception.hpp>

#include "java_data_structure.hpp"

namespace virtual_machine {
namespace jvm {

/*********************************************************************************************
 * C++ to Java data structure
 *********************************************************************************************/

JNIEXPORT jobject JNICALL JavaMakeBoolean(JNIEnv *env, jboolean value) {
  jclass booleanClass = env->FindClass("java/lang/Boolean");
  jmethodID methodID = env->GetMethodID(booleanClass, "<init>", "(Z)V");
  return env->NewObject(booleanClass, methodID, value);
}

// HashMap<String, String>
JNIEXPORT jobject JNICALL JavaMakeMap(JNIEnv *env, std::map<std::string,std::string> mMap) {
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

    return hashMapObj;
}

// HashMap<String, HashMap<String, String>>
JNIEXPORT jobject JNICALL
JavaMakeMap(JNIEnv *env,
            std::map<std::string, std::map<std::string, std::string>> mMap) {
  jclass hashMapClass = env->FindClass("java/util/HashMap");
  jmethodID hashMapInit = env->GetMethodID(hashMapClass, "<init>", "(I)V");
  jobject hashMapObj = env->NewObject(hashMapClass, hashMapInit, mMap.size());
  jmethodID hashMapOut = env->GetMethodID(
      hashMapClass, "put",
      "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

  for (auto it : mMap) {
    env->CallObjectMethod(hashMapObj, hashMapOut,
                          env->NewStringUTF(it.first.c_str()),
                          JavaMakeMap(env, it.second));
  }

  return hashMapObj;
}

std::vector<std::string>
convertJavaStringArrayRelease(JNIEnv *env, jobjectArray javaArray_) {
  std::vector<std::string> ret;
  const int arraySize = env->GetArrayLength(javaArray_);
  for (int i = 0; i < arraySize; i++) {
    jstring elementString_ =
        (jstring)(env->GetObjectArrayElement(javaArray_, i));
    const char *elementCString = env->GetStringUTFChars(elementString_, 0);
    ret.push_back(std::string(elementCString));
    env->ReleaseStringUTFChars(elementString_, elementCString);
  }
  return ret;
}

JNIEXPORT jobjectArray JNICALL
JavaMakeStringArray(JNIEnv *env, const std::vector<std::string> &vec) {
  jobjectArray ret;
  ret = (jobjectArray)env->NewObjectArray(
      vec.size(), env->FindClass("java/lang/String"), env->NewStringUTF(""));
  for (std::size_t i = 0; i < vec.size(); i++)
    env->SetObjectArrayElement(ret, i, env->NewStringUTF(vec[i].c_str()));
  return ret;
}

JNIEXPORT jobject JNICALL JavaMakeAssetValueMap(JNIEnv *env, const txbuilder::Map& value) {

  // txbuilder::Map<string, Api::BaseObject> -> map<string, map<string, string>>
  std::map<std::string, std::map<std::string, std::string>> cppMapInMap;
  for (const auto &e : value) {
    const auto &key = e.first;
    cppMapInMap.emplace(key, convertBaseObjectToMapString(e.second));
  }

  // map<string, map<string, string>>
  // -> HashMap<String, HashMap<String,String>>
  return JavaMakeMap(env, cppMapInMap);
}

/*********************************************************************************************
 * Java to C++ data structure
 *********************************************************************************************/

std::map<std::string, std::string>
convertJavaHashMapValueString(JNIEnv *env, jobject hashMapObj_) {

//  env->PushLocalFrame(256);

  jclass mapClass = env->FindClass("java/util/Map");
  jmethodID entrySet =
      env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
  jobject set = env->CallObjectMethod(hashMapObj_, entrySet);
  jclass setClass = env->FindClass("java/util/Set");
  jmethodID iterator =
      env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
  jobject iter = env->CallObjectMethod(set, iterator);
  jclass iteratorClass = env->FindClass("java/util/Iterator");
  jmethodID hasNext = env->GetMethodID(iteratorClass, "hasNext", "()Z");
  jmethodID next =
      env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
  jclass entryClass = env->FindClass("java/util/Map$Entry");
  jmethodID getKey =
      env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
  jmethodID getValue =
      env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");

  std::map<std::string, std::string> ret;

  while (env->CallBooleanMethod(iter, hasNext)) {

    jobject entry = env->CallObjectMethod(iter, next);
    jstring key = (jstring)env->CallObjectMethod(entry, getKey);

    const char *keyStr = env->GetStringUTFChars(key, nullptr);
    if (!keyStr) { // Out of memory
      return {};
    }

    jstring value = (jstring)env->CallObjectMethod(entry, getValue);
    const char *valueStr = env->GetStringUTFChars(value, nullptr);
    if (!valueStr) { // Out of memory
      env->ReleaseStringUTFChars(key, keyStr);
      return {};
    }

    ret[std::string(keyStr)] = std::string(valueStr);

    env->ReleaseStringUTFChars(value, valueStr);
    env->ReleaseStringUTFChars(key, keyStr);
  }

//  env->PopLocalFrame(nullptr);

  return ret;
}

std::map<std::string, std::map<std::string, std::string>>
convertJavaHashMapValueHashMap(JNIEnv *env, jobject hashMapObj_) {

  env->PushLocalFrame(256);  

  jclass mapClass = env->FindClass("java/util/Map");
  jmethodID entrySet =
      env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
  jobject set = env->CallObjectMethod(hashMapObj_, entrySet);
  jclass setClass = env->FindClass("java/util/Set");
  jmethodID iterator =
      env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
  jobject iter = env->CallObjectMethod(set, iterator);
  jclass iteratorClass = env->FindClass("java/util/Iterator");
  jmethodID hasNext = env->GetMethodID(iteratorClass, "hasNext", "()Z");
  jmethodID next =
      env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
  jclass entryClass = env->FindClass("java/util/Map$Entry");
  jmethodID getKey =
      env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
  jmethodID getValue =
      env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");

  std::map<std::string, std::map<std::string, std::string>> ret;

  while (env->CallBooleanMethod(iter, hasNext)) {
    jobject entry = env->CallObjectMethod(iter, next);
    jstring key = (jstring)env->CallObjectMethod(entry, getKey);

    const char *keyStr = env->GetStringUTFChars(key, nullptr);
    if (!keyStr) { // Out of memory
      return {};
    }

    jobject value = (jobject)env->CallObjectMethod(entry, getValue);
    ret[std::string(keyStr)] = convertJavaHashMapValueString(env, value);

    env->ReleaseStringUTFChars(key, keyStr);
  }

  env->PopLocalFrame(nullptr);

  return ret;
}

/**********************************************************************
 * txbuilder::Map<std::string, Api::BaseObject>
 * <-> HashMap<String, HashMap<String, String>>
 **********************************************************************/
namespace valueType {

const std::string String = "string";
const std::string Int = "int";
const std::string Boolean = "boolean";
const std::string Double = "double";

} // namespace valueTypes

Api::BaseObject convertBaseObjectFromMap(std::map<std::string, std::string>& baseObjectMap) {
  const auto valueType = baseObjectMap["type"];
  const auto content = baseObjectMap["value"];
  if (valueType == valueType::String) {
    return txbuilder::createValueString(content);
  } else if (valueType == valueType::Int) {
    try {
      return txbuilder::createValueInt(std::stoi(content));
    } catch (std::invalid_argument) {
      throw "Value type mismatch: expected int";
    }
  } else if (valueType == valueType::Boolean) {
    auto boolStr = content;
    std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(),
                   ::tolower);
    return txbuilder::createValueBool(boolStr == "true");
  } else if (valueType == valueType::Double) {
    try {
      return txbuilder::createValueDouble(std::stod(content));
    } catch (std::invalid_argument) {
      throw "Value type mismatch: expected double";
    }
  } else {
    throw "Unknown value type";
  }
}

txbuilder::Map convertAssetValueHashMap(JNIEnv *env, jobject value_) {

  txbuilder::Map ret;

  std::map<std::string, std::map<std::string, std::string>> valueMapSM =
      convertJavaHashMapValueHashMap(env, value_);

  for (auto &e : valueMapSM) {
    const auto &key = e.first;
    ret.emplace(key, convertBaseObjectFromMap(e.second));
  }

  return ret;
}

Api::BaseObject convertSimpleAssetValueHashMap(JNIEnv *env, jobject value_) {

  std::map<std::string, std::string> baseObjectMap =
    convertJavaHashMapValueString(env, value_);

  return convertBaseObjectFromMap(baseObjectMap);
}

/*********************************************************************************************
 * Api::BaseObject <-> C++ string map
 *********************************************************************************************/

std::map<std::string, std::string>
convertBaseObjectToMapString(const Api::BaseObject &value) {

  std::map<std::string, std::string> baseObjectMap;

  switch (value.value_case()) {
  case Api::BaseObject::kValueString:
    baseObjectMap.emplace("type", valueType::String);
    baseObjectMap.emplace("value", value.valuestring());
    break;
  case Api::BaseObject::kValueInt:
    baseObjectMap.emplace("type", valueType::Int);
    baseObjectMap.emplace("value", std::to_string(value.valueint()));
    break;
  case Api::BaseObject::kValueBoolean:
    baseObjectMap.emplace("type", valueType::Boolean);
    baseObjectMap.emplace("value", std::string(value.valueboolean() ? "true" : "false"));
    break;
  case Api::BaseObject::kValueDouble:
    baseObjectMap.emplace("type", valueType::Double);
    baseObjectMap.emplace("value", std::to_string(value.valuedouble()));
    break;
  default:
    throw "Invalid type";
  }

  return baseObjectMap;
}
/*
Api::BaseObject
convertMapStringToBaseObject(const std::map<std::string, std::string> &value) {

  Api::BaseObject baseObject;

  const auto& type = value.find("type")->second;
  const auto& val  = value.find("value")->second;

  if (type == valueType::String) {
    return txbuilder::createValueString(val);
  } else if (type == valueType::Int) {
    return txbuilder::createValueInt(std::stoi(val));
  } else if (type == valueType::Boolean) {
    return txbuilder::createValueBool(val == "true");
  } else if (type == valueType::Double) {
    return txbuilder::createValueDouble(std::stod(val));
  }

  throw "Invalid type";
}
*/
std::map<std::string, std::string>
convertTrustToMapString(const Api::Trust &trust) {
  std::map<std::string, std::string> ret;
  ret[jni_constants::PeerTrustValue] = std::to_string(trust.value());
  ret[jni_constants::PeerTrustIsOk] = trust.isok() ? "true" : "false";
  return ret;
}

Api::Trust
convertMapStringToTrust(const std::map<std::string, std::string> &trustMap) {
  double trustValue = 0.0;
  try {
    trustValue = std::stod(trustMap.find(jni_constants::PeerTrustValue)->second);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    throw exception::InvalidCastException("Cannot convert peer trust value, string to double", __FILE__);
  }

  bool trustIsOk = false;
  try {
    trustIsOk = trustMap.find(jni_constants::PeerTrustIsOk)->second == "true";
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    throw exception::InvalidCastException("Cannot convert peer trust isOk, string to bool", __FILE__);
  }

  return txbuilder::createTrust(trustValue, trustIsOk);
}

} // namespace jvm
} // namespace virtual_machine
