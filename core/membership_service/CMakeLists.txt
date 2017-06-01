/*
Copyright Soramitsu Co., Ltd. 2016,2017 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ADD_LIBRARY(izanami STATIC
#    izanami.cpp
#)

#target_link_libraries(izanami
#    hash
#    thread_pool
#    config_manager
#    peer_service
#    executor
#    connection_with_grpc
#    transaction_repository
#)

ADD_LIBRARY(membership_service STATIC
    peer_service.cpp
    hijiri.cpp
    synchronizer.cpp)

target_link_libraries(membership_service
    timer
    exception
    logger
    config_manager
    cache_map
    flatbuffer_service
    connection_with_grpc_flatbuffer
)