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

addtest(transaction_validator_test
    transaction_validator_test.cpp
    )
target_link_libraries(transaction_validator_test
    shared_model_proto_backend
    shared_model_stateless_validation
    )

addtest(query_validator_test
    query_validator_test.cpp
    )
target_link_libraries(query_validator_test
    shared_model_proto_backend
    shared_model_stateless_validation
    )

addtest(field_validator_test
    field_validator_test.cpp
    )
target_link_libraries(field_validator_test
    shared_model_proto_backend
    shared_model_stateless_validation
    )

addtest(container_validator_test
    container_validator_test.cpp
    )
target_link_libraries(container_validator_test
    shared_model_proto_backend
    shared_model_stateless_validation
    )
