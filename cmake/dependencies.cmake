include(ExternalProject)
set(EP_PREFIX "${PROJECT_SOURCE_DIR}/external")
set_directory_properties(PROPERTIES
    EP_PREFIX ${EP_PREFIX}
    )

# Project dependencies.
find_package(Threads REQUIRED)

############################
#         ed25519          #
############################
ExternalProject_Add(mizukisonoko_ed25519
    GIT_REPOSITORY "https://github.com/MizukiSonoko/ed25519.git"
    BUILD_IN_SOURCE 1
    BUILD_COMMAND $(MAKE)
    CONFIGURE_COMMAND "" # remove configure step
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    UPDATE_COMMAND "" # remove update step
    )
ExternalProject_Get_Property(mizukisonoko_ed25519 source_dir)
set(ed25519_SOURCE_DIR "${source_dir}")

add_library(ed25519 STATIC IMPORTED)
file(MAKE_DIRECTORY ${ed25519_SOURCE_DIR}/src)
set_target_properties(ed25519 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${ed25519_SOURCE_DIR}/src
    IMPORTED_LOCATION ${ed25519_SOURCE_DIR}/lib/libed25519.a
    )
add_dependencies(ed25519 mizukisonoko_ed25519)

####################################
#         thread-pool-cpp          #
####################################
ExternalProject_Add(warchant_thread_pool
    GIT_REPOSITORY "https://github.com/Warchant/thread-pool-cpp.git"
    GIT_TAG "a24e0726a7e804c55555fca16bc6f42d7ff4723a"
    BUILD_COMMAND "" # remove build step, header only lib
    CONFIGURE_COMMAND "" # remove configure step
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    UPDATE_COMMAND "" # remove update step
    )
ExternalProject_Get_Property(warchant_thread_pool source_dir)
set(thread_pool_SOURCE_DIR "${source_dir}")

# since it is header only, we changed STATIC to INTERFACE below
add_library(thread_pool INTERFACE IMPORTED)
file(MAKE_DIRECTORY ${thread_pool_SOURCE_DIR}/include)
set_target_properties(thread_pool PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${thread_pool_SOURCE_DIR}/include
    )
add_dependencies(thread_pool warchant_thread_pool)

##########################
#         gtest          #
##########################
# testing is an option. Look at the main CMakeLists.txt for details.
if (TESTING)
  ExternalProject_Add(google_test
      GIT_REPOSITORY "https://github.com/google/googletest.git"
      CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -Dgtest_force_shared_crt=ON
      -Dgtest_disable_pthreads=OFF
      -DBUILD_GTEST=ON
      -DBUILD_GMOCK=OFF
      INSTALL_COMMAND "" # remove install step
      UPDATE_COMMAND "" # remove update step
      TEST_COMMAND "" # remove test step
      )
  ExternalProject_Get_Property(google_test source_dir binary_dir)
  set(gtest_SOURCE_DIR ${source_dir})
  set(gtest_BINARY_DIR ${binary_dir})

  add_library(gtest STATIC IMPORTED)
  file(MAKE_DIRECTORY ${gtest_SOURCE_DIR}/googletest/include)

  set_target_properties(gtest
      PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${gtest_SOURCE_DIR}/googletest/include
      IMPORTED_LINK_INTERFACE_LIBRARIES "pthread;${gtest_BINARY_DIR}/googletest/libgtest_main.a"
      IMPORTED_LOCATION ${gtest_BINARY_DIR}/googletest/libgtest.a
      )
  add_dependencies(gtest google_test)
endif ()

#############################
#         speedlog          #
#############################
ExternalProject_Add(gabime_spdlog
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG "v0.13.0"
    CONFIGURE_COMMAND "" # remove configure step
    BUILD_COMMAND "" # remove build step
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    UPDATE_COMMAND "" # remove update step
    )
ExternalProject_Get_Property(gabime_spdlog source_dir)
set(spdlog_INCLUDE_DIRS ${source_dir}/include)
file(MAKE_DIRECTORY ${spdlog_INCLUDE_DIRS})

add_library(spdlog INTERFACE IMPORTED)
set_target_properties(spdlog PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${spdlog_INCLUDE_DIRS}
    )

add_dependencies(spdlog gabime_spdlog)

# find_package(libFuzz)

#if (NOT LIBFUZZER_FOUND)
#  ExternalProject_Add(libFuzzer
#    GIT_REPOSITORY    "https://chromium.googlesource.com/chromium/llvm-project/llvm/lib/Fuzzer"
#    GIT_TAG           "master"
#    CONFIGURE_COMMAND ""
#    BUILD_IN_SOURCE   1
#    BUILD_ALWAYS      1
#    BUILD_COMMAND     "./build.sh"
#    INSTALL_COMMAND   "" # remove install step
#    TEST_COMMAND      "" # remove test step
#    UPDATE_COMMAND    "" # remove update step
#    )
#  ExternalProject_Get_Property(libFuzzer source_dir)
#  set(LIBFUZZER_LIBRARIES ${source_dir}/libFuzzer.a)
#endif()

#add_library(fuzzer STATIC IMPORTED)
#set_target_properties(fuzzer PROPERTIES
#  IMPORTED_LOCATION ${LIBFUZZER_LIBRARIES}
#  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
#  )

################################
#           protobuf           #
################################
find_package(Protobuf 3.3.0)
if (NOT PROTOBUF_FOUND OR NOT PROTOBUF_PROTOC_EXECUTABLE)
  ExternalProject_Add(google_protobuf
      URL https://github.com/google/protobuf/releases/download/v3.3.0/protobuf-cpp-3.3.0.tar.gz
      CONFIGURE_COMMAND ./configure
      BUILD_IN_SOURCE 1
      BUILD_COMMAND $(MAKE)
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND "" # remove test step
      UPDATE_COMMAND "" # remove update step
      )
  ExternalProject_Get_Property(google_protobuf source_dir)
  set(protobuf_INCLUDE_DIRS ${source_dir}/src)
  set(protobuf_LIBRARIES ${source_dir}/src/.libs/libprotobuf.a)
  set(protoc_EXECUTABLE ${source_dir}/src/protoc)
  file(MAKE_DIRECTORY ${protobuf_INCLUDE_DIRS})
  add_custom_target(protoc DEPENDS google_protobuf)
else ()
  set(protobuf_INCLUDE_DIRS ${PROTOBUF_INCLUDE_DIRS})
  set(protobuf_LIBRARIES ${PROTOBUF_LIBRARY})
  set(protoc_EXECUTABLE ${PROTOBUF_PROTOC_EXECUTABLE})
  add_custom_target(protoc)
endif ()

add_library(protobuf STATIC IMPORTED)
set_target_properties(protobuf PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${protobuf_INCLUDE_DIRS}
    IMPORTED_LOCATION ${protobuf_LIBRARIES}
    )

if (NOT PROTOBUF_FOUND OR NOT PROTOBUF_PROTOC_EXECUTABLE)
  add_dependencies(protobuf google_protobuf protoc)
endif ()

#########################
#         gRPC          #
#########################
find_package(GRPC)
if (NOT GRPC_FOUND)
  ExternalProject_Add(grpc_grpc
      GIT_REPOSITORY "https://github.com/grpc/grpc.git"
      GIT_TAG "v1.3.0"
      BUILD_IN_SOURCE 1
      BUILD_COMMAND $(MAKE)
      CONFIGURE_COMMAND "" # remove configure step
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND "" # remove test step
      UPDATE_COMMAND "" # remove update step
      )
  ExternalProject_Get_Property(grpc_grpc source_dir)
  set(grpc_INCLUDE_DIR ${source_dir}/include)
  set(grpc_LIBRARY ${source_dir}/libs/opt/libgrpc.so)
  set(grpc_grpc++_LIBRARY ${source_dir}/libs/opt/libgrpc++.so)
  set(grpc_grpc++_reflection_LIBRARY ${source_dir}/libs/opt/libgrpc++_reflection.so)
  set(grpc_CPP_PLUGIN ${source_dir})
  file(MAKE_DIRECTORY ${grpc_INCLUDE_DIR})
  add_custom_target(grpc_cpp_plugin DEPENDS grpc_grpc protoc)
else ()
  set(grpc_INCLUDE_DIR ${GRPC_INCLUDE_DIR})
  set(grpc_LIBRARY ${GRPC_LIBRARY})
  set(grpc_grpc++_LIBRARY ${GRPC_GRPC++_LIBRARY})
  set(grpc_grpc++_reflection_LIBRARY ${GRPC_GRPC++_REFLECTION_LIBRARY})
  set(grpc_CPP_PLUGIN ${GRPC_CPP_PLUGIN})
  add_custom_target(grpc_cpp_plugin DEPENDS protoc)
endif ()

# libgrpc
add_library(grpc SHARED IMPORTED)
set_target_properties(grpc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${grpc_INCLUDE_DIR}
    IMPORTED_LOCATION ${grpc_LIBRARY}
    )

# libgrpc++
add_library(grpc++ SHARED IMPORTED)
set_target_properties(grpc++ PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${grpc_INCLUDE_DIR}
    IMPORTED_LOCATION ${grpc_grpc++_LIBRARY}
    )

# libgrpc++_reflection
add_library(grpc++_reflection SHARED IMPORTED)
set_target_properties(grpc++_reflection PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${grpc_INCLUDE_DIR}
    IMPORTED_LOCATION ${grpc_grpc++_reflection_LIBRARY}
    )

if (NOT GRPC_FOUND)
  add_dependencies(grpc grpc_grpc)
  add_dependencies(grpc++ grpc_grpc)
  add_dependencies(grpc++_reflection grpc_grpc)
endif ()

################################
#          rapidjson           #
################################
ExternalProject_Add(miloyip_rapidjson
    GIT_REPOSITORY "https://github.com/miloyip/rapidjson"
    BUILD_COMMAND "" # remove build step, header only lib
    CONFIGURE_COMMAND "" # remove configure step
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    UPDATE_COMMAND "" # remove update step
    )
ExternalProject_Get_Property(miloyip_rapidjson source_dir)
set(rapidjson_INCLUDE_DIR "${source_dir}/include")

add_library(rapidjson INTERFACE IMPORTED)
file(MAKE_DIRECTORY ${rapidjson_INCLUDE_DIR})
set_target_properties(rapidjson PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${rapidjson_INCLUDE_DIR}
    )

add_dependencies(rapidjson miloyip_rapidjson)

#############################
#         optional          #
#############################
ExternalProject_Add(martinmoene_optional
    GIT_REPOSITORY "https://github.com/martinmoene/optional-lite"
    GIT_TAG "v2.0.0"
    CONFIGURE_COMMAND "" # remove configure step
    BUILD_COMMAND "" # remove build step
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    UPDATE_COMMAND "" # remove update step
    )
ExternalProject_Get_Property(martinmoene_optional source_dir)
set(optional_INCLUDE_DIRS ${source_dir}/include)
file(MAKE_DIRECTORY ${optional_INCLUDE_DIRS})

add_library(optional INTERFACE IMPORTED)
set_target_properties(optional PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${optional_INCLUDE_DIRS}
    )

add_dependencies(optional martinmoene_optional)

########################
#         any          #
########################
ExternalProject_Add(martinmoene_any
    GIT_REPOSITORY "https://github.com/martinmoene/any-lite"
    GIT_TAG "v0.0.0"
    CONFIGURE_COMMAND "" # remove configure step
    BUILD_COMMAND "" # remove build step
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    UPDATE_COMMAND "" # remove update step
    )
ExternalProject_Get_Property(martinmoene_any source_dir)
set(any_INCLUDE_DIRS ${source_dir}/include)
file(MAKE_DIRECTORY ${any_INCLUDE_DIRS})

add_library(any INTERFACE IMPORTED)
set_target_properties(any PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${any_INCLUDE_DIRS}
    )

add_dependencies(any martinmoene_any)

################################
#            libuv             #
################################
ExternalProject_Add(libuv_libuv
    GIT_REPOSITORY "https://github.com/libuv/libuv.git"
    GIT_TAG "v1.x"
    CONFIGURE_COMMAND ./autogen.sh && ./configure
    BUILD_IN_SOURCE 1
    BUILD_COMMAND $(MAKE)
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    UPDATE_COMMAND "" # remove update step
    )
ExternalProject_Get_Property(libuv_libuv source_dir)
set(uv_INCLUDE_DIRS ${source_dir}/include)
set(uv_LIBRARIES ${source_dir}/.libs/libuv.a)
file(MAKE_DIRECTORY ${uv_INCLUDE_DIRS})

add_library(uv STATIC IMPORTED)
set_target_properties(uv PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${uv_INCLUDE_DIRS}
    IMPORTED_LOCATION ${uv_LIBRARIES}
    INTERFACE_LINK_LIBRARIES "pthread"
    )

add_dependencies(uv libuv_libuv)

################################
#             uvw              #
################################
ExternalProject_Add(skypjack_uvw
    GIT_REPOSITORY "https://github.com/skypjack/uvw.git"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    UPDATE_COMMAND "" # remove update step
    )
ExternalProject_Get_Property(skypjack_uvw source_dir)
add_dependencies(skypjack_uvw uv)
set(uvw_INCLUDE_DIRS ${source_dir}/src)
file(MAKE_DIRECTORY ${uvw_INCLUDE_DIRS})

add_library(uvw INTERFACE IMPORTED)
set_target_properties(uvw PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${uvw_INCLUDE_DIRS};${uv_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${uv_LIBRARIES};pthread"
    )

add_dependencies(uvw skypjack_uvw)

##########################
#       cpp_redis        #
##########################
find_package(CPP_redis)
if (NOT CPP_redis_FOUND)
  ExternalProject_Add(cylix_cpp_redis
      GIT_REPOSITORY "https://github.com/Cylix/cpp_redis.git"
      CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      INSTALL_COMMAND "" # remove install step
      UPDATE_COMMAND "" # remove update step
      TEST_COMMAND "" # remove test step
      )
  ExternalProject_Get_Property(cylix_cpp_redis source_dir binary_dir)
  set(CPP_REDIS_INCLUDE_DIRS ${source_dir}/includes)
  set(CPP_REDIS_LIBRARIES ${binary_dir}/lib/libcpp_redis.a)
  set(TACOPIE_LIBRARIES ${binary_dir}/lib/libtacopie.a)
  file(MAKE_DIRECTORY ${CPP_REDIS_INCLUDE_DIRS})

  add_library(cpp_redis STATIC IMPORTED)
  set_target_properties(cpp_redis PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${CPP_REDIS_INCLUDE_DIRS}
      IMPORTED_LOCATION ${CPP_REDIS_LIBRARIES}
      IMPORTED_LINK_INTERFACE_LIBRARIES "${TACOPIE_LIBRARIES};pthread"
      )

  add_dependencies(cpp_redis cylix_cpp_redis)
endif ()

##########################
#          pqxx          #
##########################
find_package(PQxx)
if (NOT PQxx_FOUND)
  find_package(PostgreSQL QUIET)
  if (NOT PostgreSQL_LIBRARY)
    message(FATAL_ERROR "libpq not found")
  endif ()

  ExternalProject_Add(jtv_libpqxx
      GIT_REPOSITORY "https://github.com/jtv/libpqxx.git"
      CONFIGURE_COMMAND ./configure --disable-documentation --with-pic
      BUILD_IN_SOURCE 1
      BUILD_COMMAND $(MAKE)
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND "" # remove test step
      UPDATE_COMMAND "" # remove update step
      )
  ExternalProject_Get_Property(jtv_libpqxx source_dir)
  set(pqxx_INCLUDE_DIRS ${source_dir}/include)
  set(pqxx_LIBRARIES ${source_dir}/src/.libs/libpqxx.a)
  file(MAKE_DIRECTORY ${pqxx_INCLUDE_DIRS})

  add_library(pqxx STATIC IMPORTED)
  set_target_properties(pqxx PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${pqxx_INCLUDE_DIRS}
      IMPORTED_LOCATION ${pqxx_LIBRARIES}
      INTERFACE_LINK_LIBRARIES "pq"
      )

  add_dependencies(pqxx jtv_libpqxx)
endif ()

################################
#            gflags            #
################################
externalproject_add(gflags_gflags
    URL "https://github.com/gflags/gflags/archive/v2.2.0.tar.gz"
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    UPDATE_COMMAND "" # remove update step
    )
externalproject_get_property(gflags_gflags binary_dir)
set(gflags_INCLUDE_DIR ${binary_dir}/include)
set(gflags_LIBRARY ${binary_dir}/lib/libgflags.a)

add_library(gflags STATIC IMPORTED)
file(MAKE_DIRECTORY ${gflags_INCLUDE_DIR})

set_target_properties(gflags PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${gflags_INCLUDE_DIR}
    IMPORTED_LOCATION ${gflags_LIBRARY}
    IMPORTED_LINK_INTERFACE_LIBRARIES "pthread"
    )
add_dependencies(gflags gflags_gflags)

##########################
#        rx c++          #
##########################

ExternalProject_Add(reactive_extensions_rxcpp
    GIT_REPOSITORY "https://github.com/Reactive-Extensions/RxCpp"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "" # remove install step
    UPDATE_COMMAND "" # remove update step
    TEST_COMMAND "" # remove test step
    )
ExternalProject_Get_Property(reactive_extensions_rxcpp source_dir binary_dir)
set(RXCPP_INCLUDE_DIRS ${source_dir}/Rx/v2/src)
file(MAKE_DIRECTORY ${RXCPP_INCLUDE_DIRS})

add_library(rxcpp INTERFACE IMPORTED)
set_target_properties(rxcpp PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${RXCPP_INCLUDE_DIRS}
    )

add_dependencies(rxcpp reactive_extensions_rxcpp)