#-----------------------------------------------------------------------
# iroha-dev - IROHA development container (base image for iroha)
#
# build : docker build -t hyperledger/iroha-dev .
#
# Copyright (c) 2016 Soramitsu,Co.,Ltd.
# All Rights Reserved.
#-----------------------------------------------------------------------
FROM ubuntu

MAINTAINER Takeshi Yonezu <yonezu@soramitsu.co.jp> / Bogdan Vaneev <bogdan@soramitsu.co.jp>

LABEL BUILD="docker build -t hyperledger/iroha-dev ."
LABEL RUN="docker run -it --rm hyperledger/iroha-dev bash"

USER root

ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64
ENV IROHA_HOME /opt/iroha
ENV IROHA_BUILD /tmp/build
ENV IROHA_RELEASE /usr/local/iroha

# we use single RUN instruction to reduce number of "layers" in docker image: https://docs.docker.com/engine/userguide/eng-image/dockerfile_best-practices
# install dependencies
RUN apt-get update && apt-get -y upgrade; \
    apt-get -y install \
        software-properties-common \
        python-software-properties \
        curl \
        git \
        make \
        build-essential \
        cmake \
        g++-5 \
        gcc-5 \
        default-jdk \
        libhdf5-serial-dev \
        libleveldb-dev \
        libsnappy-dev\
        liblmdb-dev \
        libssl-dev \
        snappy \
        unzip \
        xsltproc \
        zlib1g-dev \
        autoconf \
        automake \
        libtool \
        pkg-config ; \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test; \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 20; \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-5 20; \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 20; \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-5 20; \
    curl -sSL https://cmake.org/files/v3.5/cmake-3.5.2-Linux-x86_64.tar.gz | tar -xzC /opt; \
    ln -s /opt/cmake*/bin/c* /usr/local/bin; \
    mkdir -p ${IROHA_RELEASE}; \

    # install protobuf v3.0.0
    cd /tmp; \
    git clone -b v3.0.0 https://github.com/google/protobuf.git; \
    cd protobuf; \
    (git cherry-pick 1760feb621a913189b90fe8595fffb74bce84598; echo Force continue) && \
    ./autogen.sh && \
    ./configure --prefix=/usr && \
    make -j4 && make install; \

    # install grpc
    cd /tmp; \
    git clone --recursive -b $(curl -L http://grpc.io/release) https://github.com/grpc/grpc; \
    cd grpc; \
    make && make install; \

    # install flatbuffers
    cd /tmp; \
    git clone https://github.com/google/flatbuffers.git; \
    cd flatbuffers; \
    cmake -G "Unix Makefiles" && \
    make && make install

# build and mktar scripts
ADD ./scripts /
