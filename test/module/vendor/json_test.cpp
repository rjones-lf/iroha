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

#include <string>
#include <fstream>
#include <regex>

#include <json.hpp>

using json = nlohmann::json;

TEST(peer_service_with_json, standardConfig) {
  try {
    json::parse(
      "{\n"
      "  \"me\":{\n"
      "    \"ip\": \"172.17.0.2\",\n"
      "    \"name\": \"mizuki\",\n"
      "    \"privateKey\": \"AHR2sBfWEXyNUCacZxc/DtecvelfI3145OPCxZO/SkHXBHPOlvmb/B8Ymg2igcjh19zPIyB0En0atamrhNdXyw==\",\n"
      "    \"publicKey\": \"eAcdXyMmdjvhVzOu7fz1y0Ahaz4vbZOze28BX6OBVac=\"\n"
      "  },\n"
      "  \"group\":[\n"
      "    {\n"
      "      \"ip\": \"172.17.0.2\",\n"
      "      \"name\": \"mizuki\",\n"
      "      \"publicKey\": \"eAcdXyMmdjvhVzOu7fz1y0Ahaz4vbZOze28BX6OBVac=\"\n"
      "    }\n"
      "  ]\n"
      "}\n"
    );
  } catch(...) {
    FAIL() << "Could not read standard config json.";
  }
}

TEST(peer_service_with_json, escaped) {
  try {
    json::parse(
      "{"
        "\"object\":{"
          "\"something\": \"\\\" \u3000 \\\\ \""
        "}"
      "}"
    );    
  } catch(...) {
    FAIL() << "Could not read json which has escaped characters in text.";
  }
}

TEST(peer_service_with_json, nested) {
  try {
    json::parse(
      "{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":{\"f\":{\"g\":{\"h\":{\"i\":{\"j\":{\"k\":{\"l\":{\"m\":{\"n\":{\"o\":{\"p\":{\"q\":{\"r\":{\"s\":{\"t\":{\"u\":{\"v\":{\"w\":{\"x\":{\"y\":{\"z\":{\"tag\":\"value\"}}}}}}}}}}}}}}}}}}}}}}}}}}}"
    );
  } catch(...) {
    FAIL() << "Could not read nested json.";
  }
}

TEST(peer_service_with_json, tab) {
  try {
    json::parse(
      "\t\t{\t\"a\":\t\"b\"\t\t}\t\t"
    );
  } catch(...) {
    FAIL() << "Could not read json which has tab.";
  }
}

TEST(peer_service_with_json, empty) {
  try {
    json::parse("{}");
    json::parse("[]");
  } catch(...) {
    FAIL() << "Could not read empty json.";
  }
}

TEST(peer_service_with_json, integerValue) {
    try {
      json::parse(
        "{\"something\":123456789}"
      );
    } catch(...) {
      FAIL() << "Could not read integer value.";
    }
}
