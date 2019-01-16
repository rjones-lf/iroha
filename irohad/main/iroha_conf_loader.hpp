/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CONF_LOADER_HPP
#define IROHA_CONF_LOADER_HPP

#include <fstream>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/rapidjson.h>

#include "main/assert_config.hpp"

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
}  // namespace config_members

static constexpr size_t kBadJsonPrintLength = 15;
static constexpr size_t kBadJsonPrintOffsset = 5;
static_assert(kBadJsonPrintOffsset <= kBadJsonPrintLength,
              "The place of error is out of the printed string boundaries!");

std::string reportJsonParsingError(const rapidjson::Document &doc,
                                   const std::string &conf_path,
                                   std::istream &input) {
  const size_t error_offset = doc.GetErrorOffset();
  // This ensures the unsigned string beginning position does not cross zero:
  const size_t print_offset =
      std::max(error_offset, kBadJsonPrintOffsset) - kBadJsonPrintOffsset;
  input.seekg(print_offset);
  std::string json_error_buf(kBadJsonPrintLength, 0);
  input.readsome(&json_error_buf[0], kBadJsonPrintLength);
  return "JSON parse error [" + conf_path + "] " + "(near `" + json_error_buf
      + "'): " + std::string(rapidjson::GetParseError_En(doc.GetParseError()));
}

/**
 * parse and assert trusted peers json in `iroha.conf`
 * @param conf_path is a path to iroha's config
 * @return rapidjson::Document is a parsed equivalent of that file
 */
inline rapidjson::Document parse_iroha_config(const std::string &conf_path) {
  namespace ac = assert_config;
  namespace mbr = config_members;
  rapidjson::Document doc;
  std::ifstream ifs_iroha(conf_path);
  rapidjson::IStreamWrapper isw(ifs_iroha);
  const std::string kStrType = "string";
  const std::string kUintType = "uint";
  const std::string kBoolType = "bool";
  doc.ParseStream(isw);
  ac::assert_fatal(not doc.HasParseError(),
                   reportJsonParsingError(doc, conf_path, ifs_iroha));

  ac::assert_fatal(doc.HasMember(mbr::BlockStorePath),
                   ac::no_member_error(mbr::BlockStorePath));
  ac::assert_fatal(doc[mbr::BlockStorePath].IsString(),
                   ac::type_error(mbr::BlockStorePath, kStrType));

  ac::assert_fatal(doc.HasMember(mbr::ToriiPort),
                   ac::no_member_error(mbr::ToriiPort));
  ac::assert_fatal(doc[mbr::ToriiPort].IsUint(),
                   ac::type_error(mbr::ToriiPort, kUintType));

  ac::assert_fatal(doc.HasMember(mbr::InternalPort),
                   ac::no_member_error(mbr::InternalPort));
  ac::assert_fatal(doc[mbr::InternalPort].IsUint(),
                   ac::type_error(mbr::InternalPort, kUintType));

  ac::assert_fatal(doc.HasMember(mbr::PgOpt), ac::no_member_error(mbr::PgOpt));
  ac::assert_fatal(doc[mbr::PgOpt].IsString(),
                   ac::type_error(mbr::PgOpt, kStrType));

  ac::assert_fatal(doc.HasMember(mbr::MaxProposalSize),
                   ac::no_member_error(mbr::MaxProposalSize));
  ac::assert_fatal(doc[mbr::MaxProposalSize].IsUint(),
                   ac::type_error(mbr::MaxProposalSize, kUintType));

  ac::assert_fatal(doc.HasMember(mbr::ProposalDelay),
                   ac::no_member_error(mbr::ProposalDelay));
  ac::assert_fatal(doc[mbr::ProposalDelay].IsUint(),
                   ac::type_error(mbr::ProposalDelay, kUintType));

  ac::assert_fatal(doc.HasMember(mbr::VoteDelay),
                   ac::no_member_error(mbr::VoteDelay));
  ac::assert_fatal(doc[mbr::VoteDelay].IsUint(),
                   ac::type_error(mbr::VoteDelay, kUintType));

  ac::assert_fatal(doc.HasMember(mbr::MstSupport),
                   ac::no_member_error(mbr::MstSupport));
  ac::assert_fatal(doc[mbr::MstSupport].IsBool(),
                   ac::type_error(mbr::MstSupport, kBoolType));
  return doc;
}

#endif  // IROHA_CONF_LOADER_HPP
