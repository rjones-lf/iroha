include(ExternalProject)
set(EP_PREFIX "${PROJECT_SOURCE_DIR}/external")
set_directory_properties(PROPERTIES
  EP_PREFIX ${EP_PREFIX}
)

# Project dependencies.
find_package(Threads REQUIRED)


###########################
#         keccak          #
###########################
find_package(LibXslt QUIET)
if (NOT LIBXSLT_XSLTPROC_EXECUTABLE)
  message(FATAL_ERROR "xsltproc not found")
endif ()

ExternalProject_Add(gvanas_keccak
  GIT_REPOSITORY    "https://github.com/gvanas/KeccakCodePackage.git"
  BUILD_IN_SOURCE   1
  BUILD_COMMAND     bash -c "CFLAGS='-fPIC -DKeccakP200_excluded -DKeccakP400_excluded -DKeccakP800_excluded'\
    $(MAKE) CC='${CMAKE_C_COMPILER}' generic64/libkeccak.a"
  CONFIGURE_COMMAND "" # remove configure step
  INSTALL_COMMAND   "" # remove install step
  TEST_COMMAND      "" # remove test step
  UPDATE_COMMAND    "" # remove update step
)
ExternalProject_Get_Property(gvanas_keccak source_dir)
set(keccak_SOURCE_DIR "${source_dir}")

# Hash internals for keccak
add_library(keccak STATIC IMPORTED)
file(MAKE_DIRECTORY ${keccak_SOURCE_DIR}/bin/generic64/libkeccak.a.headers)
set_target_properties(keccak PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${keccak_SOURCE_DIR}/bin/generic64/libkeccak.a.headers"
  IMPORTED_LOCATION "${keccak_SOURCE_DIR}/bin/generic64/libkeccak.a"
  )

add_dependencies(keccak gvanas_keccak)



############################
#         ed25519          #
############################
ExternalProject_Add(mizukisonoko_ed25519
  GIT_REPOSITORY    "https://github.com/MizukiSonoko/ed25519.git"
  BUILD_IN_SOURCE   1
  BUILD_COMMAND     $(MAKE)
  CONFIGURE_COMMAND "" # remove configure step
  INSTALL_COMMAND   "" # remove install step
  TEST_COMMAND      "" # remove test step
  UPDATE_COMMAND    "" # remove update step
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
  GIT_REPOSITORY    "https://github.com/Warchant/thread-pool-cpp.git"
  BUILD_COMMAND     "" # remove build step, header only lib
  CONFIGURE_COMMAND "" # remove configure step
  INSTALL_COMMAND   "" # remove install step
  TEST_COMMAND      "" # remove test step
  UPDATE_COMMAND    "" # remove update step
  )
ExternalProject_Get_Property(warchant_thread_pool source_dir)
set(thread_pool_SOURCE_DIR "${source_dir}")

# since it is header only, we changed STATIC to INTERFACE below
add_library(thread_pool INTERFACE IMPORTED)
file(MAKE_DIRECTORY ${thread_pool_SOURCE_DIR}/thread_pool)
set_target_properties(thread_pool PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${thread_pool_SOURCE_DIR}/thread_pool
  )
add_dependencies(thread_pool warchant_thread_pool)



#########################
#         json          #
#########################
ExternalProject_Add(nlohmann_json
  GIT_REPOSITORY    "https://github.com/nlohmann/json.git"
  BUILD_COMMAND     "" # remove build step, header only lib
  CONFIGURE_COMMAND "" # remove configure step
  INSTALL_COMMAND   "" # remove install step
  TEST_COMMAND      "" # remove test step
  UPDATE_COMMAND    "" # remove update step
  )
ExternalProject_Get_Property(nlohmann_json source_dir)
set(json_SOURCE_DIR "${source_dir}")

# since it is header only, we changed STATIC to INTERFACE below
add_library(json INTERFACE IMPORTED)
file(MAKE_DIRECTORY ${json_SOURCE_DIR}/src)
set_target_properties(json PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${json_SOURCE_DIR}/src
  )
add_dependencies(json nlohmann_json)



##########################
#         gtest          #
##########################
# testing is an option. Look at the main CMakeLists.txt for details.
if(TESTING)
  ExternalProject_Add(google_test
    GIT_REPOSITORY    "https://github.com/google/googletest.git"
    CMAKE_ARGS        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                      -Dgtest_force_shared_crt=ON
                      -Dgtest_disable_pthreads=OFF
                      -DBUILD_GTEST=ON
                      -DBUILD_GMOCK=OFF
    INSTALL_COMMAND   "" # remove install step
    UPDATE_COMMAND    "" # remove update step
    TEST_COMMAND      "" # remove test step
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
endif()


###############################
#         Cappuccino          #
###############################
ExternalProject_Add(mizukisonoko_cappuccino
  GIT_REPOSITORY    "https://github.com/MizukiSonoko/Cappuccino.git"
  BUILD_COMMAND     "" # remove build step, header only lib
  CONFIGURE_COMMAND "" # remove configure step
  INSTALL_COMMAND   "" # remove install step
  TEST_COMMAND      "" # remove test step
  UPDATE_COMMAND    "" # remove update step
  )
ExternalProject_Get_Property(mizukisonoko_cappuccino source_dir)
set(cappuccino_SOURCE_DIR "${source_dir}")

# since it is header only, we changed STATIC to INTERFACE below
add_library(cappuccino INTERFACE IMPORTED)
set_target_properties(cappuccino PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${cappuccino_SOURCE_DIR}
  )

add_dependencies(cappuccino mizukisonoko_cappuccino json)



##############################
#         benchmark          #
##############################
# benchmarking is an option. Look at main CMakeLists.txt file for details.
if(BENCHMARKING)
  ExternalProject_Add(google_benchmark
    GIT_REPOSITORY    "https://github.com/google/benchmark.git"
    CMAKE_ARGS        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                      -DBENCHMARK_ENABLE_TESTING=OFF
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_COMMAND   "" # remove install step
    UPDATE_COMMAND    "" # remove update step
    TEST_COMMAND      "" # remove test step
    )
  ExternalProject_Get_Property(google_benchmark source_dir binary_dir)
  set(benchmark_SOURCE_DIR ${source_dir})
  set(benchmark_BINARY_DIR ${binary_dir})

  add_library(benchmark STATIC IMPORTED)
  file(MAKE_DIRECTORY ${benchmark_SOURCE_DIR}/include)
  set_target_properties(benchmark PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${benchmark_SOURCE_DIR}/include
    IMPORTED_LINK_INTERFACE_LIBRARIES "pthread"
    IMPORTED_LOCATION ${benchmark_BINARY_DIR}/src/libbenchmark.a
    )
  add_dependencies(benchmark google_benchmark)
endif(BENCHMARKING)



############################
#         leveldb          #
############################
ExternalProject_Add(google_leveldb
  GIT_REPOSITORY    "https://github.com/google/leveldb.git"
  BUILD_IN_SOURCE   1
  BUILD_COMMAND     $(MAKE) -j1 OPT=-fPIC
  CONFIGURE_COMMAND "" # remove configure step
  INSTALL_COMMAND   "" # remove install step
  TEST_COMMAND      "" # remove test step
  UPDATE_COMMAND    "" # remove update step
  )
ExternalProject_Get_Property(google_leveldb source_dir)
set(leveldb_SOURCE_DIR "${source_dir}")

add_library(leveldb STATIC IMPORTED)
file(MAKE_DIRECTORY ${leveldb_SOURCE_DIR}/include)
file(MAKE_DIRECTORY ${leveldb_SOURCE_DIR}/out-shared)
file(MAKE_DIRECTORY ${leveldb_SOURCE_DIR}/out-static)
set_target_properties(leveldb PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${leveldb_SOURCE_DIR}/include
  IMPORTED_LINK_INTERFACE_LIBRARIES "snappy"
  IMPORTED_LOCATION ${leveldb_SOURCE_DIR}/out-static/libleveldb.a
  )
add_dependencies(leveldb google_leveldb)



########################################################
# JNI
########################################################

if (DEFINED ENV{JAVA_HOME})
  # include_directories($ENV{JAVA_HOME}/include/)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(_JVM_INCLUDE_PATH "$ENV{JAVA_HOME}/include/darwin")
    #include_directories(
    #  $ENV{JAVA_HOME}/include/darwin/
    #  ${PROJECT_SOURCE_DIR}/core
    #)
    #link_directories(
    #  $ENV{JAVA_HOME}/jre/lib/server
    #  $ENV{JAVA_HOME}/jre/lib/amd64/server
    #)
  else()
    set(_JVM_INCLUDE_PATH "$ENV{JAVA_HOME}/include/linux")
    #include_directories(
    #  $ENV{JAVA_HOME}/include/linux
    #)
    #link_directories(
    #  $ENV{JAVA_HOME}/jre/lib/server
    #  $ENV{JAVA_HOME}/jre/lib/amd64/server/
    #)
  endif()

  add_library(jvm STATIC IMPORTED)
  set_target_properties(jvm PROPERTIES
    # $ENV{JAVA_HOME}/include;
    INCLUDE_DIRECTORIES "${_JVM_INCLUDE_PATH};$ENV{JAVA_HOME}/include"
    # is it necessary to link x32 + x64 libs?
    IMPORTED_LINK_INTERFACE_LIBRARIES "$ENV{JAVA_HOME}/jre/lib/server;$ENV{JAVA_HOME}/jre/lib/amd64/server"
  )
  #target_include_directories(jvm INTERFACE $ENV{JAVA_HOME}/include)
else()
  find_package(JNI)
  # link_directories(${JAVA_JVM_LIBRARY})
  include_directories(${JAVA_INCLUDE_PATH})
  if (!JNI_FOUND)
    message(FATAL_ERROR "JVM not found")
  endif()
endif()
