FROM ubuntu:16.04

RUN apt-get update; \
    apt-get -y upgrade; \
    apt-get -y --no-install-recommends install apt-utils; \
    apt-get -y clean

RUN apt-get -y --no-install-recommends install build-essential python-software-properties \
        automake libtool \
        # dev dependencies
        libssl-dev libc-ares-dev \
        # CircleCI dependencies
        git ssh tar gzip ca-certificates python3 python3-pip python3-setuptools \
        # other
        wget curl cmake file unzip gdb iputils-ping vim ccache; \
    apt-get -y clean

RUN pip3 install flask requests

# install protobuf 3.3.0
RUN git clone https://github.com/google/protobuf /tmp/protobuf-3.3.0; \
    (cd /tmp/protobuf-3.3.0 ; git checkout a6189acd18b00611c1dc7042299ad75486f08a1a); \
    (cd /tmp/protobuf-3.3.0/ ; /tmp/protobuf-3.3.0/autogen.sh); \
    (cd /tmp/protobuf-3.3.0/ ; /tmp/protobuf-3.3.0/configure); \
    make -C /tmp/protobuf-3.3.0; \
    make -C /tmp/protobuf-3.3.0 install; \
    ldconfig; \
    rm -rf /tmp/protobuf-3.3.0

# install grpc 1.3.2
RUN git clone https://github.com/grpc/grpc /tmp/grpc-1.3.2; \
    (cd /tmp/grpc-1.3.2 ; git checkout c80d3321d0f77bef8cfff8b32490a07c1e90a5ad); \
    make -C /tmp/grpc-1.3.2; \
    make -C /tmp/grpc-1.3.2 install; \
    ldconfig; \
    rm -rf /tmp/grpc-1.3.2

# install gtest 1.8.0
RUN git clone https://github.com/google/googletest /tmp/googletest-release-1.8.0; \
    (cd /tmp/googletest-release-1.8.0 ; git checkout ec44c6c1675c25b9827aacd08c02433cccde7780); \
    cmake -H/tmp/googletest-release-1.8.0 -B/tmp/googletest-release-1.8.0/build; \
    cmake --build /tmp/googletest-release-1.8.0/build --target install; \
    rm -rf /tmp/googletest-release-1.8.0

# install gflags 2.2.0
RUN git clone https://github.com/gflags/gflags /tmp/gflags-2.2.0; \
    (cd /tmp/gflags-2.2.0 ; git checkout f8a0efe03aa69b3336d8e228b37d4ccb17324b88); \
    cmake -H/tmp/gflags-2.2.0 -B/tmp/gflags-2.2.0/build; \
    cmake --build /tmp/gflags-2.2.0/build --target install; \
    rm -rf /tmp/gflags-2.2.0

# install spdlog 0.13.0
RUN git clone https://github.com/gabime/spdlog /tmp/spdlog-0.13.0; \
    (cd /tmp/spdlog-0.13.0 ; git checkout f85a08622e20b74bff34381cafcb8ef8167b29d0); \
    cmake -DSPDLOG_BUILD_TESTING=OFF -H/tmp/spdlog-0.13.0 -B/tmp/spdlog-0.13.0/build; \
    cmake --build /tmp/spdlog-0.13.0/build --target install; \
    rm -rf /tmp/spdlog-0.13.0

# install rxcpp 4.0.0, apply patch for include path
RUN git clone https://github.com/Reactive-Extensions/RxCpp /tmp/RxCpp-4.0.0; \
    (cd /tmp/RxCpp-4.0.0 ; git checkout 08c47e42930168cedf76037f8c76d47565251599); \
    (cd /tmp/RxCpp-4.0.0 ; git cherry-pick -n 5b7b2a5519f712a7831053865a4ae6082dac4ed5); \
    cmake -H/tmp/RxCpp-4.0.0 -B/tmp/RxCpp-4.0.0/build; \
    cmake --build /tmp/RxCpp-4.0.0/build --target install; \
    rm -rf /tmp/RxCpp-4.0.0

# install ed25519 and sha3
RUN git clone https://github.com/MizukiSonoko/ed25519 /tmp/ed25519; \
    (cd /tmp/ed25519 ; git checkout 3a5c02cbd91f84983a1622ed6bc14de6264b8361); \
    make -C /tmp/ed25519; \
    cp /tmp/ed25519/lib/* /usr/local/lib; \
    cp /tmp/ed25519/src/sha3.h /usr/local/include; \
    cp /tmp/ed25519/src/ed25519.h /usr/local/include; \
    rm -rf /tmp/ed25519

# install rapidjson 1.1.0
RUN git clone https://github.com/miloyip/rapidjson /tmp/rapidjson-1.1.0; \
    (cd /tmp/rapidjson-1.1.0 ; git checkout f54b0e47a08782a6131cc3d60f94d038fa6e0a51); \
    cmake -DRAPIDJSON_BUILD_EXAMPLES=OFF -H/tmp/rapidjson-1.1.0 -B/tmp/rapidjson-1.1.0/build; \
    cmake --build /tmp/rapidjson-1.1.0/build --target install; \
    rm -rf /tmp/rapidjson-1.1.0

# install optional 2.1.0
RUN git clone https://github.com/martinmoene/optional-lite /tmp/optional-lite-2.1.0; \
    (cd /tmp/optional-lite-2.1.0 ; git checkout a0ddabb8b52e1eaaf0dd1515bb85698b747528e4); \
    cp -r /tmp/optional-lite-2.1.0/include/nonstd /usr/local/include; \
    rm -rf /tmp/optional-lite-2.1.0

# install libpq 9.5.8
RUN git clone https://git.postgresql.org/git/postgresql.git /tmp/postgresql-9.5.8; \
    (cd /tmp/postgresql-9.5.8 ; git checkout 029386ccbddd0a33d481b94e511f5219b03e6636); \
    (cd /tmp/postgresql-9.5.8 ; /tmp/postgresql-9.5.8/configure --without-readline --prefix=/usr/local); \
    make -C /tmp/postgresql-9.5.8/src/bin/pg_config; \
    make -C /tmp/postgresql-9.5.8/src/interfaces/libpq; \
    make -C /tmp/postgresql-9.5.8/src/bin/pg_config install; \
    make -C /tmp/postgresql-9.5.8/src/interfaces/libpq install; \
    make -C /tmp/postgresql-9.5.8/src/include install; \
    rm -rf /tmp/postgresql-9.5.8

# install pqxx 5.1.0
RUN git clone https://github.com/jtv/libpqxx /tmp/libpqxx-5.1.0; \
    (cd /tmp/libpqxx-5.1.0 ; git checkout 5b17abce5ac2b1a2f8278718405b7ade8bb30ae9); \
    (cd /tmp/libpqxx-5.1.0 ; /tmp/libpqxx-5.1.0/configure --disable-documentation --with-pic); \
    make -C /tmp/libpqxx-5.1.0; \
    make -C /tmp/libpqxx-5.1.0 install; \
    rm -rf /tmp/libpqxx-5.1.0

# install cpp_redis 3.5.3 with tacopie 2.4.4
RUN git clone https://github.com/Cylix/cpp_redis /tmp/cpp_redis-3.5.3; \
    (cd /tmp/cpp_redis-3.5.3 ; git checkout 727aa5f06c8ce498168cbab5a023cad5b9c00bc0); \
    git clone https://github.com/Cylix/tacopie /tmp/cpp_redis-3.5.3/tacopie; \
    (cd /tmp/cpp_redis-3.5.3/tacopie ; git checkout 290dc38681f346adae41d3cc8feabbe534424675); \
    cmake -H/tmp/cpp_redis-3.5.3 -B/tmp/cpp_redis-3.5.3/build; \
    cmake --build /tmp/cpp_redis-3.5.3/build --target install; \
    rm -rf /tmp/cpp_redis-3.5.3

# install libuv 1.13.1
RUN git clone https://github.com/libuv/libuv /tmp/libuv-1.13.1; \
    (cd /tmp/libuv-1.13.1 ; git checkout 2bb4b68758f07cd8617838e68c44c125bc567ba6); \
    /tmp/libuv-1.13.1/autogen.sh; \
    (cd /tmp/libuv-1.13.1 ; /tmp/libuv-1.13.1/configure); \
    make -C /tmp/libuv-1.13.1; \
    make -C /tmp/libuv-1.13.1 install; \
    rm -rf /tmp/libuv-1.13.1

# install uvw 1.1.0
RUN git clone https://github.com/skypjack/uvw /tmp/uvw-1.1.0_libuv-v1.13; \
    (cd /tmp/uvw-1.1.0_libuv-v1.13 ; git checkout 00de1f1110ce4a9803a85a214af5326529f11312); \
    cp -r /tmp/uvw-1.1.0_libuv-v1.13/src/* /usr/local/include; \
    rm -rf /tmp/uvw-1.1.0_libuv-v1.13

# install tbb
RUN git clone https://github.com/01org/tbb /tmp/tbb-2017_U7; \
    (cd /tmp/tbb-2017_U7 ; git checkout eb6336ad29450f2a64af5123ca1b9429ff6bc11d); \
    make -C /tmp/tbb-2017_U7 tbb_build_prefix=build; \
    cp /tmp/tbb-2017_U7/build/build_debug/*.so* /usr/local/lib; \
    cp /tmp/tbb-2017_U7/build/build_release/*.so* /usr/local/lib; \
    cp -r /tmp/tbb-2017_U7/include/* /usr/local/include; \
    ldconfig; \
    rm -rf /tmp/tbb-2017_U7

# install docker
RUN curl -L -o /tmp/docker-17.06.0-ce.tgz https://download.docker.com/linux/static/stable/x86_64/docker-17.06.0-ce.tgz; \
    tar -xz -C /tmp -f /tmp/docker-17.06.0-ce.tgz; \
    mv /tmp/docker/* /usr/bin; \
    rm /tmp/docker-17.06.0-ce.tgz; \
    rm -rf /tmp/docker

CMD ["/bin/bash"]
