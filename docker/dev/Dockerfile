#-----------------------------------------------------------------------
# iroha-dev - IROHA development container (base image for iroha)
#
# build : docker build -t hyperledger/iroha-dev .
#
# Copyright (c) 2016, 2017 Soramitsu,Co.,Ltd.
# All Rights Reserved.
#-----------------------------------------------------------------------
FROM ubuntu:16.04

ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64
ENV IROHA_HOME /usr/local/iroha
ENV IROHA_BUILD ${IROHA_HOME}/build
ENV IROHA_RELEASE ${IROHA_BUILD}/release

RUN apt update && apt -y upgrade

RUN apt -y install \
        software-properties-common \
        python-software-properties \
        curl git wget make cmake g++-5 gcc-5 default-jdk \
        libhdf5-serial-dev libleveldb-dev libsnappy-dev \
        liblmdb-dev libssl-dev unzip xsltproc zlib1g-dev \
        autoconf automake libtool jq pkg-config cppcheck \
        valgrind clang-tidy

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 20 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-5 20 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 20 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-5 20

RUN mkdir -p ${IROHA_RELEASE}; \
    mkdir -p ${IROHA_BUILD}; \
    mkdir -p ${IROHA_HOME}

# install protobuf 3.0.0
RUN cd /tmp; \
    git clone -b v3.0.0 https://github.com/google/protobuf.git; \
    cd protobuf; \
    (git cherry-pick 1760feb621a913189b90fe8595fffb74bce84598; echo Force continue) && \
    ./autogen.sh && \
    ./configure --prefix=/usr && \
    make -j4 && make install

# install grpc
RUN cd /tmp; \
    git clone --recursive -b $(curl -L http://grpc.io/release) https://github.com/grpc/grpc; \
    cd grpc; \
    patch -p1 <scripts/grpc.patch; \
    make -j4 && make install

# install sonar-scanner
RUN cd /tmp; \
    wget -O /tmp/sonar.zip https://sonarsource.bintray.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-3.0.1.733-linux.zip; \
    unzip -d /opt /tmp/sonar.zip; \
    mv /opt/sonar-scanner* /opt/sonar-scanner

CMD ["/bin/bash"]
