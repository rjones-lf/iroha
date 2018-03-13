find_package(PackageHandleStandardArgs)

include(ExternalProject)
set(EP_PREFIX "${PROJECT_SOURCE_DIR}/external")
set_directory_properties(PROPERTIES
    EP_PREFIX ${EP_PREFIX}
    )


###################################################################
##### START OF PROJECT DEPENDENCIES

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
set(BOOST_COMPONENTS
    filesystem
    system
    )
hunter_add_package(Boost COMPONENTS
    ${BOOST_COMPONENTS}
    )
find_package(Boost 1.65.1 CONFIG REQUIRED
    COMPONENTS
    ${BOOST_COMPONENTS}
    )

##########################
#       benchmark        #
##########################
if(BENCHMARKING)
  find_package(benchmark)
endif()

###################################
#          ed25519/sha3           #
###################################
find_package(ed25519)
