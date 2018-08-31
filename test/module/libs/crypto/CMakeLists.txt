#
# Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
# http://soramitsu.co.jp
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Hash Test
AddTest(hash_test hash_test.cpp)
target_link_libraries(hash_test
    ed25519_crypto
    )

# Singature Test
AddTest(signature_test signature_test.cpp)
target_link_libraries(signature_test
    ed25519_crypto
    )

# Key manager Test
AddTest(keys_manager_test keys_manager_test.cpp)
target_link_libraries(keys_manager_test
    keys_manager
    )
