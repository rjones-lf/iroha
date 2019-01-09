/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main/iroha_conf_loader.hpp"

#include <fstream>
#include <limits>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/rapidjson.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>

#include "main/assert_config.hpp"

namespace ac = assert_config;
namespace mbr = config_members;

static constexpr size_t kBadJsonPrintLength = 15;
static constexpr size_t kBadJsonPrintOffsset = 5;
static_assert(kBadJsonPrintOffsset <= kBadJsonPrintLength,
              "The place of error is out of the printed string boundaries!");

/**
 * Adds the children logger configs from parent logger JSON object to parent
 * logger config. The parent logger JSON object is searched for the children
 * config section, and the children configs are parsed and created if the
 * section is present.
 * @param parent_config - the parent logger config
 * @param parent_obj - the parent logger json configuration
 */
void addChildrenLoggerConfigs(
    const logger::LoggerManagerTreePtr &parent_config,
    const rapidjson::Value::ConstObject &parent_obj);

/**
 * Overrides the logger configuration with the values from JSON object.
 * @param dst - the configuration to use as base
 * @param src - the JSON object to take overrides from
 */
void updateLoggerConfig(logger::LoggerConfig &cfg,
                        const rapidjson::Value::ConstObject &obj);

/**
 * Gets a value by a key from a JSON object, if present.
 * @param dest - the variable to store the value
 * @param obj - the source JSON object
 * @param key - the key for the requested value
 * @return true if the value was loaded, otherwise false.
 */
template <typename TDest, typename TKey>
bool tryGetValByKey(TDest &dest,
                    const rapidjson::Value::ConstObject &obj,
                    const TKey &key);

// ------------ getVal(dst, src) ------------
// getVal is a set of functions that load the value from rapidjson::Value to a
// given destination variable. They check the JSON type and throw exception if
// it is wrong.

template <typename TDest>
typename std::enable_if<not std::numeric_limits<TDest>::is_integer>::type
getVal(TDest &, const rapidjson::Value &) {
  BOOST_THROW_EXCEPTION(
      std::runtime_error("Wrong type. Should never reach here."));
}

template <typename TDest>
typename std::enable_if<std::numeric_limits<TDest>::is_integer>::type getVal(
    TDest &dest, const rapidjson::Value &src) {
  ac::assert_fatal(src.IsInt64(), "must be an integer");
  const int64_t val = src.GetInt64();
  ac::assert_fatal(val >= std::numeric_limits<TDest>::min()
                       && val <= std::numeric_limits<TDest>::max(),
                   "integer value out of range");
  dest = val;
}

template <>
void getVal<bool>(bool &dest, const rapidjson::Value &src) {
  ac::assert_fatal(src.IsBool(), "must be a boolean");
  dest = src.GetBool();
}

template <>
void getVal<std::string>(std::string &dest, const rapidjson::Value &src) {
  ac::assert_fatal(src.IsString(), "must be a string");
  dest = src.GetString();
}

template <>
void getVal<logger::LogLevel>(logger::LogLevel &dest,
                              const rapidjson::Value &src) {
  std::string level_str;
  getVal(level_str, src);
  const auto it = mbr::LogLevels.find(level_str);
  if (it == mbr::LogLevels.end()) {
    BOOST_THROW_EXCEPTION(std::runtime_error(
        "Log level must be one of '"
        + boost::algorithm::join(mbr::LogLevels | boost::adaptors::map_keys,
                                 "', '")
        + "'."));
  }
  dest = it->second;
}

template <>
void getVal<logger::LogPatterns>(logger::LogPatterns &dest,
                                 const rapidjson::Value &src) {
  ac::assert_fatal(src.IsObject(), "must be a map from log level to pattern");
  for (const auto &pattern_entry : src.GetObject()) {
    logger::LogLevel level;
    std::string pattern_str;
    getVal(level, pattern_entry.name);
    getVal(pattern_str, pattern_entry.value);
    dest.setPattern(level, pattern_str);
  }
}

template <>
void getVal<logger::LoggerManagerTreePtr>(logger::LoggerManagerTreePtr &dest,
                                          const rapidjson::Value &src) {
  ac::assert_fatal(src.IsObject(), " must be a logger tree config");
  logger::LoggerConfig root_config{logger::kDefaultLogLevel,
                                   logger::kDefaultLogPatterns};
  updateLoggerConfig(root_config, src.GetObject());
  dest = std::make_shared<logger::LoggerManagerTree>(std::move(root_config));
  addChildrenLoggerConfigs(dest, src.GetObject());
}

template <>
void getVal<IrohadConfig>(IrohadConfig &dest, const rapidjson::Value &src) {
  ac::assert_fatal(src.IsObject(),
                   "Irohad config top element must be an object.");
  const auto obj = src.GetObject();
  getValByKey(dest.blok_store_path, obj, mbr::BlockStorePath);
  getValByKey(dest.torii_port, obj, mbr::ToriiPort);
  getValByKey(dest.internal_port, obj, mbr::InternalPort);
  getValByKey(dest.pg_opt, obj, mbr::PgOpt);
  getValByKey(dest.max_proposal_size, obj, mbr::MaxProposalSize);
  getValByKey(dest.proposal_delay, obj, mbr::ProposalDelay);
  getValByKey(dest.vote_delay, obj, mbr::VoteDelay);
  getValByKey(dest.mst_support, obj, mbr::MstSupport);
  getValByKey(dest.logger_manager, obj, mbr::LogSection);
}

// ------------ end of getVal(dst, src) ------------

void updateLoggerConfig(logger::LoggerConfig &cfg,
                        const rapidjson::Value::ConstObject &obj) {
  tryGetValByKey(cfg.log_level, obj, mbr::LogLevel);
  tryGetValByKey(cfg.patterns, obj, mbr::LogPatternsSection);
}

template <typename TDest, typename TKey>
bool tryGetValByKey(TDest &dest,
                    const rapidjson::Value::ConstObject &obj,
                    const TKey &key) {
  const auto it = obj.FindMember(key);
  if (it == obj.MemberEnd()) {
    return false;
  } else {
    getVal(dest, it->value);
    return true;
  }
}

/**
 * Gets an optional value by a key from a JSON object.
 * @param obj - the source JSON object
 * @param key - the key for the requested value
 * @return the value if present in the JSON object, otherwise boost::none.
 */
template <typename TDest, typename TKey>
boost::optional<TDest> getOptValByKey(const rapidjson::Value::ConstObject &obj,
                                      const TKey &key) {
  TDest val;
  return boost::make_optional(tryGetValByKey(val, obj, key), val);
}

/**
 * Gets a value by a key from a JSON object. Throws an exception if the value
 * was not loaded.
 * @param dest - the variable to store the value
 * @param obj - the source JSON object
 * @param key - the key for the requested value
 */
template <typename TDest, typename TKey>
void getValByKey(TDest &dest,
                 const rapidjson::Value::ConstObject &obj,
                 const TKey &key) {
  ac::assert_fatal(tryGetValByKey(dest, obj, key), ac::no_member_error(key));
}

void addChildrenLoggerConfigs(const logger::LoggerManagerTreePtr &parent_config,
                              const rapidjson::Value::ConstObject &parent_obj) {
  const auto it = parent_obj.FindMember(mbr::LogChildrenSection);
  if (it != parent_obj.MemberEnd()) {
    for (const auto &child_json : it->value.GetObject()) {
      ac::assert_fatal(child_json.name.IsString(),
                       "Child logger key must be a string holding its tag.");
      ac::assert_fatal(child_json.value.IsObject(),
                       "Child logger value must be a JSON object.");
      const auto child_obj = child_json.value.GetObject();
      auto child_conf = parent_config->registerChild(
          child_json.name.GetString(),
          getOptValByKey<logger::LogLevel>(child_obj, mbr::LogLevel),
          getOptValByKey<logger::LogPatterns>(child_obj,
                                              mbr::LogPatternsSection));
      addChildrenLoggerConfigs(child_conf, child_obj);
    }
  }
}

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

IrohadConfig parse_iroha_config(const std::string &conf_path) {
  const rapidjson::Document doc{[&conf_path] {
    rapidjson::Document doc;
    std::ifstream ifs_iroha(conf_path);
    rapidjson::IStreamWrapper isw(ifs_iroha);
    doc.ParseStream(isw);
    ac::assert_fatal(not doc.HasParseError(),
                     reportJsonParsingError(doc, conf_path, ifs_iroha));
    return doc;
  }()};

  IrohadConfig config;
  getVal(config, doc);
  return config;
}
