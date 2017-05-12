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

#include <ametsuchi/exception.h>
#include <ametsuchi/merkle_tree/merkle_tree.h>
#include <algorithm>
#include <iomanip>

extern std::shared_ptr<spdlog::logger> console;

namespace ametsuchi {
namespace merkle {

/**
 * Returns floor(log2(x))
 * log2(0) => undefined behaviour
 */
static inline size_t log2(size_t x) {
  size_t y = 0;
  while (x > 1) {
    x >>= 1;
    y++;
  }
  return y;
}

/**
 * Returns integer log2 of x
 * ceil2(5) = 8
 */
static inline size_t ceil2(size_t x) {
  size_t l = 1u << log2(x);
  return l == x ? l : l + 1;
}

static inline size_t treesize(size_t leafs) { return leafs * 2 - 1; }

MerkleTree::MerkleTree(size_t leafs, size_t blocks) : leafs_(0) {
  if (blocks == 0) throw std::bad_alloc();

  max_blocks_ = blocks;

  // round number of leafs to the power of 2
  leafs_ = ceil2(leafs);

  // full tree size
  size_ = treesize(leafs_);
  trees_.push_back(tree_t(size_));

  i_current_ = leafs_ - 1;
  i_root_ = i_current_;
}

hash_t MerkleTree::root() { return trees_.back()[i_root_]; }

void MerkleTree::push(const hash_t &item) {
  tree_t &tree = trees_.back();

  if (i_current_ == leafs_ - 1) {
    // this is the very first push. just move item to the leftmost leaf
    tree[i_current_] = item;
    i_root_ = i_current_++;
    return;
  }


  // copy hash to current empty position
  tree[i_current_] = item;

  // find LCA(leftmost leaf, i_current_)
  // LCA is this many levels above:
  size_t np = 1 + log2(i_current_ - (leafs_ - 1));
  size_t subtree_root = parent(i_current_);
  size_t current = i_current_;
  for (size_t i = 0; i < np; i++) {
    size_t left = this->left(subtree_root);
    size_t right = this->right(subtree_root);
    if (current == left) {
      // no right child, just pass left child as hash to parent
      tree[subtree_root] = tree[left];
    } else {
      tree[subtree_root] = hash(tree[left], tree[right]);
    }

    // new root resides at this cell:
    i_root_ = subtree_root;

    subtree_root = parent(subtree_root);
    current = parent(current);
  }

  i_current_++;

  // if current tree is full, allocate new tree
  if (i_current_ == size_) {
    // tree is complete, logically means creation of a NEW BLOCK

    // allocate new tree
    trees_.push_back(std::move(tree_t(size_)));
    tree_t &last = trees_.back();

    last[leafs_ - 1] = tree[0];  // copy root to leftmost leaf
    i_root_ = leafs_ - 1;        // change root pointer
    i_current_ = leafs_;         // change pointer to current free cell

    // remove the least recently used tree
    if (trees_.size() == max_blocks_ + 2) trees_.pop_front();

    return;
  }
}

void MerkleTree::rollback(size_t steps) {
  // just do nothing
  if (steps == 0) return;

  // total number of available "rollback" steps
  size_t max_steps = max_rollback();

  // we can not rollback to more than this many steps:
  if (steps > max_steps) throw std::bad_exception();

  // rollback to more than one tree
  while (steps >= leafs_) {
    steps -= (leafs_ - 1);
    trees_.pop_back();
  }

  if (i_current_ - steps < leafs_) {
    // rollback to more than one tree
    steps -= i_current_ - leafs_;
    trees_.pop_back();

    i_current_ = size_;
    i_root_ = 0;
  }

  tree_t &tree = trees_.back();

  i_current_ = i_current_ - steps - 1;
  push(tree[i_current_]);
}

void MerkleTree::push(hash_t &&item) {
  auto it = std::move(item);
  this->push(it);
}

inline size_t MerkleTree::left(size_t parent) { return parent * 2 + 1; }

inline size_t MerkleTree::right(size_t parent) { return parent * 2 + 2; }

inline size_t MerkleTree::parent(size_t node) {
  return node == 0 ? 0 : (node - 1) / 2;
}

hash_t MerkleTree::hash(const hash_t &a, const hash_t &b) {
  std::array<uint8_t, 2 * HASH_LEN> input;
  hash_t output;

  // concatenate input hashes
  std::copy(a.begin(), a.end(), &input[0]);
  std::copy(b.begin(), b.end(), &input[HASH_LEN]);

  SHA3_256(output.data(), input.data(), input.size());

  return output;
}

hash_t MerkleTree::hash(const std::vector<uint8_t> &data) {
  hash_t output;
  SHA3_256(output.data(), data.data(), data.size());
  return output;
}

hash_t MerkleTree::hash(const uint8_t *data, size_t size) {
  hash_t output;
  SHA3_256(output.data(), data, size);
  return output;
}

std::string MerkleTree::printelement(const std::vector<hash_t> &tree, size_t i,
                                     size_t amount) {
  std::string out;
  if (i == i_root_) out += "\033[0;31m";     // root = red
  if (i == i_current_) out += "\033[0;32m";  // current = yellow/green

  if (tree[i][0] == 0 && tree[i][1] == 0) {
    std::generate_n(std::back_inserter(out), amount * 2, []() { return '0'; });
  } else {
    const char alph[] = "0123456789abcdef";
    for (size_t j = 0; j < amount; j++) {
      out += alph[(tree[i][j] / 16)];
      out += alph[(tree[i][j] % 16)];
    }
  }

  if (i == i_root_) out += "\033[0m";
  if (i == i_current_) out += "\033[0m";

  if (i != tree.size() - 1) out += ", ";
  return out;
}

void MerkleTree::dump(size_t amount) {
  tree_t &tree = trees_.back();

  std::string out = "[";
  for (size_t i = 0; i < tree.size(); i++) {
    out += printelement(tree, i, amount);
  }

  out += "]";
  printf("%s\n", out.c_str());
}

size_t MerkleTree::max_rollback() {
  return (trees_.size() - 1) * (leafs_ - 1) + (i_current_ - leafs_);
}

const MerkleTree::tree_t MerkleTree::last_block() const {
  if (trees_.size() > 0)
    return trees_.back();
  else
    throw exception::Exception("empty tree");
}
size_t MerkleTree::last_block_begin() const {
  return leafs_ - 1;
}
size_t MerkleTree::last_block_end() const {
  return i_current_;
}

}  // namespace merkle
}  // namespace ametsuchi
