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

add_library(tx_pipeline_integration_test_fixture
    tx_pipeline_integration_test_fixture.cpp
    )
target_include_directories(tx_pipeline_integration_test_fixture PUBLIC ${PROJECT_SOURCE_DIR}/test)
target_link_libraries(tx_pipeline_integration_test_fixture
    application
    gtest
    )

addtest(tx_pipeline_integration_test tx_pipeline_integration_test.cpp)
target_link_libraries(tx_pipeline_integration_test
    application
    raw_block_insertion
    model_generators
    integration_framework
    shared_model_cryptography
    shared_model_stateless_validation
    integration_framework_config_helper
    tx_pipeline_integration_test_fixture
    )

addtest(transfer_asset_inter_domain_test transfer_asset_inter_domain_test.cpp)
target_link_libraries(transfer_asset_inter_domain_test
    application
    raw_block_insertion
    model_generators
    tx_pipeline_integration_test_fixture
  )
