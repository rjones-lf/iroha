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

#ifndef IROHA_CONF_LOADER_HPP
#define IROHA_CONF_LOADER_HPP

#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/rapidjson.h>
#include <fstream>
#include <string>
#include "common/assert_config.hpp"

namespace config_members {
  const char *BlockStorePath = "block_store_path";
  const char *ToriiPort = "torii_port";
  const char *InternalPort = "internal_port";
  const char *KeyPairPath = "key_pair_path";
  const char *PgOpt = "pg_opt";
  const char *MaxProposalSize = "max_proposal_size";
  const char *ProposalDelay = "proposal_delay";
  const char *VoteDelay = "vote_delay";
  const char *LoadDelay = "load_delay";
}  // namespace config_members

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

  ac::assert_fatal(doc.HasMember(mbr::LoadDelay),
                   ac::no_member_error(mbr::LoadDelay));
  ac::assert_fatal(doc[mbr::LoadDelay].IsUint(),
                   ac::type_error(mbr::LoadDelay, kUintType));
  return doc;
}

#endif  // IROHA_CONF_LOADER_HPP
