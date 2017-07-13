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

#include <peer_service/peer_service.hpp>

// TODO
namespace peer_service {

  void initialize() {
    detail::self_ = SelfStatus();

    // detail::peers_ = ametsuchi::getPeers()
  }

  /**
   * @return List of peers that therefore permutation.
   */
  std::vector<std::shared_ptr<Peer>> getPermutationPeers() {
    return std::vector<std::shared_ptr<Peer>>();
  }

  /**
   * @param i index
   * @return A i-th peer that therefore permutation.
   */
  Peer getPermutationAt(int i) {
    return Peer();
  }

  /**
   * @return List of peers that is used by ordering service.
   */
  std::vector<std::shared_ptr<Peer>> getOrderingPeers() {
    return std::vector<std::shared_ptr<Peer>>();
  }

  /**
   * @return List of peers that is used by ordering service and is that will
   * be send sumeragi.
   */
  std::vector<std::shared_ptr<Peer>> getActiveOrderingPeers() {
    return std::vector<std::shared_ptr<Peer>>();
  }

  /**
   * @return self status
   */
  const SelfStatus& self() { return detail::self_; }

  /**
   * When on_porposal sends on_commit, it is called.
   * It check signs from Block, and identify dead peers which It throw to
   * issue Peer::Remove transaction.
   * @param commited_block commited block with signs
   */
  void RemoveDeadPeers(const iroha::model::Block& commited_block) {}

  /**
   * When on_commit, it is called.
   * It change peer oreder.
   */
  void changePermutation() {}

  /**
   * When commit fails, it is called.
   * It throw to issue Peer::Stop(Self) transaction.
   */
  void selfStop() {}

  /**
   * When commit successes and state of self peer is UnSynced, It is called.
   * It throw to issue Peer::Activate(self) transaction.
   */
  void selfActivate() {}

  /**
   * validate command
   */
  void validate(const Command::Peer::Add& cmd) {}
  void validate(const Command::Peer::Remove& cmd) {}
  void validate(const Command::Peer::Activate& cmd) {}
  void validate(const Command::Peer::Stop& cmd) {}
  void validate(const Command::Peer::ChangeRole& cmd) {}

  /**
   * execute command
   */
  void execute(const Command::Peer::Add& cmd) {}
  void execute(const Command::Peer::Remove& cmd) {}
  void execute(const Command::Peer::Activate& cmd) {}
  void execute(const Command::Peer::Stop& cmd) {}
  void execute(const Command::Peer::ChangeRole& cmd) {}

  namespace detail {
    void issueStop(const std::string& ip, const Peer& stop_peer) {}
    void issueActivate(const std::string& ip, const Peer& activate_peer) {}
  }  // namespace detail
}  // namespace peer_servince