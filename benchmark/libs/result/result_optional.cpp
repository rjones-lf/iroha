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

#include <benchmark/benchmark.h>
#include <common/result.hpp>
#include <string>
using namespace iroha::result;
using namespace std::literals::string_literals;

static void BM_StringCreation(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::string empty_string;
  }
}

static void BM_StringCopy(benchmark::State& state) {
  std::string x = "hello";
  while (state.KeepRunning()) {
    std::string copy(x);
  }
}

static void BV_ResultCreation1(benchmark::State& state) {
  while (state.KeepRunning()) {
    result<std::string, std::string> r = Ok("hello"s);
  }
}

static void BV_ResultCreation2(benchmark::State& state) {
  while (state.KeepRunning()) {
    result<std::string, std::string> r = "hello"s;
  }
}

static void BM_ResultCopy(benchmark::State& state) {
  result<std::string, std::string> r = Ok("hello"s);
  while (state.KeepRunning()) {
    result<std::string, std::string> q(r);
  }
}

static void BM_ResultIfHandling(benchmark::State& state) {
  result<std::string, std::string> r = Ok("hello"s);
  while (state.KeepRunning()) {
    // 20% chance
    if (rand() % 5 == 0) {
      r = Error("hello"s);
    } else {
      r = Ok("hello"s);
    }

    if (r) {
      benchmark::DoNotOptimize(r.ok());
    } else {
      benchmark::DoNotOptimize(r.error());
    }
  }
}

static void BM_ResultMatchHandling(benchmark::State& state) {
  result<std::string, std::string> r = Ok("hello"s);
  while (state.KeepRunning()) {
    // 20% chance
    if (rand() % 5 == 0) {
      r = Error("hello"s);
    } else {
      r = Ok("hello"s);
    }

    r.match<void>(
        [](Ok_t<std::string> const& e) { benchmark::DoNotOptimize(e.value); },
        [](Error_t<std::string> const& e) {
          benchmark::DoNotOptimize(e.reason);
        });
  }
}

static void BM_AlwaysThrow(benchmark::State& state) {
  while (state.KeepRunning()) {
    try {
      throw 1;
    } catch (int) {
    }
  }
}

static void BM_NeverThrow(benchmark::State& state) {
  while (state.KeepRunning()) {
    try {
    } catch (int) {
    }
  }
}

BENCHMARK(BM_StringCreation);
BENCHMARK(BM_StringCopy);
BENCHMARK(BV_ResultCreation1);
BENCHMARK(BV_ResultCreation2);
BENCHMARK(BM_ResultCopy);

BENCHMARK(BM_ResultIfHandling);
BENCHMARK(BM_ResultMatchHandling);

BENCHMARK(BM_AlwaysThrow);
BENCHMARK(BM_NeverThrow);

BENCHMARK_MAIN();
