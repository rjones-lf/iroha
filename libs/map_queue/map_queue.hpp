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

#ifndef IROHA_MAP_QUEUE_HPP
#define IROHA_MAP_QUEUE_HPP

#include <deque>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

namespace structure {

// MapQueue
/*
 * MapQueue is a queue of key-value pairs, that allows the highest key to be found.
 * MapQueue does not call erase method from external.
 * MapQueue does not overwrite.
 * MapQueue autoerases element that was last push, when over size.
 * MapQueue operate below all function, amortized complexity O(1) (clear is
 * not).
 */
template <typename Key, typename Value>
class MapQueue {
 private:
  size_t max_cache_size_;
  std::unordered_map<Key, Value> data_;
  std::deque<Key> max_cache_;
  std::deque<Key> cache_;

  // erase last push node
  size_t pop_last() {
    if (data_.empty()) return data_.size();
    Key& k = cache_.front();
    cache_.pop_front();
    if (max_cache_.front() == k) max_cache_.pop_front();
    data_.erase(k);
    return data_.size();
  }

 public:
  MapQueue(size_t max_cache_size = 1) : max_cache_size_(max_cache_size) {}
  ~MapQueue() { clear(); }

  void debug_view(){
    std::cout << "debug view" << std::endl;
    for( auto it = cache_.begin(); it != cache_.end(); it++)
      std::cout << *it << " ";
    std::cout << std::endl;

    for( auto it = max_cache_.begin(); it != max_cache_.end(); it++)
      std::cout << *it << " ";
    std::cout << std::endl;

    for( auto it = data_.begin(); it != data_.end(); it++)
      std::cout << it->first << ", " << it->second << "  ";
    std::cout << std::endl;

  }

  // set max_chache_size
  void set_cache_size(size_t max_cache_size) {
    max_cache_size_ = max_cache_size;
    while (data_.size() > max_cache_size_) {
      pop_last();
    }
  }


  // set key and value
  size_t set(const Key& k, const Value&& v) {
    if( data_.count(k) ) return data_.size();
    cache_.push_back(k);
    while (!max_cache_.empty() && max_cache_.back() < k) max_cache_.pop_back();
    max_cache_.push_back(k);
    data_[k] = std::move(v);
    return data_.size();
  }
  size_t set(const Key& k, const Value& v) {
    return set(k,std::move(v));
  }

  // [] oprator
  const Value& operator[](const Key& k) {
    if (data_.count(k) == 0) throw std::out_of_range("cache_map");
    return data_[k];
  }

  const Value& operator[](Key&& k) {
    if (data_.count(k) == 0) throw std::out_of_range("cache_map");
    return data_[k];
  }


  // get maximum key
  const Key& getMaxKey() const {
    if( max_cache_.empty() ) throw std::out_of_range("cache_map");
    return max_cache_.front();
  }

  size_t max_size() const noexcept { return max_cache_size_; }
  size_t size() const noexcept { return data_.size(); }
  bool empty() const noexcept { return data_.empty(); }
  size_t exists(const Key& k) const { return data_.count(k); }
  void clear() noexcept {
    data_.clear();
    max_cache_.clear();
    cache_.clear();
  }
};
} // namespace structure

#endif  // IROHA_MAP_QUEUE_HPP
