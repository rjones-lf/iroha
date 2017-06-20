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

#include <gflags/gflags.h>
#include <iostream>
#include <ed25519.h>

static bool ValidatePort(const char* flagname, std::int32_t value) {
  if (value > 0 && value < 32768)  // value is ok
    return true;
  std::printf("Invalid value for --%s: %d\n", flagname, (int)value);
  return false;
}
DEFINE_int32(port, 0, "What port to listen on");
DEFINE_validator(port, &ValidatePort);

DEFINE_bool(new_account, false, "Choose if account does not exist");

static bool ValidateQuorum(const char* flagname, std::uint32_t value) {
  if (value > 0)  // value is ok
    return true;
  std::printf("Invalid value for --%s: %d\n", flagname, (int)value);
  return false;
}
DEFINE_uint32(quorum, 0, "Define quorum size");

void create_account();

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage("some usage message");
  gflags::SetVersionString("1.0.0");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  gflags::ShutDownCommandLineFlags();

  if (FLAGS_new_account) {
    create_account();
  }
}

void create_account() {
  uint8_t quorum;
  std::cout << "Define quorum:" << std::endl;
  std::cin >> quorum;


}