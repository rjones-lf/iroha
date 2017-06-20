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

#include <ametsuchi/index/backend/redis.hpp>

namespace iroha {

  namespace ametsuchi {

    namespace index {

      Redis::Redis(const std::string &host, std::size_t port)
          : host_(host), port_(port) {
        client_.connect(host_, port_);
        read_client_.connect(host_, port_);
        start_multi();
      }

      Redis::~Redis() {
        client_.disconnect();
        read_client_.disconnect();
      }

      bool Redis::add_blockhash_blockid(std::string block_hash,
                                        uint32_t height) {
        bool res;
        client_.set("block:" + block_hash, std::to_string(height),
                    [&res](cpp_redis::reply &reply) { res = reply.ok(); });
        client_.set("last_id", std::to_string(height),
                    [&res](cpp_redis::reply &reply) { res &= reply.ok(); });
        client_.sync_commit();
        return res;
      }

      bool Redis::add_pubkey_txhash(std::string pubkey, std::string txhash) {
        bool res;
        std::vector<std::string> txhashes(
            {txhash});  // cpp redis requires to put vector into rpush
        client_.rpush("account_pubkey:" + pubkey, txhashes,
                      [&res](cpp_redis::reply &reply) { res = reply.ok(); });
        client_.sync_commit();
        return res;
      }

      nonstd::optional<uint64_t> Redis::get_blockid_by_blockhash(
          std::string hash) {
        nonstd::optional<uint64_t> res;
        read_client_.get("block:" + hash, [&res](cpp_redis::reply &reply) {
          if (reply.ok() && reply.is_string()) {
            res = std::stoul(reply.as_string());
          }
        });
        read_client_.sync_commit();
        return res;
      }

      bool Redis::add_txhash_blockid_txid(std::string txhash, uint32_t height,
                                          int txid) {
        bool res = _add_txhash_blockid_txid(txhash, height, txid);
        client_.sync_commit();
        return res;
      }

      nonstd::optional<uint64_t> Redis::get_txid_by_txhash(std::string txhash) {
        nonstd::optional<uint64_t> res;
        read_client_.hget("tx:" + txhash, "txid",
                          [&res](cpp_redis::reply &reply) {
                            if (reply.ok() && reply.is_string()) {
                              res = std::stoul(reply.as_string());
                            }
                          });
        read_client_.sync_commit();
        return res;
      }

      nonstd::optional<uint64_t> Redis::get_blockid_by_txhash(
          std::string txhash) {
        nonstd::optional<uint64_t> res;
        read_client_.hget("tx:" + txhash, "blockid",
                          [&res](cpp_redis::reply &reply) {
                            if (reply.ok() && reply.is_string()) {
                              res = std::stoul(reply.as_string());
                            }
                          });
        read_client_.sync_commit();
        return res;
      }

      nonstd::optional<std::vector<std::string>> Redis::get_txhashes_by_pubkey(
          std::string pubkey) {
        nonstd::optional<std::vector<std::string>> res;
        read_client_.lrange("account_pubkey:" + pubkey, 0, -1,
                            [&res](cpp_redis::reply &reply) {
                              if (reply.ok() && reply.is_array()) {
                                auto replies = reply.as_array();
                                res = std::vector<std::string>();
                                for (const auto &one_reply : replies) {
                                  res->push_back(one_reply.as_string());
                                }
                              }
                            });
        read_client_.sync_commit();
        return res;
      }

      nonstd::optional<uint64_t> Redis::get_last_blockid() {
        nonstd::optional<uint64_t> res;
        read_client_.get("last_id", [&res](cpp_redis::reply &reply) {
          res = std::stoul(reply.as_string());
        });
        read_client_.sync_commit();
        return res;
      }

      bool Redis::exec_multi() {
        bool res;
        client_.exec([&res](cpp_redis::reply &reply) { res = reply.ok(); });
        client_.sync_commit();
        return res && start_multi();
      }

      bool Redis::discard_multi() {
        bool res;
        client_.discard([&res](cpp_redis::reply &reply) { res = reply.ok(); });
        client_.sync_commit();
        return res && start_multi();
      }

      bool Redis::_add_txhash_blockid_txid(std::string txhash, uint32_t height,
                                           int txid) {
        bool res;
        client_.hset("tx:" + txhash, "blockid", std::to_string(height),
                     [&res](cpp_redis::reply &reply) { res = reply.ok(); });
        client_.hset("tx:" + txhash, "txid", std::to_string(txid),
                     [&res](cpp_redis::reply &reply) { res &= reply.ok(); });
        return true;
      }

      bool Redis::start_multi() {
        bool res;
        client_.multi([&res](cpp_redis::reply &reply) { res = reply.ok(); });
        client_.sync_commit();
        return res;
      }
    }  // namespace index

  }  // namespace ametsuchi
}  // namespace iroha