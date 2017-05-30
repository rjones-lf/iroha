#
# Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Hash Test
add_executable(hash_test
        hash_test.cpp
)
target_link_libraries(hash_test
  hash
  crypto
  hash
  ssl
  gtest
)
add_test(
  NAME hash_test
  COMMAND $<TARGET_FILE:hash_test>
)


# Base64 Test
add_executable(base64_test base64_test.cpp)
target_link_libraries(base64_test 
  base64
  ssl
  crypto
  gtest
)
add_test(
  NAME base64_test
  COMMAND $<TARGET_FILE:base64_test>
)


# Singature Test
add_executable(signature_test signature_test.cpp)
target_link_libraries(signature_test 
  signature
  base64
  hash
  ed25519
  gtest
)
add_test(
  NAME signature_test
  COMMAND $<TARGET_FILE:signature_test>
)
