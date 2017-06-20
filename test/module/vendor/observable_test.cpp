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

#include <iostream>
#include <observable/observable.hpp>

using namespace std;
using namespace observable;

TEST(observable_test, usage_observable_test) {
  auto sub = subject<void(string)> {};
  sub.subscribe([](auto const &msg) { cout << msg << endl; });

  // "Hello world!" will be printed on stdout.
  sub.notify("Hello world!");

  auto a = value<int> {5};
  auto b = value<int> {5};
  auto avg = observe(
      (a + b) / 2.0f
  );
  auto eq_msg = observe(
      select(a == b, "equal", "not equal")
  );

  avg.subscribe([](auto val) { cout << val << endl; });
  eq_msg.subscribe([](auto const &msg) { cout << msg << endl; });

  // "10" and "not equal" will be printed on stdout in an
  // unspecified order.
  b = 15;
}