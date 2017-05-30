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

########################################################################################
# CacheMapTEST
########################################################################################
add_executable(cache_map_test cache_map_test.cpp)
target_link_libraries(cache_map_test
  gtest
  cache_map
)
add_test(
  NAME cache_map_test
  COMMAND $<TARGET_FILE:cache_map_test>
)
########################################################################################
# exceptionTEST
########################################################################################
add_executable(exception_test exception_test.cpp)
target_link_libraries(exception_test
  gtest
  exception
)
add_test(
  NAME exception_test
  COMMAND $<TARGET_FILE:exception_test>
)
########################################################################################
# loggerTEST
########################################################################################
add_executable(logger_test logger_test.cpp)
target_link_libraries(logger_test
  gtest
  logger
)
add_test(
  NAME logger_test
  COMMAND $<TARGET_FILE:logger_test>
)
