FROM ubuntu:16.04

# number of concurrent threads during build
# usage: docker build --build-arg PARALLELISM=8 -t name/name .
ARG PARALLELISM=1

RUN apt-get update; \
    apt-get -y upgrade; \
    apt-get -y --no-install-recommends install apt-utils; \
    apt-get -y clean

RUN apt-get -y --no-install-recommends install build-essential python-software-properties \
        automake libtool \
        # dev dependencies
        libssl-dev libc-ares-dev libboost-all-dev libc6-dbg \
        # CircleCI dependencies
        git ssh tar gzip ca-certificates python3 python3-pip python3-setuptools \
        # code coverage
        lcov \
        # other
        wget curl cmake file unzip gdb iputils-ping vim ccache gcovr vera++ cppcheck; \
    apt-get -y clean

RUN pip3 install --upgrade pip
RUN pip3 install flask requests

# install protobuf 3.3.0
ENV PROTOBUF_VERSION=3.3.0
RUN git clone https://github.com/google/protobuf /tmp/protobuf-${PROTOBUF_VERSION}; \
    (cd /tmp/protobuf-${PROTOBUF_VERSION} ; git checkout a6189acd18b00611c1dc7042299ad75486f08a1a); \
    (cd /tmp/protobuf-${PROTOBUF_VERSION}/ ; /tmp/protobuf-${PROTOBUF_VERSION}/autogen.sh); \
    (cd /tmp/protobuf-${PROTOBUF_VERSION}/ ; /tmp/protobuf-${PROTOBUF_VERSION}/configure); \
    make -j${PARALLELISM} -C /tmp/protobuf-${PROTOBUF_VERSION}; \
    make -j${PARALLELISM} -C /tmp/protobuf-${PROTOBUF_VERSION} install; \
    ldconfig; \
    rm -rf /tmp/protobuf-${PROTOBUF_VERSION}

# install grpc 1.3.2
ENV GRPC_VERSION=1.3.2
RUN git clone https://github.com/grpc/grpc /tmp/grpc-${GRPC_VERSION}; \
    (cd /tmp/grpc-${GRPC_VERSION} ; git checkout c80d3321d0f77bef8cfff8b32490a07c1e90a5ad); \
    make -j${PARALLELISM} -C /tmp/grpc-${GRPC_VERSION}; \
    make -j${PARALLELISM} -C /tmp/grpc-${GRPC_VERSION} install; \
    ldconfig; \
    rm -rf /tmp/grpc-${GRPC_VERSION}

# install gtest 1.8.0
ENV GTEST_VERSION=1.8.0
RUN git clone https://github.com/google/googletest /tmp/googletest-release-${GTEST_VERSION}; \
    (cd /tmp/googletest-release-${GTEST_VERSION} ; git checkout ec44c6c1675c25b9827aacd08c02433cccde7780); \
    cmake -H/tmp/googletest-release-${GTEST_VERSION} -B/tmp/googletest-release-${GTEST_VERSION}/build; \
    cd /tmp/googletest-release-${GTEST_VERSION}/build; \
    make -j${PARALLELISM} install; \
    cd -; \
    rm -rf /tmp/googletest-release-${GTEST_VERSION}

# install gflags 2.2.0
ENV CFLAGS_VERSION=2.2.0
RUN git clone https://github.com/gflags/gflags /tmp/gflags-${GFLAGS_VERSION}; \
    (cd /tmp/gflags-${GFLAGS_VERSION} ; git checkout f8a0efe03aa69b3336d8e228b37d4ccb17324b88); \
    cmake -H/tmp/gflags-${GFLAGS_VERSION} -B/tmp/gflags-${GFLAGS_VERSION}/build; \
    cd /tmp/gflags-${GFLAGS_VERSION}/build; \
    make -j${PARALLELISM} install; \
    cd -; \
    rm -rf /tmp/gflags-${GFLAGS_VERSION}

# install spdlog 0.13.0
ENV SPDLOG_VERSION=0.13.0
RUN git clone https://github.com/gabime/spdlog /tmp/spdlog-${SPDLOG_VERSION}; \
    (cd /tmp/spdlog-${SPDLOG_VERSION} ; git checkout f85a08622e20b74bff34381cafcb8ef8167b29d0); \
    cmake -DSPDLOG_BUILD_TESTING=OFF -H/tmp/spdlog-${SPDLOG_VERSION} -B/tmp/spdlog-${SPDLOG_VERSION}/build; \
    cd /tmp/spdlog-${SPDLOG_VERSION}/build; \
    make -j${PARALLELISM} install; \
    cd -; \
    rm -rf /tmp/spdlog-${SPDLOG_VERSION}

# install rxcpp 4.0.0, apply patch for include path
ENV RXCPP_VERSION=4.0.0
RUN git clone https://github.com/Reactive-Extensions/RxCpp /tmp/RxCpp-${RXCPP_VERSION}; \
    (cd /tmp/RxCpp-${RXCPP_VERSION} ; git checkout 08c47e42930168cedf76037f8c76d47565251599); \
    (cd /tmp/RxCpp-${RXCPP_VERSION} ; git cherry-pick -n 5b7b2a5519f712a7831053865a4ae6082dac4ed5); \
    cmake -H/tmp/RxCpp-${RXCPP_VERSION} -B/tmp/RxCpp-${RXCPP_VERSION}/build; \
    cd /tmp/RxCpp-${RXCPP_VERSION}/build; \
    make -j${PARALLELISM} install; \
    cd -; \
    rm -rf /tmp/RxCpp-${RXCPP_VERSION}

# install ed25519 and sha3
RUN git clone https://github.com/MizukiSonoko/ed25519 /tmp/ed25519; \
    (cd /tmp/ed25519 ; git checkout 3a5c02cbd91f84983a1622ed6bc14de6264b8361); \
    make -j${PARALLELISM} -C /tmp/ed25519; \
    cp /tmp/ed25519/lib/* /usr/local/lib; \
    cp /tmp/ed25519/src/sha3.h /usr/local/include; \
    cp /tmp/ed25519/src/ed25519.h /usr/local/include; \
    rm -rf /tmp/ed25519

# install rapidjson 1.1.0
ENV RAPIDJSON_VERSION=1.1.0
RUN git clone https://github.com/miloyip/rapidjson /tmp/rapidjson-${RAPIDJSON_VERSION}; \
    (cd /tmp/rapidjson-${RAPIDJSON_VERSION} ; git checkout f54b0e47a08782a6131cc3d60f94d038fa6e0a51); \
    cmake -DRAPIDJSON_BUILD_EXAMPLES=OFF -H/tmp/rapidjson-${RAPIDJSON_VERSION} -B/tmp/rapidjson-${RAPIDJSON_VERSION}/build; \
    cd /tmp/rapidjson-${RAPIDJSON_VERSION}/build; \
    make -j${PARALLELISM} install; \
    cd -; \
    rm -rf /tmp/rapidjson-${RAPIDJSON_VERSION}

# install optional 2.1.0
ENV OPTIONAL_VERSION=2.1.0
RUN git clone https://github.com/martinmoene/optional-lite /tmp/optional-lite-${OPTIONAL_VERSION}; \
    (cd /tmp/optional-lite-${OPTIONAL_VERSION} ; git checkout a0ddabb8b52e1eaaf0dd1515bb85698b747528e4); \
    cp -r /tmp/optional-lite-${OPTIONAL_VERSION}/include/nonstd /usr/local/include; \
    rm -rf /tmp/optional-lite-${OPTIONAL_VERSION}

# install libpq 9.5.8
ENV POSTGRESQL_VERSION=9.5.8
RUN git clone https://git.postgresql.org/git/postgresql.git /tmp/postgresql-${POSTGRESQL_VERSION}; \
    (cd /tmp/postgresql-${POSTGRESQL_VERSION} ; git checkout 029386ccbddd0a33d481b94e511f5219b03e6636); \
    (cd /tmp/postgresql-${POSTGRESQL_VERSION} ; /tmp/postgresql-${POSTGRESQL_VERSION}/configure --without-readline --prefix=/usr/local); \
    make -C /tmp/postgresql-${POSTGRESQL_VERSION}/src/bin/pg_config; \
    make -C /tmp/postgresql-${POSTGRESQL_VERSION}/src/interfaces/libpq; \
    make -C /tmp/postgresql-${POSTGRESQL_VERSION}/src/bin/pg_config install; \
    make -C /tmp/postgresql-${POSTGRESQL_VERSION}/src/interfaces/libpq install; \
    make -C /tmp/postgresql-${POSTGRESQL_VERSION}/src/include install; \
    rm -rf /tmp/postgresql-${POSTGRESQL_VERSION}

# install pqxx 5.1.0
ENV PQXX_VERSION=5.1.0
RUN git clone https://github.com/jtv/libpqxx /tmp/libpqxx-${PQXX_VERSION}; \
    (cd /tmp/libpqxx-${PQXX_VERSION} ; git checkout 5b17abce5ac2b1a2f8278718405b7ade8bb30ae9); \
    (cd /tmp/libpqxx-${PQXX_VERSION} ; /tmp/libpqxx-${PQXX_VERSION}/configure --disable-documentation --with-pic); \
    make -j${PARALLELISM} -C /tmp/libpqxx-${PQXX_VERSION}; \
    make -j${PARALLELISM} -C /tmp/libpqxx-${PQXX_VERSION} install; \
    rm -rf /tmp/libpqxx-${PQXX_VERSION}

# install cpp_redis 3.5.3 with tacopie 2.4.4
ENV CPP_REDIS_VERSION=3.5.3
RUN git clone https://github.com/Cylix/cpp_redis /tmp/cpp_redis-${CPP_REDIS_VERSION}; \
    (cd /tmp/cpp_redis-${CPP_REDIS_VERSION} ; git checkout 727aa5f06c8ce498168cbab5a023cad5b9c00bc0); \
    git clone https://github.com/Cylix/tacopie /tmp/cpp_redis-${CPP_REDIS_VERSION}/tacopie; \
    (cd /tmp/cpp_redis-${CPP_REDIS_VERSION}/tacopie ; git checkout 290dc38681f346adae41d3cc8feabbe534424675); \
    cmake -H/tmp/cpp_redis-${CPP_REDIS_VERSION} -B/tmp/cpp_redis-${CPP_REDIS_VERSION}/build; \
    cd /tmp/cpp_redis-${CPP_REDIS_VERSION}/build; \
    make -j${PARALLELISM} install; \
    cd -; \
    rm -rf /tmp/cpp_redis-${CPP_REDIS_VERSION}

# install libuv 1.13.1
ENV LIBUV_VERSION=1.13.1
RUN git clone https://github.com/libuv/libuv /tmp/libuv-${LIBUV_VERSION}; \
    (cd /tmp/libuv-${LIBUV_VERSION} ; git checkout 2bb4b68758f07cd8617838e68c44c125bc567ba6); \
    /tmp/libuv-${LIBUV_VERSION}/autogen.sh; \
    (cd /tmp/libuv-${LIBUV_VERSION} ; /tmp/libuv-${LIBUV_VERSION}/configure); \
    make -j${PARALLELISM} -C /tmp/libuv-${LIBUV_VERSION}; \
    make -j${PARALLELISM} -C /tmp/libuv-${LIBUV_VERSION} install; \
    rm -rf /tmp/libuv-${LIBUV_VERSION}

# install uvw 1.1.0
ENV UVW_VERSION=1.1.0_libuv-v1.13
RUN git clone https://github.com/skypjack/uvw /tmp/uvw-${UVW_VERSION}; \
    (cd /tmp/uvw-${UVW_VERSION} ; git checkout 00de1f1110ce4a9803a85a214af5326529f11312); \
    cp -r /tmp/uvw-${UVW_VERSION}/src/* /usr/local/include; \
    rm -rf /tmp/uvw-${UVW_VERSION}

# install tbb
ENV TBB_VERSION=2017_U7
RUN git clone https://github.com/01org/tbb /tmp/tbb-${TBB_VERSION}; \
    (cd /tmp/tbb-${TBB_VERSION} ; git checkout eb6336ad29450f2a64af5123ca1b9429ff6bc11d); \
    make -j${PARALLELISM} -C /tmp/tbb-${TBB_VERSION} tbb_build_prefix=build; \
    cp /tmp/tbb-${TBB_VERSION}/build/build_debug/*.so* /usr/local/lib; \
    cp /tmp/tbb-${TBB_VERSION}/build/build_release/*.so* /usr/local/lib; \
    cp -r /tmp/tbb-${TBB_VERSION}/include/* /usr/local/include; \
    ldconfig; \
    rm -rf /tmp/tbb-${TBB_VERSION}

# install docker
ENV DOCKER_VERSION=17.06.0-ce
RUN curl -L -o /tmp/docker-${DOCKER_VERSION}.tgz https://download.docker.com/linux/static/stable/x86_64/docker-${DOCKER_VERSION}.tgz; \
    tar -xz -C /tmp -f /tmp/docker-${DOCKER_VERSION}.tgz; \
    mv /tmp/docker/* /usr/bin; \
    rm /tmp/docker-${DOCKER_VERSION}.tgz; \
    rm -rf /tmp/docker

# install sonar cli
ENV SONAR_CLI_VERSION=3.0.3.778
RUN mkdir -p /opt/sonar; \
    curl -L -o /tmp/sonar.zip https://sonarsource.bintray.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-${SONAR_CLI_VERSION}-linux.zip; \
    unzip -o -d /tmp/sonar-scanner /tmp/sonar.zip; \
    mv /tmp/sonar-scanner/sonar-scanner-${SONAR_CLI_VERSION}-linux /opt/sonar/scanner; \
    cd /usr/local/bin; \
    ln -s -f /opt/sonar/scanner/bin/sonar-scanner sonar-scanner; \
    rm -rf /tmp/sonar*

# install sonar build wrapper
RUN curl -L -o /tmp/sonar-build-wrapper.zip https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip; \
    unzip -o -d /tmp/sonar-build-wrapper /tmp/sonar-build-wrapper.zip; \
    mv /tmp/sonar-build-wrapper/build-wrapper-linux-x86 /opt/sonar/build-wrapper; \
    cd /usr/local/bin; \
    ln -s -f /opt/sonar/build-wrapper/build-wrapper-linux-x86-64 sonar-build-wrapper; \
    rm -rf /tmp/sonar*

# install latest valgrind
RUN cd /tmp; git clone git://sourceware.org/git/valgrind.git; \
    cd /tmp/valgrind; \
    ./autogen.sh; ./configure; make -j${PARALLELISM};\
    make install; \
    rm -rf /tmp/valgrind*

WORKDIR /opt/iroha
CMD ["/bin/bash"]
