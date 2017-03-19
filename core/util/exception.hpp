/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

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

#ifndef __EXCEPTIONS_
#define __EXCEPTIONS_

#include <stdexcept>
#include <string>
#include <iostream>
#include <typeinfo>

namespace exception {

  class FileOpenException : public std::invalid_argument {
  public:
    FileOpenException(const std::string &);
  };

  class NotImplementedException : public std::invalid_argument {
  public:
    NotImplementedException(
        const std::string &functionName,
        const std::string &filename
    );
  };

  class BaseMethodException : public std::domain_error {
  public:
    BaseMethodException(
        const std::string &functionName,
        const std::string &filename
    );
  };

  class ParseFromStringException : public std::domain_error {
  public:
    ParseFromStringException(
        const std::string &filename
    );
  };

  class InvalidCastException : public std::domain_error {
  public:
    InvalidCastException(
        const std::string &from,
        const std::string &to,
        const std::string &filename
    );

    InvalidCastException(
        const std::string &meg,
        const std::string &filename
    );
  };

  namespace config {
    class ConfigException : public std::domain_error {
    public:
      ConfigException(const std::string &message);
    };
  }

  namespace service {
    class DuplicationIPException : public std::domain_error {
    public:
      DuplicationIPException(const std::string &);
    };

    class DuplicationPublicKeyException : public std::domain_error {
    public:
      DuplicationPublicKeyException(const std::string &);
    };

    class UnExistFindPeerException : public std::domain_error {
    public:
      UnExistFindPeerException(const std::string &);
    };
  }

  namespace crypto {
    class InvalidKeyException : public std::invalid_argument {
    public:
      InvalidKeyException(const std::string &);
    };
  };

  namespace repository {
    class WriteFailedException : public std::invalid_argument {
    public:
      WriteFailedException(const std::string &);
    };

    class DuplicateAddException : public std::invalid_argument {
    public:
      explicit DuplicateAddException(const std::string &);
    };
  }

  namespace txbuilder {
    class DuplicateSetArgmentException : public std::domain_error {
    public:
      DuplicateSetArgmentException(const std::string &, const std::string &);
    };

    class UnsetBuildArgmentsException : public std::domain_error {
    public:
      UnsetBuildArgmentsException(const std::string &, const std::string &);
    };
  }
}  // namespace exception

#define IROHA_ASSERT_TRUE(Condition)  \
  if ((Condition) == false) { std::cout << __func__ << " #" << __LINE__ << " in " << __FILE__ << std::endl; throw "Assertion failed." #Condition; }

#define IROHA_ASSERT_FALSE(Condition) \
  if ((Condition) == true) { std::cout << __func__ << " #" << __LINE__ << " in " << __FILE__ << std::endl; throw "Assertion failed." #Condition; }

#endif