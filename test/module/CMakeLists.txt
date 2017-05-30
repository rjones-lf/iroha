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

set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

add_subdirectory(config)
add_subdirectory(connection)
add_subdirectory(crypto)
add_subdirectory(expected)
add_subdirectory(membership_service)
add_subdirectory(utils)
#add_subdirectory(infra/repository)
#add_subdirectory(infra/service)
#add_subdirectory(infra/config)
# commented because these tests do nothing
# TODO: make them do something :)
# add_subdirectory(consensus)
#add_subdirectory(vendor)
#add_subdirectory(validation)
#add_subdirectory(transaction_builder)

add_subdirectory(service)
