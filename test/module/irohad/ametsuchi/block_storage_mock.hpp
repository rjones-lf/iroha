/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include <gmock/gmock.h>
#include <string>
#include <vector>

#include "ametsuchi/block_storage.hpp"

namespace iroha {
  namespace ametsuchi {

    class BlockStorageMock : public BlockStorage {
     public:
      BlockStorageMock() = default;

      MOCK_METHOD2(add, bool(Identifier, const std::vector<uint8_t> &));
      MOCK_CONST_METHOD0(directory, const std::string &());
      MOCK_CONST_METHOD1(get,
                         boost::optional<std::vector<uint8_t>>(Identifier id));
      MOCK_CONST_METHOD0(total_keys, size_t());
      MOCK_METHOD0(drop_all, bool());
    };

  }  // namespace ametsuchi
}  // namespace iroha

