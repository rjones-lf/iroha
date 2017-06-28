/*
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dao/dao.hpp>
#include <network/network_api.h>
#include <consensus/sumeragi.hpp>

/**
 * |ーーー|　|ーーー|　|ーーー|　|ーーー|
 * |　ス　|ー|　メ　|ー|　ラ　|ー|　ギ　|
 * |ーーー|　|ーーー|　|ーーー|　|ーーー|
 *
 * A chain-based byzantine fault tolerant consensus algorithm, based in large
 * part on BChain:
 *
 * Duan, S., Meling, H., Peisert, S., & Zhang, H. (2014). Bchain: Byzantine
 * replication with high throughput and embedded reconfiguration. In
 * International Conference on Principles of Distributed Systems (pp. 91-106).
 * Springer.
 */

namespace consensus {

  /**
   * Event is triggered when proposal arrives.
   */
  void Sumeragi::onProposal(iroha::dao::Proposal const &proposal) {

  }

  /**
   * Event is triggered when commit block arrives.
   */
  void Sumeragi::onCommit(iroha::dao::Block const &block) {
    
  }
}  // namespace consensus
