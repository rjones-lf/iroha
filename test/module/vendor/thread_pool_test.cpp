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

#include <cassert>
#include <fixed_function.hpp>
#include <functional>
#include <mpsc_bounded_queue.hpp>
#include <mutex>
#include <queue>
#include <string>
#include <thread_pool.hpp>
#include <type_traits>
#include <worker.hpp>

//-----------------------------------------------------------------------------
int test_free_func(int i) { return i; }

template <typename T>
T test_free_func_template(T p) {
  return p;
}

void test_void(int &p, int v) { p = v; }

struct A {
  int b(const int &p) { return p; }
  void c(int &i) { i = 43; }
};

template <typename T>
struct Foo {
  template <typename U>
  U bar(U p) {
    return p + payload;
  }

  T payload;
};

static std::string str_fun() { return "123"; }

//-----------------------------------------------------------------------------
TEST(FixedFunction, AllocDealloc) {
  static size_t def = 0;
  static size_t cop = 0;
  static size_t mov = 0;
  static size_t cop_ass = 0;
  static size_t mov_ass = 0;
  static size_t destroyed = 0;

  // STRUCT
  struct cnt {
    std::string payload;
    cnt() { def++; }
    cnt(const cnt &o) {
      payload = o.payload;
      cop++;
    }
    cnt(cnt &&o) {
      payload = std::move(o.payload);
      mov++;
    }
    cnt &operator=(const cnt &o) {
      payload = o.payload;
      cop_ass++;
      return *this;
    }
    cnt &operator=(cnt &&o) {
      payload = std::move(o.payload);
      mov_ass++;
      return *this;
    }
    ~cnt() { destroyed++; }
    std::string operator()() { return payload; }
  };

  {
    cnt c1;
    c1.payload = "xyz";
    FixedFunction<std::string()> f1(c1);
    ASSERT_STREQ(std::string("xyz").c_str(), f1().c_str());

    FixedFunction<std::string()> f2;
    f2 = std::move(f1);
    ASSERT_STREQ(std::string("xyz").c_str(), f2().c_str());

    FixedFunction<std::string()> f3(std::move(f2));
    ASSERT_STREQ(std::string("xyz").c_str(), f3().c_str());

    FixedFunction<std::string()> f4(str_fun);
    ASSERT_STREQ(std::string("123").c_str(), f4().c_str());

    f4 = std::move(f3);
    ASSERT_STREQ(std::string("xyz").c_str(), f4().c_str());

    cnt c2;
    c2.payload = "qwe";
    f4 = std::move(FixedFunction<std::string()>(c2));
    ASSERT_STREQ(std::string("qwe").c_str(), f4().c_str());
  }

  ASSERT_EQ((unsigned int)(def + cop + mov), destroyed);
  ASSERT_EQ(2u, def);
  ASSERT_EQ(0u, cop);
  ASSERT_EQ(6u, mov);
  ASSERT_EQ(0u, cop_ass);
  ASSERT_EQ(0u, mov_ass);
}

//-----------------------------------------------------------------------------
TEST(FixedFunction, FreeFunc) {
  FixedFunction<int(int)> f(test_free_func);
  ASSERT_EQ(3, f(3));
}

//-----------------------------------------------------------------------------
TEST(FixedFunction, FreeFuncTemplate) {
  FixedFunction<std::string(std::string)> f(
      test_free_func_template<std::string>);
  ASSERT_STREQ(std::string("abc").c_str(), f("abc").c_str());
}

//-----------------------------------------------------------------------------
TEST(FixedFunction, VoidFunc) {
  FixedFunction<void(int &, int)> f(test_void);
  int p = 0;
  f(p, 42);
  ASSERT_EQ(42, p);
}

//-----------------------------------------------------------------------------
TEST(FixedFunction, ClassMethodVoid) {
  using namespace std::placeholders;
  A a;
  int i = 0;
  FixedFunction<void(int &)> f(std::bind(&A::c, &a, _1));
  f(i);
  ASSERT_EQ(43, i);
}

//-----------------------------------------------------------------------------
TEST(FixedFunction, ClassMethod1) {
  using namespace std::placeholders;
  A a;
  FixedFunction<int(const int &)> f(std::bind(&A::b, &a, _1));
  ASSERT_EQ(4, f(4));
}

//-----------------------------------------------------------------------------
TEST(FixedFunction, ClassMethod2) {
  using namespace std::placeholders;
  Foo<float> foo;
  foo.payload = 1.f;
  FixedFunction<int(int)> f(std::bind(&Foo<float>::bar<int>, &foo, _1));
  ASSERT_EQ(2, f(1));
}

//-----------------------------------------------------------------------------
TEST(FixedFunction, Lambda) {
  const std::string s1 = "s1";
  FixedFunction<std::string()> f([&s1]() { return s1; });

  ASSERT_STREQ(s1.c_str(), f().c_str());
}

//-----------------------------------------------------------------------------
TEST(ThreadPool, PostJob) {
  ThreadPool pool;

  std::packaged_task<int()> t([]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return 42;
  });

  std::future<int> r = t.get_future();

  pool.post(t);

  ASSERT_EQ(42, r.get());
}

//-----------------------------------------------------------------------------
TEST(ThreadPool, ProcessJob) {
  ThreadPool pool;

  std::future<int> r = pool.process([]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return 42;
  });

  ASSERT_EQ(42, r.get());
}

//-----------------------------------------------------------------------------
TEST(ThreadPool, ProcessJobWithException) {
  ThreadPool pool;

  struct CustomException {};

  std::future<int> r = pool.process([]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    throw CustomException();
    return 42;
  });

  ASSERT_THROW(r.get(), CustomException);
}

//-----------------------------------------------------------------------------
TEST(ThreadPool, Parallel_HeavyJob) {
  ThreadPool pool;  // by default uses N_threads = 'hardware_concurrency'

  // in this test we add jobs faster than process them
  const int repetitions = (int)100;

  // 128 = queue size, must be power of 2
  MPMCBoundedQueue<std::future<int>> queue(128);

  // producer thread
  std::thread([&]() {
    // we calculate sum([1..repetitions])
    for (int i = 1; i <= repetitions; i++) {
      auto &&f = pool.process([i]() {
        // stateless processing: process one request 20 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return i;
      });

      queue.push(std::move(f));

      // new job arrives every 5 ms
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }).detach();

  // consumer thread (this)
  // lets agregate return values and calculate sum([1..repetitions])
  int sum_actual = 0;
  for (int handled = 1; handled <= repetitions; handled++) {
    std::future<int> result;
    while (!queue.pop(result)) {
      // empty queue, wait 10 ms
      // TODO: there should be better algorithm than waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // stateful processing
    sum_actual += result.get();
  }

  auto sum = [](int n) -> int { return n * (n + 1) / 2; };

  // NOTE!!!
  // total time = repetitions * "processing time" / N_cores + <overhead>
  // if and only if "processing time" > "job arrival time"
  ASSERT_EQ(sum_actual, sum(repetitions));
}

//-----------------------------------------------------------------------------
TEST(ThreadPool, Sequential_WithStdQueue) {
  // add N jobs to the queue, and wait until all will be completed

  ThreadPool pool;

  std::queue<std::future<int>> queue;

  const int repetitions = 100;

  for (int i = 1; i <= repetitions; i++) {
    queue.push(std::move(pool.process([i]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      return i;
    })));
  }

  int sum_actual = 0;
  while (!queue.empty()) {
    sum_actual += queue.front().get();
    queue.pop();
  }

  auto sum = [](int n) -> int { return n * (n + 1) / 2; };

  ASSERT_EQ(sum_actual, sum(repetitions));
}

//-----------------------------------------------------------------------------
TEST(ThreadPool, Sequential_WithMPSCBoundedQueue) {
  // add N jobs to the queue, and wait until all will be completed
  // this test should be faster than Sequential_WithStdQueue
  ThreadPool pool;

  MPMCBoundedQueue<std::future<int>> queue(128);

  const int repetitions = 100;

  for (int i = 1; i <= repetitions; i++) {
    queue.push(std::move(pool.process([i]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      return i;
    })));
  }

  int sum_actual = 0;
  std::future<int> item;
  while (queue.pop(item)) {
    sum_actual += item.get();
  }

  auto sum = [](int n) -> int { return n * (n + 1) / 2; };

  ASSERT_EQ(sum_actual, sum(repetitions));
}

//-----------------------------------------------------------------------------
