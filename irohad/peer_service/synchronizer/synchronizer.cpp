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

//
// Created by Takumi Yamashita on 2017/04/28.
//

#include <map_queue/map_queue.hpp>
#include <peer_service/monitor.hpp>
#include <peer_service/self_state.hpp>
#include <peer_service/synchronizer/synchronizer.hpp>
#include <timer/timer.hpp>

//#include <ametsuchi/ametsuchi.hpp>

namespace peer_service {
  namespace sync {
    std::shared_ptr<Node> leader;

    bool start() {
      ::peer_service::self_state::stop();

      std::string default_leader_ip =
          ::peer_service::monitor::getCurrentLeaderIp();

      // TODO [WIP] search default_leader in all peerList
      if (default_leader_ip == ::peer_service::self_state::getIp())
        return false;

      // TODO connection Test Ping default_leader_ip
      // TODO to issue addPeer(myself) Transaction to default_leader_ip

      // TODO start streaming connect to default_leader_ip

      {  // TOOD another thread.
        detail::appending();
      }
      return true;
    }

    // TODO continue implement after complete ametsuchi
    bool trigger(const Block &block) {
      if (::peer_service::self_state::getState() == PREPARE) {
        // check Statefull validate Block
        if (true && "WIP") {
          ::peer_service::self_state::activate();

          // TODO ametsuchi
          //          ::ametsuchi::append(block);
          //          ::ametsuchi::commit();

          detail::clearCache();
        }
        return true;
      }
      return false;
    }

    namespace detail {

      structure::MapQueue<uint64_t, Block> temp_block_;
      uint64_t current_;
      uint64_t upd_time_;

      bool append_temporary(uint64_t tx_id, const Block &&tx) {
        temp_block_.set(tx_id, std::move(tx));
        return true;
      }

      SYNCHRO_RESULT append() {
        while (temp_block_.exists(current_)) {
          auto &ap_tx = temp_block_[current_];
          // ametsuchi::append(ap_tx);
          current_++;
          upd_time_ = iroha::time::now64();
        }

        if (!temp_block_.empty() &&
            upd_time_ < (uint64_t)-1) {  // if started downlaoding
          // if elapsed that tiem is updated more than 2 sec and cache has more
          // than index tx.
          if (iroha::time::now64() - upd_time_ > 2 &&
              temp_block_.getMaxKey() > current_)
            return SYNCHRO_RESULT::APPEND_ERROR;
        }

        return SYNCHRO_RESULT::APPEND_ONGOING;
      }

      void appending() {
        upd_time_ = (uint64_t)-1;
        clearCache();

        while (::peer_service::self_state::getState() == PREPARE) {
          timer::waitTimer(100);
          switch (append()) {
            case SYNCHRO_RESULT::APPEND_ERROR:
              return;
            case SYNCHRO_RESULT::APPEND_FINISHED:
              return;
            case SYNCHRO_RESULT::APPEND_ONGOING:
              break;
          }
        }
      }
      void clearCache() {
        current_ = 0;
        temp_block_.clear();
      }
    }  // namespace datail

  }  // namespace sync
}  // namespace peer