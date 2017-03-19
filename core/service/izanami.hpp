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

#ifndef __CORE_IZANAMI_SERVICE_HPP__
#define __CORE_IZANAMI_SERVICE_HPP__

#include <infra/protobuf/api.pb.h>
#include <memory>
#include <string>
#include <vector>

namespace peer {
  namespace izanami {
    using Api::TransactionResponse;

    class InitializeEvent {
    private:
      uint64_t now_progress;
      std::unordered_map<std::string, std::unique_ptr<TransactionResponse>>
          txResponses;
      std::unordered_map<uint64_t, std::vector<std::string>> hashes;
      bool is_finished;

    public:
      InitializeEvent();

      void add_transactionResponse(std::unique_ptr<TransactionResponse>);

      const std::vector<std::string> &getHashes(uint64_t);

      const std::unique_ptr<TransactionResponse>
      getTransactionResponse(const std::string &);

      void next_progress();

      uint64_t now() const;

      void storeTxResponse(const std::string &);

      void executeTxResponse(const std::string &);

      bool isExistTransactionFromHash(const std::string &);

      void finish();

      bool isFinished() const;
    };

    namespace detail {
      bool isFinishedReceiveAll(InitializeEvent &event);

      bool isFinishedReceive(InitializeEvent &);

      std::string getCorrectHash(InitializeEvent &);

      void storeTransactionResponse(InitializeEvent &);
    }

// invoke when receive TransactionResponse.
    void receiveTransactionResponse(TransactionResponse &);

// invoke when initialize Peer that to config Participation on the way
    void startIzanami();

    void setAwkTimer(int const sleepMillisecs,
                     const std::function<void(void)> &action);
  }
}

#endif
