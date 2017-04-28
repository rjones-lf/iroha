/**
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_FLATBUF_SERVICE_NESTED_FLATBUFFER_SERVICE_H_
#define IROHA_FLATBUF_SERVICE_NESTED_FLATBUFFER_SERVICE_H_

#include <string>
#include <vector>
#include <cstdint>
#include <flatbuffers/flatbuffers.h>

namespace flatbuffer_service {
// namespace nested_flatbuffer_service {

std::vector<uint8_t> CreateAccountBuffer(
    const char* publicKey, const char* alias,
    const std::vector<std::string>& signatories,
    uint16_t useKeys);

// } // namespace nested_flatbuffer_service {
}  // namespace flatbuffer_service

#endif
