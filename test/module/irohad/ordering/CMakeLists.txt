# Copyright 2017 Soramitsu Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

addtest(ordering_service_test ordering_service_test.cpp)
target_link_libraries(ordering_service_test
    ordering_service
    shared_model_stateless_validation
    )

addtest(ordering_gate_test ordering_gate_test.cpp)
target_link_libraries(ordering_gate_test
    ordering_service
    )

addtest(ordering_gate_service_test ordering_gate_service_test.cpp)
target_link_libraries(ordering_gate_service_test
        ordering_service
        shared_model_stateless_validation
    )
