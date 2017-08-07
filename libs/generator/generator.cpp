/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include <stdlib.h>
#include "generator.hpp"

namespace generator {

  int64_t random_number(int64_t min, int64_t max) {
    uint32_t SEED_ = 1337;
    return min + (rand_r(&SEED_) % (max - min));
  }

  uint8_t random_printable_char() {
    return (uint8_t)random_number(32, 126 + 1);
  }

}  // namespace generator
