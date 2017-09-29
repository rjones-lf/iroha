include(ExternalProject)
set(EP_PREFIX "${PROJECT_SOURCE_DIR}/external")
set_directory_properties(PROPERTIES
    EP_PREFIX ${EP_PREFIX}
    )

# Project dependencies.
find_package(Threads REQUIRED)

##########################
#         gtest          #
##########################
# testing is an option. Look at the main CMakeLists.txt for details.
if (TESTING)
find_package(gtest)
endif ()

#############################
#         speedlog          #
#############################
find_package(spdlog)

################################
#           protobuf           #
################################
option(FIND_PROTOBUF "Try to find protobuf in system" ON)
find_package(protobuf)

#########################
#         gRPC          #
#########################
option(FIND_GRPC "Try to find gRPC in system" ON)
find_package(grpc)

################################
#          rapidjson           #
################################
find_package(rapidjson)

#############################
#         optional          #
#############################
find_package(optional)

################################
#            libuv             #
################################
find_package(uv)

################################
#             uvw              #
################################
find_package(uvw)

##########################
#       cpp_redis        #
##########################
find_package(cpp_redis)

##########################
#           pq           #
##########################
find_package(pq)

##########################a
#          pqxx          #
##########################
find_package(pqxx)

################################
#            gflags            #
################################
find_package(gflags)

##########################
#        rx c++          #
##########################
find_package(rxcpp)

##########################
#          TBB           #
##########################
find_package(tbb)

##########################
#         boost          #
##########################
find_package(Boost REQUIRED)


##########################
#    google benchmark    #
##########################
if(BENCHMARK)
  message(STATUS "google benchmark enabled")
  find_package(benchmark)
endif()


#######################
#    documentation    #
#######################
if(DOCS)
  cmake_policy(SET CMP0057 NEW)

  find_package(Doxygen
      REQUIRED dot
      )

  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_GENERATE_MAN NO)
  set(DOXYGEN_UML_LOOK YES)
  set(DOXYGEN_EXTRACT_ALL YES)
  # set(DOXYGEN_PROJECT_LOGO "${CMAKE_SOURCE_DIR}/docs/Iroha_3_sm.png")
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)

  doxygen_add_docs(docs
      ${CMAKE_SOURCE_DIR}/irohad
      ${CMAKE_SOURCE_DIR}/iroha-cli
      ${CMAKE_SOURCE_DIR}/libs
      # ${CMAKE_SOURCE_DIR}/test
      COMMENT "Creating doxygen documentation"
      )
endif()
