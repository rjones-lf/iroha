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

// Iroha should halt.
class Critical : public IrohaException {
 public:
  explicit Critical(const std::string &);
};

// used by debug.
class WontFix : public IrohaException {
 public:
  explicit WontFix(const std::string &);
};

// No problem for running, but it is insecure. ex. using default private key.
class Insecure : public IrohaException {
 public:
  explicit Insecure(const std::string &);
};

// no problem for running. Iroha manages for unstop.
class Ordinary : public IrohaException {
 public:
  explicit Ordinary(const std::string &);
};

class None : public NoError {
 public:
  None();
};

class NotImplementedException : public Ordinary {
 public:
  explicit NotImplementedException(const std::string &functionName,
                                   const std::string &filename);
};

class InvalidCastException : public Ordinary {
 public:
  InvalidCastException(const std::string &from, const std::string &to,
                       const std::string &filename);
  InvalidCastException(const std::string &meg, const std::string &filename);
};

class DuplicateSetArgumentException : public Ordinary {
 public:
  DuplicateSetArgumentException(const std::string &, const std::string &);
};

class UnsetBuildArgumentsException : public Ordinary {
 public:
  UnsetBuildArgumentsException(const std::string &, const std::string &);
};

class RequirePropertyMissingException : public IrohaException {
public:
    RequirePropertyMissingException(const std::string &, const std::string &);
};

class NotFoundPathException : public Insecure {
 public:
  NotFoundPathException(const std::string &path);
};

namespace config {

// deprecated, will remove
class ConfigException : public Insecure {
 public:
  ConfigException(const std::string &message, const std::string &filename);
};

class ParseException : public Insecure {
 public:
  ParseException(const std::string &target, bool setDefaultMessage = false);
};

class UndefinedIrohaHomeException : Critical {
 public:
  UndefinedIrohaHomeException();
};

}  // namespace config

namespace connection {
class NullptrException : public Ordinary {
 public:
  NullptrException(const std::string &target);
};

class FailedToCreateConsensusEvent : public Ordinary {
 public:
  FailedToCreateConsensusEvent();
};
}  // namespace connection

namespace service {

class DuplicationIPException : public Insecure {
 public:
  explicit DuplicationIPException(const std::string &);
};

class DuplicationPublicKeyException : public Insecure {
 public:
  explicit DuplicationPublicKeyException(const std::string &);
};

class UnExistFindPeerException : public Insecure {
 public:
  explicit UnExistFindPeerException(const std::string &);
};

}  // namespace service

namespace crypto {

class InvalidKeyException : public Insecure {
 public:
  explicit InvalidKeyException(const std::string &);
};

class InvalidMessageLengthException : public Insecure {
 public:
  explicit InvalidMessageLengthException(const std::string &);
};

}  // namespace crypto
}  // namespace exception

#endif  // IROHA_UTILS_EXCEPTION_INSTANCES_HPP_
