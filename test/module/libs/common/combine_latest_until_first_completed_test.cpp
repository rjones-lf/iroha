/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/combine_latest_until_first_completed.hpp"

#include <gtest/gtest.h>
#include <chrono>

#include <boost/variant.hpp>
#include "common/visitor.hpp"
#include "rxcpp/rx.hpp"

using namespace rxcpp;
using namespace rxcpp::sources;
using namespace rxcpp::subjects;
using namespace rxcpp::util;
using namespace std::literals::chrono_literals;
using std::chrono::steady_clock;

using time_point = steady_clock::time_point;
using duration = std::chrono::milliseconds;

struct IntervalObservebleHelper {
  IntervalObservebleHelper(duration period)
      : IntervalObservebleHelper(steady_clock::now(), period) {}

  IntervalObservebleHelper(time_point start, duration period)
      : observable_(rxcpp::observable<>::interval(start, period)),
        start_(start),
        period_(period) {}

  long valueAtTime(time_point time) const {
    return 1 + (time - start_) / period_;
  }

  long lastValue() const {
    //return valueAtTime(steady_clock::now());
    return last_emitted_value;
  }

  virtual rxcpp::observable<long> getObservable() {
    return rememberLastVal(observable_);
  }

  rxcpp::observable<long> rememberLastVal(rxcpp::observable<long> observable) {
    return observable.tap([this](long val) { this->last_emitted_value = val; });
  }

  rxcpp::observable<long> observable_;
  time_point start_;
  duration period_;
  long last_emitted_value{0};
};

struct IntervalObservebleTakeNHelper : public IntervalObservebleHelper {
  IntervalObservebleTakeNHelper(duration period, size_t taken)
      : IntervalObservebleHelper(period), taken_(taken) {}

  IntervalObservebleTakeNHelper(time_point start, duration period, size_t taken)
      : IntervalObservebleHelper(start, period), taken_(taken) {}

  rxcpp::observable<long> getObservable() override {
    return rememberLastVal(observable_.take(taken_));
  }

  size_t taken_;
};

using IntervalObservebleHelperVariant =
    boost::variant<IntervalObservebleHelper, IntervalObservebleTakeNHelper>;

template <typename... Types>
void checkValues(
    const std::vector<IntervalObservebleHelperVariant> &observable_helpers,
    size_t current_observable_idx,
    bool &completed,
    long current_test_value,
    Types... args) {
  if (current_observable_idx == 0) {
    EXPECT_FALSE(completed) << "Got values when already completed!";
  }
  checkValues(observable_helpers,
              current_observable_idx,
              completed,
              current_test_value);
  checkValues(
      observable_helpers, current_observable_idx + 1, completed, args...);
}

template <>
void checkValues(
    const std::vector<IntervalObservebleHelperVariant> &observable_helpers,
    size_t current_observable_idx,
    bool &completed,
    long current_test_value) {
  auto last_value =
      iroha::visit_in_place(observable_helpers[current_observable_idx],
                            [](const auto &observable_helper) {
                              return observable_helper.lastValue();
                            });
  EXPECT_EQ(last_value, current_test_value)
      << "Got wrong value from observable #" << current_observable_idx;

  iroha::visit_in_place(
      observable_helpers[current_observable_idx],
      [=, &completed](const IntervalObservebleTakeNHelper &observable_helper) {
        const long completion_value = observable_helper.taken_;
        EXPECT_LE(last_value, completion_value)
            << "Got value after the last one from observable #"
            << current_observable_idx;
        completed |= completion_value <= last_value;
      },
      [](const auto &) {});
}

auto makeValuesChecker(
    const std::vector<IntervalObservebleHelperVariant> &observable_helpers) {
  return [&observable_helpers](auto... values) {
    bool completed;
    checkValues(observable_helpers, 0, completed, values...);
    return completed;
  };
}

template <class T>
rxcpp::observable<long> getObservable(T &observable_helper) {
  return iroha::visit_in_place(observable_helper, [](auto &observable_helper) {
    return observable_helper.getObservable();
  });
}

/**
 * Checks that correct values are emitted and that observing stops as soon as
 * any of them completes.
 * @return whether the final value of any source observable was reached
 */
bool check(rxcpp::observable<bool> observable) {
  bool completed;
  observable.last().as_blocking().subscribe(
      [&completed](bool val) { completed = val; },
      [](std::exception_ptr ep) {
        try {
          std::rethrow_exception(ep);
        } catch (const std::exception &e) {
          FAIL() << "OnError: " << e.what();
        }
      });
  return completed;
}

/**
 * @given three observables
 * @when they are combined in parallel
 * @then the returned values are correct
 * @and the pipeline stops as soon as the first observable stops.
 */
TEST(combineLatestUntilFirstCompleted, ThreeParallel) {
  std::vector<IntervalObservebleHelperVariant> observable_helpers{
      IntervalObservebleHelper(17ms),
      IntervalObservebleHelper(23ms),
      IntervalObservebleTakeNHelper(29ms, 10)};

  auto values = iroha::makeCombineLatestUntilFirstCompleted(
      getObservable(observable_helpers[0]),
      rxcpp::identity_current_thread(),
      makeValuesChecker(observable_helpers),
      getObservable(observable_helpers[1]),
      getObservable(observable_helpers[2]));

  bool completed = check(values);
  EXPECT_TRUE(completed) << "Did not reach completed state!";
}

/**
 * @given four observables
 * @when they are combined in parallel
 * @then the returned values are correct
 * @and the pipeline stops as soon as the first observable stops.
 */
TEST(combineLatestUntilFirstCompleted, FourParallel) {
  std::vector<IntervalObservebleHelperVariant> observable_helpers{
      IntervalObservebleTakeNHelper(13ms, 15),
      IntervalObservebleHelper(17ms),
      IntervalObservebleHelper(23ms),
      IntervalObservebleTakeNHelper(29ms, 10)};

  auto values = iroha::makeCombineLatestUntilFirstCompleted(
      getObservable(observable_helpers[0]),
      rxcpp::identity_current_thread(),
      makeValuesChecker(observable_helpers),
      getObservable(observable_helpers[1]),
      getObservable(observable_helpers[2]));

  bool completed = check(values);
  EXPECT_TRUE(completed) << "Did not reach completed state!";
}

/**
 * @given four observables
 * @when they are combined in pairs and then these pairs are combined
 * @then the returned values are correct
 * @and the whole pipeline stops as soon as the first observable stops.
 */
TEST(combineLatestUntilFirstCompleted, TwoPairsParallel) {
  std::vector<IntervalObservebleHelperVariant> observable_helpers{
      IntervalObservebleTakeNHelper(13ms, 15),
      IntervalObservebleHelper(17ms),
      IntervalObservebleHelper(23ms),
      IntervalObservebleTakeNHelper(29ms, 10)};

  auto pass_array = [](long a, long b) { return std::array<long, 2>{{a, b}}; };

  auto pair1 = iroha::makeCombineLatestUntilFirstCompleted(
      getObservable(observable_helpers[0]),
      rxcpp::identity_current_thread(),
      pass_array,
      getObservable(observable_helpers[1]));

  auto pair2 = iroha::makeCombineLatestUntilFirstCompleted(
      getObservable(observable_helpers[2]),
      rxcpp::identity_current_thread(),
      pass_array,
      getObservable(observable_helpers[3]));

  auto values = iroha::makeCombineLatestUntilFirstCompleted(
      pair1,
      rxcpp::identity_current_thread(),
      [checker = makeValuesChecker(observable_helpers)](auto pair1,
                                                        auto pair2) {
        return checker(pair1[0], pair1[1], pair2[0], pair2[1]);
      },
      pair2);

  check(values);
}
