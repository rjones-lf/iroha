/*
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * http://soramitsu.co.jp
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_UTILS_EXCEPTION_INSTANCES_HPP_
#define IROHA_UTILS_EXCEPTION_INSTANCES_HPP_

#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace exception {

class IrohaException : public std::exception {
 public:
  explicit IrohaException(const std::string &);

  virtual ~IrohaException();
  virtual const char *what() const
      throw();  // This is for throwing exception (auto appended '"');
  virtual std::string message() const;  // This is for getting exception message

 protected:
  std::string message_;
};

class NoError : public IrohaException {
 public:
  explicit NoError(const std::string &);
};

class Critical : public IrohaException {
 public:
  explicit Critical(const std::string &);
};

class WontFix : public IrohaException {
 public:
  explicit WontFix(const std::string &);
};

class HelpWanted : public IrohaException {
 public:
  explicit HelpWanted(const std::string &);
};

class ConfigError : public IrohaException {
 public:
  explicit ConfigError(const std::string &);
};

class None : public NoError {
 public:
  None();
};

class NotImplementedException : public HelpWanted {
 public:
  explicit NotImplementedException(const std::string &functionName,
                                   const std::string &filename);
};

class InvalidCastException : public Critical {
 public:
  InvalidCastException(const std::string &from, const std::string &to,
                       const std::string &filename);
  InvalidCastException(const std::string &meg, const std::string &filename);
};

class DuplicateSetArgumentException : public Critical {
 public:
  DuplicateSetArgumentException(const std::string &, const std::string &);
};

class UnsetBuildArgumentsException : public Critical {
 public:
  UnsetBuildArgumentsException(const std::string &, const std::string &);
};

class NotFoundPathException : public ConfigError {
 public:
  NotFoundPathException(const std::string &path);
};

namespace config {

// deprecated, will remove
class ConfigException : public ConfigError {
 public:
  ConfigException(const std::string &message, const std::string &filename);
};

class ParseException : public ConfigError {
 public:
  ParseException(const std::string &target, bool setDefaultMessage = false);
};

class UndefinedIrohaHomeException : ConfigError {
 public:
  UndefinedIrohaHomeException();
};

}  // namespace config

namespace connection {
class NullptrException : public Critical {
 public:
  NullptrException(const std::string &target);
};

class FailedToCreateConsensusEvent : public Critical {
 public:
  FailedToCreateConsensusEvent();
};
}  // namespace connection

namespace service {

class DuplicationIPException : public ConfigError {
 public:
  explicit DuplicationIPException(const std::string &);
};

class DuplicationPublicKeyException : public ConfigError {
 public:
  explicit DuplicationPublicKeyException(const std::string &);
};

class UnExistFindPeerException : public ConfigError {
 public:
  explicit UnExistFindPeerException(const std::string &);
};

}  // namespace service

namespace crypto {

class InvalidKeyException : public ConfigError {
 public:
  explicit InvalidKeyException(const std::string &);
};

class InvalidMessageLengthException : public Critical {
 public:
  explicit InvalidMessageLengthException(const std::string &);
};

}  // namespace crypto
}  // namespace exception

#endif  // IROHA_UTILS_EXCEPTION_INSTANCES_HPP_
