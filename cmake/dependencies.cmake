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
find_package(protobuf)

#########################
#         gRPC          #
#########################
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
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.53.0 COMPONENTS multiprecision)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(progname file1.cxx file2.cxx)
    target_link_libraries(progname ${Boost_LIBRARIES})
endif()