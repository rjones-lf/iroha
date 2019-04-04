/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "main/iroha_conf_loader.hpp"

#include <fstream>
#include <limits>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/rapidjson.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include "main/iroha_conf_literals.hpp"

/// The length of the string around the error place to print in case of JSON
/// syntax error.
static constexpr size_t kBadJsonPrintLength = 15;

/// The offset of printed chunk towards file start from the error position.
static constexpr size_t kBadJsonPrintOffsset = 5;

static_assert(kBadJsonPrintOffsset <= kBadJsonPrintLength,
              "The place of error is out of the printed string boundaries!");

/**
 * Adds the children logger configs from parent logger JSON object to parent
 * logger config. The parent logger JSON object is searched for the children
 * config section, and the children configs are parsed and created if the
 * section is present.
 * @param path - current config node path used to denote the possible error
 *    place.
 * @param parent_config - the parent logger config
 * @param parent_obj - the parent logger json configuration
 */
void addChildrenLoggerConfigs(const std::string &path,
                              const logger::LoggerManagerTreePtr &parent_config,
                              const rapidjson::Value::ConstObject &parent_obj);

/**
 * Overrides the logger configuration with the values from JSON object.
 * @param path - current config node path used to denote the possible error
 *    place.
 * @param cfg - the configuration to use as base
 * @param obj - the JSON object to take overrides from
 */
void updateLoggerConfig(const std::string &path,
                        logger::LoggerConfig &cfg,
                        const rapidjson::Value::ConstObject &obj);

/**
 * Gets a value by a key from a JSON object, if present.
 * @param path - current config node path used to denote the possible error
 *    place.
 * @param dest - the variable to store the value
 * @param obj - the source JSON object
 * @param key - the key for the requested value
 * @return true if the value was loaded, otherwise false.
 */
template <typename TDest, typename TKey>
bool tryGetValByKey(const std::string &path,
                    TDest &dest,
                    const rapidjson::Value::ConstObject &obj,
                    const TKey &key);

/**
 * Gets an optional value by a key from a JSON object.
 * @param path - current config node path used to denote the possible error
 *    place.
 * @param obj - the source JSON object
 * @param key - the key for the requested value
 * @return the value if present in the JSON object, otherwise boost::none.
 */
template <typename TDest, typename TKey>
boost::optional<TDest> getOptValByKey(const std::string &path,
                                      const rapidjson::Value::ConstObject &obj,
                                      const TKey &key);

/**
 * Adds one sublevel to the path denoting a place in config tree.
 * @param parent - the location of the sublevel
 * @param child - the name of sublevel
 * @return the path to the sublevel
 */
template <typename TChild>
inline std::string sublevelPath(std::string parent, TChild child) {
  std::stringstream child_sstream;
  child_sstream << child;
  return std::move(parent) + "/" + child_sstream.str();
}

/**
 * Gets a value by a key from a JSON object. Throws an exception if the value
 * was not loaded.
 * @param path - current config node path used to denote the possible error
 *    place.
 * @param dest - the variable to store the value
 * @param obj - the source JSON object
 * @param key - the key for the requested value
 */
template <typename TDest, typename TKey>
void getValByKey(const std::string &path,
                 TDest &dest,
                 const rapidjson::Value::ConstObject &obj,
                 const TKey &key);

/**
 * Throws a runtime exception if the given condition is false.
 * @param condition
 * @param error - error message
 */
inline void assert_fatal(bool condition, std::string error) {
  if (!condition) {
    throw std::runtime_error(error);
  }
}

// ------------ getVal(path, dst, src) ------------
// getVal is a set of functions that load the value from rapidjson::Value to a
// given destination variable. They check the JSON type and throw exception if
// it is wrong. The path argument is used to denote the possible error place.

template <typename TDest>
typename std::enable_if<not std::numeric_limits<TDest>::is_integer>::type
getVal(const std::string &path, TDest &, const rapidjson::Value &) {
  BOOST_THROW_EXCEPTION(
      std::runtime_error("Wrong type. Should never reach here."));
}

template <typename TDest>
typename std::enable_if<std::numeric_limits<TDest>::is_integer>::type getVal(
    const std::string &path, TDest &dest, const rapidjson::Value &src) {
  assert_fatal(src.IsInt64(), path + " must be an integer");
  const int64_t val = src.GetInt64();
  assert_fatal(val >= std::numeric_limits<TDest>::min()
                   && val <= std::numeric_limits<TDest>::max(),
               path + ": integer value out of range");
  dest = val;
}

template <>
void getVal<bool>(const std::string &path,
                  bool &dest,
                  const rapidjson::Value &src) {
  assert_fatal(src.IsBool(), path + " must be a boolean");
  dest = src.GetBool();
}

template <>
void getVal<std::string>(const std::string &path,
                         std::string &dest,
                         const rapidjson::Value &src) {
  assert_fatal(src.IsString(), path + " must be a string");
  dest = src.GetString();
}

template <>
void getVal<logger::LogLevel>(const std::string &path,
                              logger::LogLevel &dest,
                              const rapidjson::Value &src) {
  std::string level_str;
  getVal(path, level_str, src);
  const auto it = config_members::LogLevels.find(level_str);
  if (it == config_members::LogLevels.end()) {
    BOOST_THROW_EXCEPTION(std::runtime_error(
        "Wrong log level at " + path + ": must be one of '"
        + boost::algorithm::join(
              config_members::LogLevels | boost::adaptors::map_keys, "', '")
        + "'."));
  }
  dest = it->second;
}

template <>
void getVal<logger::LogPatterns>(const std::string &path,
                                 logger::LogPatterns &dest,
                                 const rapidjson::Value &src) {
  assert_fatal(src.IsObject(),
               path + " must be a map from log level to pattern");
  for (const auto &pattern_entry : src.GetObject()) {
    logger::LogLevel level;
    std::string pattern_str;
    getVal(sublevelPath(path, "(level name)"), level, pattern_entry.name);
    getVal(sublevelPath(path, "(pattern)"), pattern_str, pattern_entry.value);
    dest.setPattern(level, pattern_str);
  }
}

template <>
void getVal<logger::LoggerManagerTreePtr>(const std::string &path,
                                          logger::LoggerManagerTreePtr &dest,
                                          const rapidjson::Value &src) {
  assert_fatal(src.IsObject(), path + " must be a logger tree config");
  logger::LoggerConfig root_config{logger::kDefaultLogLevel,
                                   logger::LogPatterns{}};
  updateLoggerConfig(path, root_config, src.GetObject());
  dest = std::make_shared<logger::LoggerManagerTree>(
      std::make_shared<const logger::LoggerConfig>(std::move(root_config)));
  addChildrenLoggerConfigs(path, dest, src.GetObject());
}

template <>
void getVal<IrohadConfig>(const std::string &path,
                          IrohadConfig &dest,
                          const rapidjson::Value &src) {
  assert_fatal(src.IsObject(),
               path + " Irohad config top element must be an object.");
  const auto obj = src.GetObject();
  getValByKey(path, dest.block_store_path, obj, config_members::BlockStorePath);
  getValByKey(path, dest.torii_port, obj, config_members::ToriiPort);
  getValByKey(path, dest.internal_port, obj, config_members::InternalPort);
  getValByKey(path, dest.pg_opt, obj, config_members::PgOpt);
  getValByKey(
      path, dest.max_proposal_size, obj, config_members::MaxProposalSize);
  getValByKey(path, dest.proposal_delay, obj, config_members::ProposalDelay);
  getValByKey(path, dest.vote_delay, obj, config_members::VoteDelay);
  getValByKey(path, dest.mst_support, obj, config_members::MstSupport);
  getValByKey(
      path, dest.mst_expiration_time, obj, config_members::MstExpirationTime);
  getValByKey(
      path, dest.mst_state_txs_limit, obj, config_members::MstStateTxsLimit);
  getValByKey(
      path, dest.max_round_delay_ms, obj, config_members::MaxRoundsDelay);
  getValByKey(path,
              dest.stale_stream_max_rounds,
              obj,
              config_members::StaleStreamMaxRounds);
  getValByKey(path, dest.logger_manager, obj, config_members::LogSection);
}

// ------------ end of getVal(path, dst, src) ------------

void updateLoggerConfig(const std::string &path,
                        logger::LoggerConfig &cfg,
                        const rapidjson::Value::ConstObject &obj) {
  tryGetValByKey(path, cfg.log_level, obj, config_members::LogLevel);
  tryGetValByKey(path, cfg.patterns, obj, config_members::LogPatternsSection);
}

template <typename TDest, typename TKey>
bool tryGetValByKey(const std::string &path,
                    TDest &dest,
                    const rapidjson::Value::ConstObject &obj,
                    const TKey &key) {
  const auto it = obj.FindMember(key);
  if (it == obj.MemberEnd()) {
    return false;
  } else {
    getVal(sublevelPath(path, key), dest, it->value);
    return true;
  }
}

template <typename TDest, typename TKey>
bool tryGetValByKey(const std::string &path,
                    boost::optional<TDest> &dest,
                    const rapidjson::Value::ConstObject &obj,
                    const TKey &key) {
  dest = getOptValByKey<TDest>(path, obj, key);
  return true; // value loaded any way, either from file or boost::none
}

template <typename TDest, typename TKey>
boost::optional<TDest> getOptValByKey(const std::string &path,
                                      const rapidjson::Value::ConstObject &obj,
                                      const TKey &key) {
  TDest val;
  return boost::make_optional(tryGetValByKey(path, val, obj, key), val);
}

/**
 * Gets a value by a key from a JSON object. Throws an exception if the value
 * was not loaded.
 * @param path - current config node path used to denote the possible error
 *    place.
 * @param dest - the variable to store the value
 * @param obj - the source JSON object
 * @param key - the key for the requested value
 */
template <typename TDest, typename TKey>
void getValByKey(const std::string &path,
                 TDest &dest,
                 const rapidjson::Value::ConstObject &obj,
                 const TKey &key) {
  assert_fatal(tryGetValByKey(path, dest, obj, key),
               path + " has no key '" + key + "'.");
}

void addChildrenLoggerConfigs(const std::string &path,
                              const logger::LoggerManagerTreePtr &parent_config,
                              const rapidjson::Value::ConstObject &parent_obj) {
  const auto it = parent_obj.FindMember(config_members::LogChildrenSection);
  if (it != parent_obj.MemberEnd()) {
    auto children_section_path =
        sublevelPath(path, config_members::LogChildrenSection);
    for (const auto &child_json : it->value.GetObject()) {
      assert_fatal(child_json.name.IsString(),
                   "Child logger key must be a string holding its tag.");
      assert_fatal(child_json.value.IsObject(),
                   "Child logger value must be a JSON object.");
      auto child_tag = child_json.name.GetString();
      const auto child_obj = child_json.value.GetObject();
      auto child_path = sublevelPath(children_section_path, child_tag);
      auto child_conf = parent_config->registerChild(
          std::move(child_tag),
          getOptValByKey<logger::LogLevel>(
              child_path, child_obj, config_members::LogLevel),
          getOptValByKey<logger::LogPatterns>(
              child_path, child_obj, config_members::LogPatternsSection));
      addChildrenLoggerConfigs(std::move(child_path), child_conf, child_obj);
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
    assert_fatal(not doc.HasParseError(),
                 reportJsonParsingError(doc, conf_path, ifs_iroha));
    return doc;
  }()};

  IrohadConfig config;
  getVal("", config, doc);
  return config;
}
