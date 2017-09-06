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

# Interactive Client
add_library(interactive_cli
    impl/interactive_cli.cpp
    impl/interactive_common_cli.cpp
    impl/interactive_query_cli.cpp
    impl/interactive_transaction_cli.cpp
    )
target_link_libraries(interactive_cli
    model
    parser
    )
target_include_directories(interactive_cli PUBLIC
    ${PROJECT_SOURCE_DIR}/iroha-cli
    )
