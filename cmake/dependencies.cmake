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

##########################
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
# boost multiprecision   #
##########################
find_package(Boost REQUIRED)
