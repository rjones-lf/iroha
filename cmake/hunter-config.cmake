# gtest
hunter_config(GTest
  VERSION
    1.8.0-hunter-p11   # latest hunterized version
  CMAKE_ARGS
    gtest_force_shared_crt=ON
    gtest_disable_pthreads=OFF
  )


# spdlog
hunter_config(spdlog
  VERSION
    0.13.0-p1          # lataest hunterized version
  CMAKE_ARGS
    SPDLOG_BUILD_EXAMPLES=OFF
    SPDLOG_BUILD_TESTS=OFF
  )

# rapidjson
hunter_config(RapidJSON
  VERSION
    1.1.0
  CMAKE_ARGS
    RAPIDJSON_BUILD_DOC=OFF
    RAPIDJSON_BUILD_EXAMPLES=OFF
    RAPIDJSON_BUILD_TESTS=OFF
    RAPIDJSON_BUILD_THIRDPARTY_GTEST=OFF
    RAPIDJSON_BUILD_CXX11=ON
    RAPIDJSON_BUILD_ASAN=OFF
    RAPIDJSON_BUILD_UBSAN=OFF
  )

# boost
hunter_config(Boost
  VERSION
    1.65.1
  )


# postgresql
hunter_config(PostgreSQL
  VERSION
    9.5.0
  CMAKE_ARGS
    EXTRA_FLAGS=--without-readline
  )


# gflags
if(BUILD_SHARED_LIBS)
  set(GFLAGS_BUILD_SHARED_LIBS TRUE)
  set(GFLAGS_BUILD_STATIC_LIBS FALSE)
else()
  set(GFLAGS_BUILD_SHARED_LIBS FALSE)
  set(GFLAGS_BUILD_STATIC_LIBS TRUE)
endif()

hunter_config(gflags
  VERSION
    2.2.1
  CMAKE_ARGS
    GFLAGS_IS_SUBPROJECT=TRUE
    BUILD_SHARED_LIBS=${GFLAGS_BUILD_SHARED_LIBS}
    BUILD_STATIC_LIBS=${GFLAGS_BUILD_STATIC_LIBS}
    BUILD_gflags_LIB=FALSE           # disable build of multithreaded gflags lib
    BUILD_gflags_nothreads_LIB=TRUE  # enable build of singlethreaded gflags lib
    BUILD_PACKAGING=FALSE
    BUILD_TESTING=FALSE
    INSTALL_HEADERS=FALSE
    REGISTER_INSTALL_PREFIX=OFF      # required!
  )


## Protobuf
## TODO: latest releast is 3.5, we can hunterize it
#hunter_config(Protobuf
#    VERSION
#      3.3.0               # latest hunterized version
#    CMAKE_ARGS
#      -Dprotobuf_BUILD_TESTS=OFF
#      -Dprotobuf_BUILD_SHARED_LIBS=OFF
#    )


## gRPC
#hunter_config(gRPC
#    VERSION
#      1.8.1
#    CMAKE_ARGS
#      -DgRPC_PROTOBUF_PROVIDER=package
#      -DgRPC_PROTOBUF_PACKAGE_TYPE=CONFIG
#      -DProtobuf_DIR=${EP_PREFIX}/src/google_protobuf-build/lib/cmake/protobuf
#      -DgRPC_ZLIB_PROVIDER=package
#      -DBUILD_SHARED_LIBS=ON
#    )

