FROM ubuntu:16.04

# number of concurrent threads during build
# usage: docker build --build-arg PARALLELISM=8 -t name/name .
ARG PARALLELISM=1
ARG CMAKE_BUILD_TYPE=Release

ENV IROHA_HOME /opt/iroha
ENV IROHA_BUILD /opt/iroha/build

RUN apt-get update; \
    apt-get -y upgrade; \
    apt-get -y --no-install-recommends install apt-utils software-properties-common; \
    apt-get -y clean


# add git repository
RUN add-apt-repository -y ppa:git-core/ppa; \
    apt-get update


RUN apt-get -y --no-install-recommends install build-essential python-software-properties \
        automake libtool \
        # dev dependencies
        libssl-dev zlib1g-dev libc6-dbg golang \
        # CircleCI dependencies
        git ssh tar gzip ca-certificates python3 python3-pip python3-setuptools \
        # other
        wget curl file unzip gdb iputils-ping vim ccache \
        gcovr cppcheck doxygen graphviz graphviz-dev; \
    apt-get -y clean

# install cmake 3.5.1
RUN git clone https://gitlab.kitware.com/cmake/cmake.git /tmp/cmake; \
    (cd /tmp/cmake ; git checkout 64130a7e793483e24c1d68bdd234f81d5edb2d51); \
    (cd /tmp/cmake ; /tmp/cmake/bootstrap --parallel=${PARALLELISM} --enable-ccache); \
    make -j${PARALLELISM} -C /tmp/cmake; \
    make -C /tmp/cmake install; \
    rm -rf /tmp/cmake

# install boost 1.65.1
RUN git clone https://github.com/boostorg/boost /tmp/boost; \
    (cd /tmp/boost ; git checkout 436ad1dfcfc7e0246141beddd11c8a4e9c10b146); \
    (cd /tmp/boost ; git submodule update --init --recursive); \
    (cd /tmp/boost ; /tmp/boost/bootstrap.sh --with-libraries=system,filesystem); \
    (cd /tmp/boost ; /tmp/boost/b2 headers); \
    (cd /tmp/boost ; /tmp/boost/b2 cxxflags="-std=c++14" -j ${PARALLELISM} install); \
    rm -rf /tmp/boost

# install protobuf
RUN git clone https://github.com/google/protobuf /tmp/protobuf; \
    (cd /tmp/protobuf ; git checkout 80a37e0782d2d702d52234b62dd4b9ec74fd2c95); \
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_SHARED_LIBS=ON \
        -H/tmp/protobuf/cmake \
        -B/tmp/protobuf/.build; \
    cmake --build /tmp/protobuf/.build --target install -- -j${PARALLELISM}

# install gflags
RUN git clone https://github.com/gflags/gflags /tmp/gflags; \
    (cd /tmp/gflags ; git checkout f8a0efe03aa69b3336d8e228b37d4ccb17324b88); \
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -H/tmp/gflags \
        -B/tmp/gflags/build; \
    cmake --build /tmp/gflags/build --target install -- -j${PARALLELISM}; \
    rm -rf /tmp/gflags

# install c-ares
RUN git clone https://github.com/c-ares/c-ares /tmp/c-ares; \
    (cd /tmp/c-ares ; git checkout 3be1924221e1326df520f8498d704a5c4c8d0cce); \
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -H/tmp/c-ares \
        -B/tmp/c-ares/build; \
    cmake --build /tmp/c-ares/build --target install -- -j${PARALLELISM}; \
    rm -rf /tmp/c-ares

# install grpc
RUN git clone https://github.com/grpc/grpc /tmp/grpc; \
    cd /tmp/grpc; \
    git checkout bfcbad3b86c7912968dc8e64f2121c920dad4dfb; \
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -DgRPC_BENCHMARK_PROVIDER="" \
        -DgRPC_ZLIB_PROVIDER=package \
        -DgRPC_CARES_PROVIDER=package \
        -DgRPC_SSL_PROVIDER=package \
        -DgRPC_PROTOBUF_PROVIDER=package \
        -DgRPC_GFLAGS_PROVIDER=package \
        -DBUILD_SHARED_LIBS=ON \
        -H/tmp/grpc \
        -B/tmp/grpc/.build; \
    cd /tmp/grpc/.build; \
    make -j${PARALLELISM} gpr grpc grpc++ grpc_cpp_plugin; \
    cp lib{gpr,grpc,grpc++,grpc_plugin_support}.so /usr/local/lib; \
    cp grpc_cpp_plugin /usr/local/bin; \
    rm -rf /tmp/grpc

# install gtest
RUN git clone https://github.com/google/googletest /tmp/googletest; \
    (cd /tmp/googletest ; git checkout ec44c6c1675c25b9827aacd08c02433cccde7780); \
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -H/tmp/googletest \
        -B/tmp/googletest/build; \
    cmake --build /tmp/googletest/build --target install -- -j${PARALLELISM}; \
    rm -rf /tmp/googletest

# install spdlog v0.16.3
RUN git clone https://github.com/gabime/spdlog /tmp/spdlog; \
    (cd /tmp/spdlog ; git checkout ccd675a286f457068ee8c823f8207f13c2325b26); \
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -DSPDLOG_BUILD_TESTING=OFF -H/tmp/spdlog -B/tmp/spdlog/build; \
    cmake --build /tmp/spdlog/build --target install; \
    rm -rf /tmp/spdlog

# install rxcpp
RUN git clone https://github.com/Reactive-Extensions/RxCpp /tmp/RxCpp; \
    (cd /tmp/RxCpp ; git checkout 1b2e0589f19cb34d8cd58803677701dcf2161876); \
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -H/tmp/RxCpp \
        -B/tmp/RxCpp/build; \
    cmake --build /tmp/RxCpp/build --target install; \
    rm -rf /tmp/RxCpp

# install rapidjson
RUN git clone https://github.com/miloyip/rapidjson /tmp/rapidjson; \
    (cd /tmp/rapidjson ; git checkout f54b0e47a08782a6131cc3d60f94d038fa6e0a51); \
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -DRAPIDJSON_BUILD_EXAMPLES=OFF \
        -H/tmp/rapidjson \
        -B/tmp/rapidjson/build; \
    cmake --build /tmp/rapidjson/build --target install; \
    rm -rf /tmp/rapidjson

# install optional
RUN git clone https://github.com/martinmoene/optional-lite /tmp/optional-lite; \
    (cd /tmp/optional-lite ; git checkout a0ddabb8b52e1eaaf0dd1515bb85698b747528e4); \
    cp -r /tmp/optional-lite/include/nonstd /usr/local/include; \
    rm -rf /tmp/optional-lite

# install libpq
RUN git clone --progress https://git.postgresql.org/git/postgresql.git /tmp/postgresql; \
    cd /tmp/postgresql; \
    git checkout 029386ccbddd0a33d481b94e511f5219b03e6636; \
    ./configure --without-readline --prefix=/usr/local; \
    # build
    make -j${PARALLELISM} -C src/bin/pg_config; \
    make -j${PARALLELISM} -C src/interfaces/libpq; \
    make -j${PARALLELISM} -C src/backend/utils fmgroids.h; \
    cp src/backend/utils/fmgroids.h src/include/utils/fmgroids.h; \
    # install
    make -C src/bin/pg_config install; \
    make -C src/interfaces/libpq install; \
    make -C src/include install; \
    # remove
    rm -rf /tmp/postgresql

# install pqxx
RUN git clone https://github.com/jtv/libpqxx /tmp/libpqxx; \
    (cd /tmp/libpqxx ; git checkout 5b17abce5ac2b1a2f8278718405b7ade8bb30ae9); \
    curl -L -o /tmp/libpqxx/config/config.guess 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=6b2374c79506ee82a8b440f6d1ca293e2e2e2463'; \
    curl -L -o /tmp/libpqxx/config/config.sub 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=6b2374c79506ee82a8b440f6d1ca293e2e2e2463'; \
    (cd /tmp/libpqxx ; /tmp/libpqxx/configure --disable-documentation --with-pic); \
    make -j${PARALLELISM} -C /tmp/libpqxx; \
    make -C /tmp/libpqxx install; \
    rm -rf /tmp/libpqxx

# install tbb
RUN git clone https://github.com/01org/tbb /tmp/tbb; \
    (cd /tmp/tbb ; git checkout eb6336ad29450f2a64af5123ca1b9429ff6bc11d); \
    make -j${PARALLELISM} -C /tmp/tbb tbb_build_prefix=build; \
    cp /tmp/tbb/build/build_debug/*.so* /usr/local/lib; \
    cp /tmp/tbb/build/build_release/*.so* /usr/local/lib; \
    cp -r /tmp/tbb/include/* /usr/local/include; \
    rm -rf /tmp/tbb

# install docker
ENV DOCKER_VERSION=17.06.0-ce
RUN curl -L -o /tmp/docker-${DOCKER_VERSION}.tgz https://download.docker.com/linux/static/stable/armhf/docker-${DOCKER_VERSION}.tgz; \
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
    ln -s -f /opt/sonar/scanner/bin/sonar-scanner /usr/local/bin/sonar-scanner; \
    rm -rf /tmp/sonar*

# install ed25519
RUN git clone git://github.com/hyperledger/iroha-ed25519.git /tmp/ed25519; \
    (cd /tmp/ed25519 ; git checkout e7188b8393dbe5ac54378610d53630bd4a180038); \
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -DTESTING=OFF \
        -H/tmp/ed25519 \
        -B/tmp/ed25519/build; \
    cmake --build /tmp/ed25519/build --target install -- -j${PARALLELISM}; \
    rm -rf /tmp/ed25519

# fetch lcov reports converter
RUN curl -L -o /tmp/lcov_cobertura.py https://raw.githubusercontent.com/eriwen/lcov-to-cobertura-xml/8c55cd11f80a21e7e46f20f8c81fcde0bf11f5e5/lcov_cobertura/lcov_cobertura.py

# install lcov
RUN curl -L -o /tmp/lcov-1.13.tar.gz https://github.com/linux-test-project/lcov/releases/download/v1.13/lcov-1.13.tar.gz; cd /tmp; tar zxf lcov-1.13.tar.gz; cd lcov-1.13; make install

# non-interactive adduser
#   -m = create home dir
#   -s = set default shell
#   iroha-ci = username
#   -u = userid, default for Ubuntu is 1000
#   -U = create a group same as username
#   no password
RUN useradd -ms /bin/bash iroha-ci -u 1000 -U

WORKDIR /opt/iroha
RUN chmod -R 777 /opt/iroha; \
    mkdir -p /tmp/ccache -m 777; \
    ldconfig; \
    ccache --clear


USER iroha-ci
CMD ["/bin/bash"]
