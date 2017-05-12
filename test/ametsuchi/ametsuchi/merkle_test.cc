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

#include <ametsuchi/merkle_tree/merkle_tree.h>
#include <gtest/gtest.h>

namespace ametsuchi {
namespace merkle {

auto str2hex = [](const std::string &hex) {
  const std::string alp = "0123456789abcdef";

  hash_t out;
  size_t ptr = 0;
  for (size_t i = 0; i < hex.length(); i += 2) {
    out[ptr++] = alp.find(hex[i]) * 16 + alp.find(hex[i + 1]);
  }

  return out;
};

// sha3_256 of "1", https://emn178.github.io/online-tools/sha3_256.html
hash_t h =
    str2hex("67b176705b46206614219f47a05aee7ae6a3edbe850bbbe214c536b989aea4d2");

// calculated with
// https://gist.github.com/Warchant/774ad06f7c3e0e17bad0359f4bae3411
// for tree with 4 leafs
std::vector<hash_t> roots = {
    str2hex("67b176705b46206614219f47a05aee7ae6a3edbe850bbbe214c536b989aea4d2"),
    str2hex("80141312b118fb9099ec9277809de617cacc05a33312d3802d70a3db0d57e44a"),
    str2hex("a789f4a3c9b124149f17fda28573de4f4a9f205d428339f6308324b0d61917ed"),
    str2hex("5c3924aaff4fac9a843bc2229fec34ec01843accb996faf949d3a51e0800c2f8"),
    str2hex("1408519e75164ccfce0bd858b1c32f53dda2422f77adc2166508cbbd658ebab4"),
    str2hex("eef6f63a47272f280cb39f72d2ada587411e95c488d402044df20072630a8b9f"),
    str2hex("e618578c471c0eae2a05121d8bbe4a67cccb53f9511286bb60d93c295afcd263"),
    str2hex(
        "f13ec23c48a494d98b10cd5144dd0e3d8e2619222921e72aa403a0cead44a296")};


TEST(NaiveMerkle, Tree4_1block) {
  // should with size 4
  merkle::MerkleTree tree(4);
  for (size_t i = 0; i < 8; i++) {
    tree.push(h);
    auto root = tree.root();
    ASSERT_EQ(root, roots[i]);
  }
}

TEST(NaiveMerkle, Tree4_2blocks) {
  // tree with size 4, which stores 2 blocks
  merkle::MerkleTree tree(4, 2);
  for (size_t i = 0; i < 8; i++) {
    tree.push(h);
    auto root = tree.root();
    ASSERT_EQ(root, roots[i]) << "Expected root is different.";
  }
}


TEST(NaiveMerkle, Tree128_1_step_rollback) {
  size_t iterations = 10000;

  std::list<hash_t> history;

  merkle::MerkleTree tree(128);
  for (size_t i = 0; i < iterations; i++) {
    uint8_t *ptr = reinterpret_cast<uint8_t *>(&i);
    tree.push(tree.hash(ptr, sizeof(i)));
    history.push_back(tree.root());
    /* // for debug
    printf("%ld: ", i);
    tree.dump();
     */
  }

  auto rbegin = history.rbegin();
  auto rend = history.rend();

  // iterate in reverse direction
  size_t steps = 0;
  for (auto it = rbegin; it != rend; ++it) {
    const hash_t &expected_root = *it;
    const hash_t &actual_root = tree.root();

    size_t max_rollback = tree.max_rollback();

    /* // for debug
    printf("[%ld] ", max_rollback);
    printf("%ld: ", iterations - 1 - steps);
    tree.dump();
    */

    ASSERT_EQ(expected_root, actual_root) << "Roots are different after "
                                          << steps << " rollbacks.";

    steps++;

    // rollback on single step
    if (max_rollback == 0) {
      // correct behavior: tree throws std::bad_exception if max_rollback is 0
      ASSERT_THROW(tree.rollback(1), std::bad_exception)
          << "tree should have thrown an exception!";
      SUCCEED();
      break;  // end this loop
    } else {
      ASSERT_NO_THROW(tree.rollback(1));
    }
  }
}


TEST(NaiveMerkle, Tree128_single_max_rollback) {
  size_t iterations = 10000;

  std::list<hash_t> history;

  merkle::MerkleTree tree(128, 10);
  for (size_t i = 0; i < iterations; i++) {
    uint8_t *ptr = reinterpret_cast<uint8_t *>(&i);
    tree.push(tree.hash(ptr, sizeof(i)));
    history.push_back(tree.root());
  }

  size_t max_ = tree.max_rollback();
  auto ptr = history.rend();
  for (size_t i = 0; i <= max_; i++) ++ptr;

  tree.rollback(max_);
  ASSERT_EQ(tree.root(), *ptr);
}


TEST(NaiveMerkle, Tree128_100k_pushes) {
  merkle::MerkleTree tree(128);
  for (size_t i = 0; i < 100000; i++) {
    tree.push(roots[i % 8]);
  }
  SUCCEED();
}

// TODO(@warchant): add more tests, which use different combinations of block
// size and number of trees. Add more tests for rollback.

}  // namespace merkle
}  // namespace ametsuchi
