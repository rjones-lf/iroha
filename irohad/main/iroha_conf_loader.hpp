/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CONF_LOADER_HPP
#define IROHA_CONF_LOADER_HPP

#include <string>
#include <unordered_map>

#include "logger/logger_manager.hpp"

namespace config_members {
  const char *BlockStorePath = "block_store_path";
  const char *ToriiPort = "torii_port";
  const char *InternalPort = "internal_port";
  const char *KeyPairPath = "key_pair_path";
  const char *PgOpt = "pg_opt";
  const char *MaxProposalSize = "max_proposal_size";
  const char *ProposalDelay = "proposal_delay";
  const char *VoteDelay = "vote_delay";
  const char *MstSupport = "mst_enable";
  const char *LogSection = "log";
  const char *LogLevel = "level";
  const char *LogPatternsSection = "patterns";
  const char *LogChildrenSection = "children";
  const std::unordered_map<std::string, logger::LogLevel> LogLevels{
      {"trace", logger::LogLevel::kTrace},
      {"debug", logger::LogLevel::kDebug},
      {"info", logger::LogLevel::kInfo},
      {"warning", logger::LogLevel::kWarn},
      {"error", logger::LogLevel::kError},
      {"critical", logger::LogLevel::kCritical}};
}  // namespace config_members

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
