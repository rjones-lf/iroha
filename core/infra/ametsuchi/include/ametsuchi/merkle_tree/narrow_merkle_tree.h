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

#pragma once

#include <ametsuchi/merkle_tree/circular_stack.h>
#include <ametsuchi/exception.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <bitset>
#include <iostream>
#include <bitset>
#include <functional>

namespace ametsuchi {
namespace merkle {

  template <typename T>
  constexpr size_t popcount(T n) {
    //std::bitset<sizeof(T)> b(n);
    return std::bitset<sizeof(T)>(n).count();
  }

/**
 *   \brief Optimized merkle tree for append-only
 *   storage. With quick rollback property
 *
 *   As far as TXs in ledger cannot be undone
 *   except earlier than checkpoint thus we
 *   don't need to store entire tree and should
 *   only service capacity equals TXs to checkpoint
 *
 *                             _______[+]_________
 *                            /                   \
 *                           /                     \
 *                  ______[*]______                 [+]
 *                 /               \                 |
 *                /                 \                |
 *               /                   \               |
 *          __[-]__                 __[*]__         [+]
 *         /       \               /       \         |
 *      [^]         [^]         [-]         [*]     [+]
 *     /   \       /   \       /   \       /   \     |
 *   [^]   [^]   [^]   [^]   [^]   [^]   [-]   [*]  [t]
 *
 *   Current implementation stores `capacity`
 *   hash-paths marked as "*" and "-". In this example
 *   capacity equals 2. So rest blocks (maked as "^")
 *   doesn't stored and not wasting space.
 *
 *   After adding new hash of tx (marked as "t") merkle
 *   tree adds new hash-line marked as "+" and "t".
 *   Also it removes blocks marked with "-".
 *
 *   Tree stores $\sum^{capacity}_{n=0} (tx - n - 1)$
 *   elements where tx represents number tx added to
 *   the tree. Note that tree with capacity=0 still
 *   store $(tx - n - 1)$ hashes
 *  @tparam T type of the storing hash
 */
  template <typename T>
  class NarrowMerkleTree {
  public:
    // HashFn(t, T()) should be equal t
    using HashFn = std::function<T(const T &, const T &)>;
    using Storage = std::vector<buffer::CircularStack<T>>;
    NarrowMerkleTree(HashFn, size_t capacity = 2);

    /**
     * Perform appending to the tree, recalculating
     * root and dropping outdated hashes
     * O( log_capacity( txs ) )
     */
    void add(T);

    /**
     * Drops provided erase Tx's hash that has identify after num.
     * But num is maximum number that is lower than num and remain hash.
     * return value is num after above calculate.
     *
     * !!!!CAUTION!!!!! num must be greater than num that is previous called.
     * (for example : drop(5) and capacity() = 2
     * hash[ 0, 1, 2, 3, 4, 5, 6, 7, 8 ]
     * merkle tree :
     * 3: (0,1, 2,3,  4,5, 6,7)
     * 2: (0,1, 2,3) (4,5, 6,7)
     * 1:            (4,5)(6,7)
     * 0:                   (7)(8)
     * num = 5 -> 4
     * -> hash[ 0, 1, 2, 3 ]
     * return 4 )
     * O( log_capacity( txs ) )
     */
    size_t drop(size_t);

    /**
     * get Merkle Tree Infomation ( Use sarch broken point algorithm )
     */
    const Storage& merkle() const {
      return data;
    }

    /**
     * Calculate height of tree for `node`
     */
    size_t height(size_t n) {
      size_t num = 0;
      size_t cap = 1;
      while( n >= cap ){
        cap *= capacity();
        num++;
      }
      return num;
    }


    /**
     * Comparing common nodes on the path except
     * root between tx hash and subsequent tx hash
     */
    static inline size_t path_diff(size_t node) {
      // calculate difference of path from root
      // between `node` and `node + 1`
      auto x = node ^ (node + 1);
      // count number of leading zeros in x
      // in tree with height `node + 1`
      return floor(log2(node + 1)) - floor(log2(x));
    }

    inline size_t capacity() const { return capacity_; }

    /**
     * get root hash
     * in most case  : O(1)
     * in worst case : O( log_capacity(txs) )
     */
    inline T get_root() const;

    inline size_t size() const;

  private:
    Storage data;
    /**
     * Represents maximum number of tx hashes
     * that can be reseted w/o recalculation
     * fxd: moved to data[0].capacity()
     */
    size_t capacity_;
    /**
     * TXs' hashes added so far
     */
    size_t txs;
    /**
     * Function for parent vertices calculation
     * It need to exist associative law.
     * e.g ( hash( hash( a, b ), c ) = hash ( a, hash( b, c ) ) ).
     * It need to exist only identity element.
     * e.g ( hash( T(), a ) = hash( T(), a ) = a )
     */
    HashFn hash;

    /**
     * previous called drop's argument
     */
    size_t previous_drop_number;

    /**
     * Extends data by the default capacity
     */
    void grow() { data.emplace_back(capacity()); }

    /**
     * Extends data by capacity + 2. So it's able to
     * save one more element for rollback and one more
     * for internal usage (see `add`)
     */
    void grow(size_t capacity) { data.emplace_back(capacity); }
  };


  template <typename T>
  NarrowMerkleTree<T>::NarrowMerkleTree(HashFn c, size_t capacity)
      : capacity_(capacity), txs(0), hash(c), previous_drop_number(0) {
    if (capacity_ == 0) throw "capacity can't assign empty.";
    grow(capacity_);
  }

  template <typename T>
  void NarrowMerkleTree<T>::add(T t) {
    txs++;
    // Cumulative sum of Hash
    data[0].push( hash( get_root(), t ) );
    if (txs != 1 && height(txs) > height(txs - 1) && height(txs) > data.size() ) {
      grow();
    }

    // layer_idx is:(cap = 2)
    //        0
    //    0       1
    //  0   1   2
    // 0 1 2 3 4 5
    size_t layer_idx = txs-1;
    // This loop traverse up to the tree and update hash
    for( auto child = data.begin(), parent = child+1; parent != data.end();
         ++child, ++parent, layer_idx /= capacity() ) {
      // child extend and right most node
      if( layer_idx % capacity() == capacity()-1 ) {
        parent->push( child->back() );
      } else
        break;
    }
  }

  template <typename T>
  size_t NarrowMerkleTree<T>::drop(size_t ind) {
    if( ind == 0 ) { // when ind = 0, clear all hashes.
      data.clear();
      previous_drop_number = 0;
      txs = 0;
      data.emplace_back( capacity() );
      return 0;
    }

    // it is dengerous calc, so return immediatelly.
    if( previous_drop_number >= ind ) return ind;

    size_t id_tx = txs; // now seeing hash pointer
    size_t cap = 1, xcap = capacity();
    bool upd_flag = false; txs = ind;

    // This loop traverse up to the tree
    for( auto layer = data.begin(); layer != data.end(); ++layer ) {
      size_t num = ( id_tx % ( cap * xcap ) )/cap; // it is number of hashes, layer has.
      size_t rm_num = std::min( layer->size(), (id_tx - ind + cap - 1)/cap ); // it is number of hashes should be removed in this layer.
      layer->pop( rm_num );
      if( !upd_flag && layer->size() ) { // if this layer is barely point can be removed, determine current tx pointer after dropped.
        txs = id_tx - rm_num * cap;
        upd_flag = true;
      }
      id_tx -= num * cap;
      cap *= xcap;
    }
    // memo revert point.
    previous_drop_number = txs;

    return txs;
  }

  template <typename T>
  T NarrowMerkleTree<T>::get_root() const {
    for( auto layer = data.begin(); layer != data.end(); ++layer ) {
      if( layer->size() ) return layer->back(); // if this layer has hash, its back is root.
    }
    return T();
  }


  template <typename T>
  size_t NarrowMerkleTree<T>::size() const {
    return txs;
  }

}  // namespace merkle
}  // namespace ametsuchi