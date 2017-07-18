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

#include "ordering/OrderingService.hpp"

namespace iroha {

  void iroha::OrderingService::create_proposal() {
    Proposal proposal;

    // while not empty AND TODO: the number of transactions inside < BLOCK_SIZE
    while (!queue_->empty()) {
      Transaction tx = queue_->front();
      queue_->pop();
      proposal.add_transactions(tx.SerializeAsString());
    }

    publish(proposal);
  }

  void OrderingService::bind_timer() {
    timer_ = loop_->resource<uvw::TimerHandle>();
    timer_->on<uvw::ErrorEvent>(
        [](const uvw::ErrorEvent &e, uvw::TimerHandle &handle) {
          throw "timer is dead";  // TODO
        });

    timer_->on<uvw::TimerEvent>([this](const uvw::TimerEvent &e,
                                              uvw::TimerHandle &handle) {
      // TODO
      if (!queue_->empty()) {
        console_->info("{} transactions in ordering service", queue_->size());
        create_proposal();
      } else {
        console_->info("No transactions in ordering service");
      }
    });
  }

  void OrderingService::run() {
    console_->info("started");
    timer_->start(uvw::TimerHandle::Time{0},
                  uvw::TimerHandle::Time{5000});  // repeated timer
  }

  void OrderingService::stop() { timer_->stop(); }

  // for debug
  void OrderingService::simulate_one() {
    Transaction tx;
    //tx.set_test(std::to_string(rand()));
    queue_->push(tx);
  }

  OrderingService::OrderingService(std::shared_ptr<uvw::Loop> loop)
      : loop_{loop} {
    queue_ = std::make_shared<std::queue<Transaction>>();
    console_ = spdlog::get("orderingService");
    if (!console_) {
      console_ = spdlog::stdout_color_st("orderingService");
    }
    bind_timer();
  }
}
