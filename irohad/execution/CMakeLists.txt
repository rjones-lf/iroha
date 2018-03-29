# Copyright 2018 Soramitsu Co., Ltd.
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

add_library(common_execution
        impl/common_executor.cpp
        )
target_link_libraries(common_execution
        rxcpp
        )

add_library(command_execution
        impl/command_executor.cpp
        )
target_link_libraries(command_execution
        ${Boost_LIBRARIES}
        logger
        common_execution
        validator
        rxcpp
        )
