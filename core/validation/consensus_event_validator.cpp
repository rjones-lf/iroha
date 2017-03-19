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

#include "consensus_event_validator.hpp"
#include <consensus/consensus_event.hpp>
#include <crypto/signature.hpp>

namespace consensus_event_validator {

  bool isValid(const consensus_event::ConsensusEvent event) {
    return true;//signaturesAreValid; // TODO: add more tests
  }

  bool isValid(std::string sig) {
    return signaturesAreValid; // TODO: add more tests
  }


  bool signaturesAreValid(const consensus_event::ConsensusEvent event) {
    for (auto sig : event.signatures) {
      if (!consensus_event_validator::isValid(sig)) {
        return false;
      }
    }
    return true;
  }

};  // namespace consensus_event_validator
