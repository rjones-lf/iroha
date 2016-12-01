/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

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
#ifndef CORE_CONSENSUS_EVENT_HPP_
#define CORE_CONSENSUS_EVENT_HPP_

#include <string>
#include <tuple>
#include <vector>

#include "../service/json_parse.hpp"

namespace event{

class Event {
public:
  int order = 0;
  virtual void addSignature(const std::string& publicKey, const std::string& signature) = 0;
  virtual int getNumValidSignatures() = 0;
  virtual bool eventSignatureIsEmpty() = 0;
  virtual std::vector<std::tuple<std::string,std::string>> eventSignatures() = 0;
  virtual std::string getHash() = 0;
  virtual json_parse::Object dump() = 0;
};

}
#endif