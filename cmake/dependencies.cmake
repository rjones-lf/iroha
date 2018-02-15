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
# testing is an option. Look at the main policies.cmake for details.
if (TESTING)
  hunter_add_package(GTest)

  find_package(GTest 1.8.0 CONFIG REQUIRED)
  find_package(GMock 1.8.0 CONFIG REQUIRED)
endif ()

#############################
#         speedlog          #
#############################
hunter_add_package(spdlog)
# TODO: create package with 0.16+
find_package(spdlog CONFIG REQUIRED)

################################
#           protobuf           #
################################
# TODO: hunterize protobuf
option(FIND_PROTOBUF "Try to find protobuf in system" ON)
find_package(protobuf)

#########################
#         gRPC          #
#########################
# TODO: hunterize grpc
option(FIND_GRPC "Try to find gRPC in system" ON)
find_package(grpc)

################################
#          rapidjson           #
################################
hunter_add_package(RapidJSON)
find_package(RapidJSON 1.1.0 CONFIG REQUIRED)

#############################
#         optional          #
#############################
# TODO: hunterize optional
find_package(optional)

##########################
#           pq           #
##########################
# TODO: hunterize pq + pqxx
find_package(pq)
#hunter_add_package(PostgreSQL)
#find_package(PostgreSQL 9.5 REQUIRED)

##########################a
#          pqxx          #
##########################
find_package(pqxx)

################################
#            gflags            #
################################
# TODO: check that gflags works on all OS
#hunter_add_package(gflags)
#find_package(gflags 2.2 CONFIG REQUIRED)
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
set(BOOST_VERSION 1.65.1)
hunter_add_package(Boost COMPONENTS
    ${BOOST_COMPONENTS}
    )
find_package(Boost ${BOOST_VERSION} CONFIG REQUIRED
    COMPONENTS
    ${BOOST_COMPONENTS}
    )


#add_library(boost INTERFACE IMPORTED)
#set_target_properties(boost PROPERTIES
#    INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS}
#    INTERFACE_LINK_LIBRARIES "${Boost_LIBRARIES}"
#    )
#
#if(ENABLE_LIBS_PACKAGING)
#  foreach (library ${Boost_LIBRARIES})
#    add_install_step_for_lib(${library})
#  endforeach(library)
#endif()

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
