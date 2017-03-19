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

#include <benchmark/benchmark.h>
#include <algorithm>
#include <crypto/hash.hpp>


static void HASH_Sha3_256_with_keccak(benchmark::State &state) {
  std::string s = "0123456789";  // length = 10
  while (state.KeepRunning()) {
    hash::sha3_256_hex(s);
  }
}

static void HASH_Sha3_512_with_keccak(benchmark::State &state) {
  std::string s = "0123456789";  // length = 10
  while (state.KeepRunning()) {
    hash::sha3_512_hex(s);
  }
}

/**
 * These two tests show the number of hashes calculated per sec.
 */
BENCHMARK(HASH_Sha3_256_with_keccak);
BENCHMARK(HASH_Sha3_512_with_keccak);

BENCHMARK_MAIN();
