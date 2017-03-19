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

#include "convert_string.hpp"
#include <sstream>
#include <string>
#include <vector>

namespace convert_string {
  std::string to_string(const std::vector<std::string> &vs) {
    std::string ret = "[";
    for (std::size_t i = 0; i < vs.size(); i++) {
      if (i)
        ret += ", ";
      ret += vs[i];
    }
    ret += "]";
    return ret;
  }

  std::string stringifyVector(const std::vector<std::string> &vs) {
    std::string ret;
    for (std::size_t i = 0; i < vs.size(); i++) {
      ret += "[" + std::to_string(vs[i].size()) + "]";
      ret += vs[i];
    }
    return ret;
  }

  std::vector<std::string> parseVector(const std::string &s) {
    std::vector<std::string> ret;
    const char *cstr = s.c_str();
    const std::size_t size = s.size();
    for (std::size_t index = 0; index < size;) {
      if (s[index] == '[') {
        std::string buf;
        index++;
        char *e = nullptr;
        int num = std::strtol(&cstr[index], &e, 10);
        index += e - &cstr[index];
        index++;
        for (int k = index; k < static_cast<int>(index) + num; k++) {
          buf += cstr[k];
        }
        ret.push_back(buf);
        index += num;
      }
    }
    return ret;
  }
}
