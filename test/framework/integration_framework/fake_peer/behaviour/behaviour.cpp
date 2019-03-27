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
      // subscribe for all messages
      subscriptions_.emplace_back(
          getFakePeer().getMstStatesObservable().subscribe(
              [this](const auto &message) {
                this->processMstMessage(message);
              }));
      subscriptions_.emplace_back(
          getFakePeer().getYacStatesObservable().subscribe(
              [this](const auto &message) {
                this->processYacMessage(message);
              }));
      subscriptions_.emplace_back(
          getFakePeer().getOsBatchesObservable().subscribe(
              [this](const auto &batch) { this->processOsBatch(batch); }));
      subscriptions_.emplace_back(
          getFakePeer().getOgProposalsObservable().subscribe(
              [this](const auto &proposal) {
                this->processOgProposal(proposal);
              }));
      subscriptions_.emplace_back(
          getFakePeer().getBatchesObservable().subscribe(
              [this](const auto &batches) {
                this->processOrderingBatches(*batches);
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
