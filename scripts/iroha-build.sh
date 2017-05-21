#!/bin/bash

export IROHA_HOME=/opt/iroha
export IROHA_BUILD=${IROHA_HOME}/build
export IROHA_RELEASE=${IROHA_BUILD}/iroha

clone_protobuf() {
  git clone -b v3.0.0 https://github.com/google/protobuf.git

  cd protobuf
  git cherry-pick 1760feb621a913189b90fe8595fffb74bce84598

  ./autogen.sh
  ./configure --prefix=/usr
}

clone_grpc() {
  git clone --recursive -b $(curl -L http://grpc.io/release) https://github.com/grpc/grpc
}

#
# install protobuf 3.0.0
#
cd ${IROHA_BUILD}

if [ ! -d protobuf ]; then
  clone_protobuf
else
  cd protobuf
  git status >/dev/null
  if [ ! $? ]; then
    cd ..
    rm -fr protobuf

    clone_protobuf
  fi
fi

cd ${IROHA_BUILD}/protobuf

make -j4
make install

#
# install grpc
#
cd ${IROHA_BUILD}

if [ ! -d grpc ]; then
  clone_grpc
else
  cd grpc
  git status >/dev/null
  if [ ! $? ]; then
    cd ..
    rm -fr grpc

    clone_grpc
  fi
fi

cd ${IROHA_BUILD}/grpc

make -j4
make install

#
# install sonar-scanner
#
cd ${IROHA_BUILD}

if [[ ! -x ${IROHA_BUILD}/opt/sonar-scanner/bin/sonar-scanner ]]; then
  wget -O sonar.zip https://sonarsource.bintray.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-3.0.1.733-linux.zip

  unzip -d ${IROHA_BUILD}/opt sonar.zip && rm sonar.zip

  mv ${IROHA_BUILD}/opt/sonar-scanner* ${IROHA_BUILD}/opt/sonar-scanner
fi

#
# install Iroha
#
if [[ "$(uname -m)" == "armv7l" ]]; then
  cd ${IROHA_HOME}/core/infra/ametsuchi

  if grep -q __int128_t include/ametsuchi/currency.h; then
    sed -i \
      -e 's/1024L /1024LL /' \
      -e 's/__int128_t/int64_t/g'  \
      -e 's/__uint128_t/uint64_t/g' \
      include/ametsuchi/ametsuchi.h \
      include/ametsuchi/currency.h \
      src/ametsuchi/currency.cc \
      src/ametsuchi/wsv.cc
  fi
fi

cd ${IROHA_BUILD}

cmake ${IROHA_HOME} -DCMAKE_BUILD_TYPE=Release
make -j 10

if [[ "$(uname -m)" == "armv7l" ]]; then
  # Dirty fix to build libkeccak.a
  if [ ! -f ${IROHA_HOME}/external/src/gvanas_keccak/bin/generic32/libkeccak.a ]; then
    echo "$(date +"%Y/%m/%d %H:%M:%S") >>> Retry Build libkeccak.a"
    cd ${IROHA_HOME}/external/src/gvanas_keccak
    make -j4
    make -j4 generic32/libkeccak.a

    cd ${IROHA_BUILD}
    make -j 10
  fi
fi

if [[ $? != 0 ]]; then
  exit 1
fi

#
# copy Iroha binary and configuration file to release directory
#

rm -fr ${IROHA_RELEASE}

mkdir -p ${IROHA_RELEASE}/lib

LIBS=$(ldd ${IROHA_BUILD}/bin/iroha-main | cut -f 2 | cut -d " " -f 3)
cp -H $LIBS ${IROHA_RELEASE}/lib

mkdir -p ${IROHA_RELEASE}/config

cp ${IROHA_HOME}/config/sumeragi.json ${IROHA_RELEASE}/config/sumeragi.json
cp ${IROHA_HOME}/config/config.json   ${IROHA_RELEASE}/config/config.json

rsync -av ${IROHA_HOME}/build/bin      ${IROHA_RELEASE}
rsync -av ${IROHA_HOME}/build/test_bin ${IROHA_RELEASE}
