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

addtest(pipeline_test pipeline_test.cpp)
target_link_libraries(pipeline_test
    integration_framework
    shared_model_stateless_validation
    shared_model_proto_builders
    )

addtest(batch_pipeline_test batch_pipeline_test.cpp)
target_link_libraries(batch_pipeline_test
    acceptance_fixture
    )

addtest(multisig_tx_pipeline_test multisig_tx_pipeline_test.cpp)
target_link_libraries(multisig_tx_pipeline_test
    acceptance_fixture
    )
