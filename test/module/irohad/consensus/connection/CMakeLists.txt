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

# Consensus Connection Test
add_executable(consensus_connection_test
  consensus_connection_test.cpp
)
target_link_libraries(consensus_connection_test
  server_runner
  consensus_connection
  schema
  endpoint
  gtest
)
add_test(
  NAME consensus_connection_test
  COMMAND $<TARGET_FILE:consensus_connection_test>
)
