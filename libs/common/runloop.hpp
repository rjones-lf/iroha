/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_RUNLOOP_HPP
#define IROHA_RUNLOOP_HPP

#include <rxcpp/schedulers/rx-runloop.hpp>

/*
 * This file contains modified copies of run_loop_state and run_loop objects
 * from rxcpp.
 *
 * The method `dispatch` of `run_loop` would not immediately return with the
 * applied patch. The method will pause execution on an additional cond var till
 * some task will be scheduled for execution.
 *
 * That fix solves the problem of busy waiting when we call dispatch inside a
 * loop.
 *
 * Applied modifications:
 * - conditional variable `wake` was added to `run_loop_state`
 * - `wake.wait` and `wake.wait_until` were added to `run_loop.dispatch` to
 * pause execution
 * - `wake.notify_one` was added to `run_loop.schedule`
 */

namespace iroha {

  namespace detail {

    struct run_loop_state
        : public std::enable_shared_from_this<run_loop_state> {
      typedef rxcpp::schedulers::scheduler::clock_type clock_type;

      typedef rxcpp::schedulers::detail::schedulable_queue<
          clock_type::time_point>
          queue_item_time;

      typedef queue_item_time::item_type item_type;
      typedef queue_item_time::const_reference const_reference_item_type;

      virtual ~run_loop_state() {}

      run_loop_state() {}

      rxcpp::composite_subscription lifetime;
      mutable std::mutex lock;
      mutable std::condition_variable wake;
      mutable queue_item_time q;
      rxcpp::schedulers::recursion r;
      std::function<void(clock_type::time_point)> notify_earlier_wakeup;
    };

  }  // namespace detail

  struct run_loop_scheduler : public rxcpp::schedulers::scheduler_interface {
   private:
    typedef run_loop_scheduler this_type;
    run_loop_scheduler(const this_type &);

    struct run_loop_worker : public rxcpp::schedulers::worker_interface {
     private:
      typedef run_loop_worker this_type;

      run_loop_worker(const this_type &);

     public:
      std::weak_ptr<detail::run_loop_state> state;

      virtual ~run_loop_worker() {}

      explicit run_loop_worker(std::weak_ptr<detail::run_loop_state> ws)
          : state(ws) {}

      virtual clock_type::time_point now() const {
        return clock_type::now();
      }

      virtual void schedule(const rxcpp::schedulers::schedulable &scbl) const {
        schedule(now(), scbl);
      }

      virtual void schedule(clock_type::time_point when,
                            const rxcpp::schedulers::schedulable &scbl) const {
        if (scbl.is_subscribed()) {
          auto st = state.lock();
          std::unique_lock<std::mutex> guard(st->lock);
          const bool need_earlier_wakeup_notification =
              st->notify_earlier_wakeup
              && (st->q.empty() || when < st->q.top().when);
          st->q.push(detail::run_loop_state::item_type(when, scbl));
          st->r.reset(false);
          if (need_earlier_wakeup_notification)
            st->notify_earlier_wakeup(when);
          guard.unlock();  // So we can't get attempt to recursively lock the
                           // state
        }
        if (auto st = state.lock()) {
          st->wake.notify_one();
        }
      }
    };

    std::weak_ptr<detail::run_loop_state> state;

   public:
    explicit run_loop_scheduler(std::weak_ptr<detail::run_loop_state> ws)
        : state(ws) {}
    virtual ~run_loop_scheduler() {}

    virtual clock_type::time_point now() const {
      return clock_type::now();
    }

    virtual rxcpp::schedulers::worker create_worker(
        rxcpp::composite_subscription cs) const {
      auto lifetime = state.lock()->lifetime;
      auto token = lifetime.add(cs);
      cs.add([=]() { lifetime.remove(token); });
      return rxcpp::schedulers::worker(cs, create_worker_interface());
    }

    std::shared_ptr<rxcpp::schedulers::worker_interface>
    create_worker_interface() const {
      return std::make_shared<run_loop_worker>(state);
    }
  };

  class run_loop {
   private:
    typedef run_loop this_type;
    // don't allow this instance to copy/move since it owns current_thread
    // queue for the thread it is constructed on.
    run_loop(const this_type &);
    run_loop(this_type &&);

    typedef rxcpp::schedulers::detail::action_queue queue_type;

    typedef detail::run_loop_state::item_type item_type;
    typedef detail::run_loop_state::const_reference_item_type
        const_reference_item_type;

    std::shared_ptr<detail::run_loop_state> state;
    std::shared_ptr<run_loop_scheduler> sc;

   public:
    typedef rxcpp::schedulers::scheduler::clock_type clock_type;
    run_loop()
        : state(std::make_shared<detail::run_loop_state>()),
          sc(std::make_shared<run_loop_scheduler>(state)) {
      // take ownership so that the current_thread scheduler
      // uses the same queue on this thread
      queue_type::ensure(sc->create_worker_interface());
    }
    ~run_loop() {
      state->lifetime.unsubscribe();

      std::unique_lock<std::mutex> guard(state->lock);

      // release ownership
      queue_type::destroy();

      auto expired = std::move(state->q);
      if (!state->q.empty())
        std::terminate();
    }

    clock_type::time_point now() const {
      return clock_type::now();
    }

    rxcpp::composite_subscription get_subscription() const {
      return state->lifetime;
    }

    bool empty() const {
      return state->q.empty();
    }

    const_reference_item_type peek() const {
      return state->q.top();
    }

    void dispatch() const {
      std::unique_lock<std::mutex> guard(state->lock);
      if (state->q.empty()) {
        state->wake.wait(guard, [this] {
          return !state->lifetime.is_subscribed() || !state->q.empty();
        });
      }
      auto &peek = state->q.top();
      if (!peek.what.is_subscribed()) {
        state->q.pop();
        return;
      }
      if (clock_type::now() < peek.when) {
        state->wake.wait_until(guard, peek.when);
        return;
      }
      auto what = peek.what;
      state->q.pop();
      state->r.reset(state->q.empty());
      guard.unlock();
      what(state->r.get_recurse());
    }

    rxcpp::schedulers::scheduler get_scheduler() const {
      return rxcpp::schedulers::make_scheduler(sc);
    }

    void set_notify_earlier_wakeup(
        std::function<void(clock_type::time_point)> const &f) {
      std::unique_lock<std::mutex> guard(state->lock);
      state->notify_earlier_wakeup = f;
    }
  };

  inline rxcpp::schedulers::scheduler make_run_loop(const run_loop &r) {
    return r.get_scheduler();
  }

}  // namespace iroha

#endif  // IROHA_RUNLOOP_HPP
