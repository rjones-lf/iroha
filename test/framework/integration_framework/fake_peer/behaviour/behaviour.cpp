/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/behaviour/behaviour.hpp"

#include "logger/logger.hpp"

namespace integration_framework {
  namespace fake_peer {

    Behaviour::~Behaviour() {
      absolve();
    }

    void Behaviour::setup(const std::shared_ptr<FakePeer> &fake_peer,
                          logger::LoggerPtr log) {
      // This code feels like part of constructor, but the use of `this'
      // to call virtual functions from base class constructor seems wrong.
      // Hint: such calls would precede the derived class construction.
      fake_peer_wptr_ = fake_peer;
      log_ = std::move(log);
      std::weak_ptr<Behaviour> weak_this = shared_from_this();
      // subscribe for all messages
      subscriptions_.emplace_back(
          getFakePeer().getMstStatesObservable().subscribe(
              [weak_this,
               weak_fake_peer = fake_peer_wptr_](const auto &message) {
                auto me_alive = weak_this.lock();
                auto fake_peer_is_alive = weak_fake_peer.lock();
                if (me_alive and fake_peer_is_alive) {
                  me_alive->processMstMessage(message);
                }
              }));
      subscriptions_.emplace_back(
          getFakePeer().getYacStatesObservable().subscribe(
              [weak_this,
               weak_fake_peer = fake_peer_wptr_](const auto &message) {
                auto me_alive = weak_this.lock();
                auto fake_peer_is_alive = weak_fake_peer.lock();
                if (me_alive and fake_peer_is_alive) {
                  me_alive->processYacMessage(message);
                }
              }));
      subscriptions_.emplace_back(
          getFakePeer().getOsBatchesObservable().subscribe(
              [weak_this, weak_fake_peer = fake_peer_wptr_](const auto &batch) {
                auto me_alive = weak_this.lock();
                auto fake_peer_is_alive = weak_fake_peer.lock();
                if (me_alive and fake_peer_is_alive) {
                  me_alive->processOsBatch(batch);
                }
              }));
      subscriptions_.emplace_back(
          getFakePeer().getOgProposalsObservable().subscribe(
              [weak_this,
               weak_fake_peer = fake_peer_wptr_](const auto &proposal) {
                auto me_alive = weak_this.lock();
                auto fake_peer_is_alive = weak_fake_peer.lock();
                if (me_alive and fake_peer_is_alive) {
                  me_alive->processOgProposal(proposal);
                }
              }));
      subscriptions_.emplace_back(
          getFakePeer().getBatchesObservable().subscribe(
              [weak_this,
               weak_fake_peer = fake_peer_wptr_](const auto &batches) {
                auto me_alive = weak_this.lock();
                auto fake_peer_is_alive = weak_fake_peer.lock();
                if (me_alive and fake_peer_is_alive) {
                  me_alive->processOrderingBatches(*batches);
                }
              }));
    }

    void Behaviour::absolve() {
      for (auto &subscription : subscriptions_) {
        subscription.unsubscribe();
      }
      fake_peer_wptr_.reset();
    }

    FakePeer &Behaviour::getFakePeer() {
      auto fake_peer = fake_peer_wptr_.lock();
      assert(fake_peer && "Fake peer shared pointer is not set!"
        " Probably the fake peer has gone before the associated behaviour.");
      return *fake_peer;
    }

    logger::LoggerPtr &Behaviour::getLogger() {
      return log_;
    }

  }  // namespace fake_peer
}  // namespace integration_framework
