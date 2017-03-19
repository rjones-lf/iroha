/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdexcept>
#include <string>

#include "exception.hpp"

namespace exception {

  FileOpenException::FileOpenException(const std::string &filename) :
      std::invalid_argument("file " + filename + " is not found!") {
  }

  NotImplementedException::NotImplementedException(
      const std::string &functionName,
      const std::string &filename
  ) :
      std::invalid_argument("TODO: sorry [" + functionName + "] in " + filename +
                            " is not yet implemented, would you like to contribute it?") {
  }

  BaseMethodException::BaseMethodException(
      const std::string &functionName,
      const std::string &filename
  ) :
      std::domain_error("BaseMethodException [" + functionName + "] in " + filename) {
  }

  namespace config {
    ConfigException::ConfigException(
        const std::string &message
    ) :
        std::domain_error("ConfigException: " + message) {
    }
  }

  ParseFromStringException::ParseFromStringException(
      const std::string &filename
  ) :
      std::domain_error("ParseFromStringException in " + filename) {
  }

  InvalidCastException::InvalidCastException(
      const std::string &from,
      const std::string &to,
      const std::string &filename
  ) :
      std::domain_error("InvalidCastException in " + filename + ". Cannot cast from " + from + " to " + to) {
  }

  InvalidCastException::InvalidCastException(
      const std::string &meg,
      const std::string &filename
  ) :
      std::domain_error("InvalidCastException in " + filename + ". " + meg) {}

  namespace service {
    DuplicationIPException::DuplicationIPException(const std::string &ip) :
        std::domain_error("DuplicationIPException : IP = " + ip + "\n") {
    }

    DuplicationPublicKeyException::DuplicationPublicKeyException(const std::string &publicKey) :
        std::domain_error("DuplicationPublicKeyException : publicKey = " + publicKey + "\n") {
    }

    UnExistFindPeerException::UnExistFindPeerException(const std::string &publicKey) :
        std::domain_error("UnExistFindPeerException : publicKey = " + publicKey + "\n") {
    }
  }


  namespace crypto {
    InvalidKeyException::InvalidKeyException(const std::string &message) :
        std::invalid_argument("keyfile is invalid cause:" + message) {
    }
  }  // namespace crypto

  namespace repository {
    WriteFailedException::WriteFailedException(const std::string &message) :
        std::invalid_argument("Data could note be saved:" + message) {
    }

    DuplicateAddException::DuplicateAddException(const std::string &object) :
        std::invalid_argument("DuplicateAddException: " + object) {
    }
  }  // namespace crypto

  namespace txbuilder {
    DuplicateSetArgmentException::DuplicateSetArgmentException(const std::string &buildTarget,
                                                               const std::string &duplicateMember) :
        std::domain_error("DuplicateSetArgmentException in " + buildTarget + ", argment: " + duplicateMember) {
    }

    UnsetBuildArgmentsException::UnsetBuildArgmentsException(const std::string &buildTarget,
                                                             const std::string &unsetMembers) :
        std::domain_error("UnsetBuildArgmentsException in " + buildTarget + ", argments: " + unsetMembers) {
    }
  }  // namespace transaction
}  // namespace exception