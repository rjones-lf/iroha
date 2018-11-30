/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/port_guard.hpp"

#include <boost/assert.hpp>

namespace integration_framework {

  constexpr PortGuard::PortType PortGuard::kMaxPort;
  PortGuard::UsedPorts PortGuard::all_used_ports_ = {};
  std::mutex PortGuard::all_used_ports_mutex_ = {};

  PortGuard::PortGuard() = default;

  PortGuard::PortGuard(PortGuard &&other) noexcept
      : instance_used_ports_(std::move(other.instance_used_ports_)) {
    other.instance_used_ports_.reset();
  }

  PortGuard::~PortGuard() {
    std::lock_guard<std::mutex> lock(all_used_ports_mutex_);
    BOOST_ASSERT_MSG(
        ((all_used_ports_ | instance_used_ports_) ^ all_used_ports_).none(),
        "Some ports used by this PortGuard instance are not set in ports "
        "used by all instances!");
    all_used_ports_ &= ~instance_used_ports_;
  }

  boost::optional<PortGuard::PortType> PortGuard::tryGetPort(
      const PortType min_value, const PortType max_value) {
    std::lock_guard<std::mutex> lock(all_used_ports_mutex_);
    PortType tested_port = min_value;
    while (all_used_ports_.test(tested_port)) {
      if (tested_port == max_value) {
        return boost::none;
      }
      ++tested_port;
    }
    BOOST_ASSERT_MSG(!all_used_ports_.test(tested_port),
                     "PortGuard chose an occupied port!");
    BOOST_ASSERT_MSG(tested_port >= min_value && tested_port <= max_value,
                     "PortGuard chose a port outside boundaries!");
    instance_used_ports_.set(tested_port);
    all_used_ports_.set(tested_port);
    return tested_port;
  }

  PortGuard::PortType PortGuard::getPort(const PortType min_value,
                                         const PortType max_value) {
    const boost::optional<PortType> opt_port = tryGetPort(min_value, max_value);
    BOOST_VERIFY_MSG(
        opt_port,
        ("Could not get a port in interval [" + std::to_string(min_value) + ", "
         + std::to_string(max_value) + "]!")
            .c_str());
    return *opt_port;
  }

  }  // namespace integration_framework
