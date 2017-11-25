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

addtest(stateless_transaction_validator_test stateless/transaction_validator_test.cpp)
target_link_libraries(stateless_transaction_validator_test
    stateless_validator
    hash
    cryptography
    schema
    model
    )

addtest(query_execution_test query_execution_test.cpp)
target_link_libraries(query_execution_test
    model_generators
    model
    )

addtest(chain_validation_test chain_validation_test.cpp)
target_link_libraries(chain_validation_test
    chain_validator
    )
