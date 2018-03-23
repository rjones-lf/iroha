#
# Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

addtest(get_transactions_test get_transactions_test.cpp)
target_link_libraries(get_transactions_test
  application
  integration_framework
  shared_model_proto_builders
  shared_model_stateless_validation
  )

addtest(create_role_test create_role_test.cpp)
target_link_libraries(create_role_test
  application
  integration_framework
  shared_model_proto_builders
  shared_model_stateless_validation
  )

addtest(invalid_fields_test invalid_fields_test.cpp)
target_link_libraries(invalid_fields_test
  application
  integration_framework
  shared_model_proto_builders
  shared_model_stateless_validation
  )

addtest(create_domain_test create_domain_test.cpp)
target_link_libraries(create_domain_test
  application
  integration_framework
  shared_model_proto_builders
  shared_model_stateless_validation
  )

addtest(create_account_test create_account_test.cpp)
target_link_libraries(create_account_test
  application
  integration_framework
  shared_model_proto_builders
  shared_model_stateless_validation
  )
