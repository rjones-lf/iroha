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
#include <vector>

#include <typeinfo>

namespace exception {

  class FileOpenException : public std::invalid_argument {
    public: FileOpenException(const std::string&);
  };

  class NotImplementedException : public std::invalid_argument {
    public: NotImplementedException(
      const std::string& message,
      const std::string& filename
    );
  };

  class InvalidCastException : public std::domain_error {
    public: InvalidCastException(
      const std::string& from,
			const std::string&   to,
      const std::string& filename
    );
  };

  namespace crypto {
    class InvalidKeyException : public std::invalid_argument{
      public: InvalidKeyException(const std::string&);
    };
  }

  namespace repository {
    class WriteFailedException : public std::invalid_argument {
    public:
      WriteFailedException(const std::string&);
    };
  }

  namespace transaction {
    class UnsetBuildArgmentsException : public std::domain_error {
    public:
      UnsetBuildArgmentsException(const std::string&, const std::string&);
    };
  }

};  // namespace exception

#endif
