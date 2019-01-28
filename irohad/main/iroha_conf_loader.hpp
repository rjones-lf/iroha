/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CONF_LOADER_HPP
#define IROHA_CONF_LOADER_HPP

#include <string>
#include <unordered_map>

#include "logger/logger_manager.hpp"

struct IrohadConfig {
  std::string blok_store_path;
  uint16_t torii_port;
  uint16_t internal_port;
  std::string pg_opt;
  uint32_t max_proposal_size;
  uint32_t proposal_delay;
  uint32_t vote_delay;
  bool mst_support;
  logger::LoggerManagerTreePtr logger_manager;
};

/**
 * parse and assert trusted peers json in `iroha.conf`
 * @param conf_path is a path to iroha's config
 * @return a parsed equivalent of that file
 */
IrohadConfig parse_iroha_config(const std::string &conf_path);

#endif  // IROHA_CONF_LOADER_HPP
