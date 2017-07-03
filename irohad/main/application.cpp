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

#include "application.hpp"

#include <ametsuchi/ametsuchi_stub.hpp>
#include "dao/dao_crypto_provider_stub.hpp"
#include "dao/dao_hash_provider_impl.hpp"

namespace main {

  // TODO add initialization of ametsuchi and peer service
  Irohad::Irohad()
      : ametsuchi(Irohad::initialize_ametsuchi()),
        hashProvider(Irohad::initialize_hash_provider()),
        cryptoProvider(initialize_crypto_provider()) {}

  dao::HashProvider<32> &Irohad::initialize_hash_provider() {
    dao::HashProviderImpl res;
    return res;
  }
  dao::DaoCryptoProvider &Irohad::initialize_crypto_provider() {
    dao::DaoCryptoProviderStub res;
    return res;
  }
  ametsuchi::Ametsuchi &Irohad::initialize_ametsuchi() {
    ametsuchi::AmetsuchiStub res;
    return res;
  }
}