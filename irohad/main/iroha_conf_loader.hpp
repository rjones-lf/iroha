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

#include <boost/range/adaptor/map.hpp>
#include "consensus/yac/consistency_model.hpp"
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
  const char *ConsistencyModelKey = "consistency_model";
}  // namespace config_members

using iroha::consensus::yac::ConsistencyModel;
const std::unordered_map<std::string, ConsistencyModel> kConsistencyModels{
  {"CFT", ConsistencyModel::kCft},
  {"BFT", ConsistencyModel::kBft}
};

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
  ac::assert_fatal(
      not doc.HasParseError(),
      "JSON parse error [" + conf_path + "]: "
          + std::string(rapidjson::GetParseError_En(doc.GetParseError())));

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

  ac::assert_fatal(doc.HasMember(mbr::ConsistencyModelKey),
                   ac::no_member_error(mbr::ConsistencyModelKey));
  ac::assert_fatal(doc[mbr::ConsistencyModelKey].IsString(),
                   ac::type_error(mbr::ConsistencyModelKey, kStrType));
  ac::assert_fatal(
      kConsistencyModels.count(doc[mbr::ConsistencyModelKey].GetString()),
      ac::value_error(doc[mbr::ConsistencyModelKey].GetString(),
                      kConsistencyModels | boost::adaptors::map_keys));

  return doc;
}

#endif  // IROHA_CONF_LOADER_HPP
