#!/bin/bash

set -ex
if [ "$(uname)" != "Darwin" ]; then
    echo "Only MacOS is supported"
    exit 1
fi
command -v xcodebuild > /dev/null 2>&1 || { echo >&2 "xcodebuild is required but it's not installed.  Aborting.";
    exit 1; }
if [ -d lib ]; then
    echo "Please run this script from an empty directory"
    exit 1
fi

CORES=$(getconf _NPROCESSORS_ONLN)
if [ "$CORES" -gt 1 ]; then
    CORES=$((CORES - 1))
fi

if [[ ( "$#" -lt 1 ) || ( "$#" -gt 3 ) ]]; then
    echo "Illegal number of parameters"
    echo "Usage: $0 <PLATFORM> [BUILD_TYPE=Release] [CORES=${CORES}]"
    echo "Example: $0 OS Debug ${CORES}"
    exit 1
fi

PLATFORM=$1
BUILD_TYPE=$2

if [[ -n $3 ]]; then
    case $3 in
        *[!0-9]*)
            echo Wrong number of cores: "$3"
            exit 1
        ;;
        *)
            CORES=$3
        ;;
    esac
fi

case "$PLATFORM" in
    SIMULATOR | SIMULATOR64 | OS)
    ;;
    *)
        echo Wrong platform name: "$PLATFORM"
        exit 1
    ;;
esac

if [[ ( -z ${BUILD_TYPE} ) || ( ${BUILD_TYPE} = Release ) ]]; then
    PROTOBUF_LIB_NAME=protobuf
elif [[ ${BUILD_TYPE} = Debug ]]; then
    PROTOBUF_LIB_NAME=protobufd
else
    echo "Unknown build type: ${BUILD_TYPE}"
    exit 1
fi

[[ ${PLATFORM} =~ "SIMULATOR" ]] && ARC_TYPE="-DENABLE_ARC=0" && echo "Forcing ENABLE_ARC to OFF on SIMULATOR platforms"
IOS_TOOLCHAIN_ARGS=( -DCMAKE_TOOLCHAIN_FILE="$PWD"/ios-cmake/ios.toolchain.cmake -DIOS_PLATFORM=${PLATFORM} ${ARC_TYPE} )
DEPS_DIR="$PWD"/iroha/dependencies
INSTALL_ARGS=( -DCMAKE_INSTALL_PREFIX="$DEPS_DIR" )

# ios toolchain file for cmake
git clone https://github.com/leetal/ios-cmake
(cd ./ios-cmake;
git checkout 096778e59743648b37b871edb6c4e57facf5866e)

# iroha develop
git clone -b develop --depth=1 https://github.com/hyperledger/iroha
mkdir "$DEPS_DIR"
mkdir "$DEPS_DIR"/lib
mkdir "$DEPS_DIR"/include

# boost
curl -OL https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz
tar xf ./boost_1_66_0.tar.gz
cp -R ./boost_1_66_0/boost "$DEPS_DIR"/include

# protobuf
git clone https://github.com/google/protobuf
(cd ./protobuf;
git checkout 80a37e0782d2d702d52234b62dd4b9ec74fd2c95)
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -Dprotobuf_BUILD_TESTS=OFF -H./protobuf/cmake -B./protobuf/host_build # build for host to get js_embed
VERBOSE=1 cmake --build ./protobuf/host_build -- -j"$CORES"
# to be able to run js_embed we need its host version
sed -i.bak "s~COMMAND js_embed~COMMAND \"$PWD/protobuf/host_build/js_embed\"~" ./protobuf/cmake/libprotoc.cmake
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${IOS_TOOLCHAIN_ARGS[@]}" "${INSTALL_ARGS[@]}" -Dprotobuf_BUILD_TESTS=OFF -H./protobuf/cmake -B./protobuf/.build
VERBOSE=1 cmake --build ./protobuf/.build --target install -- -j"$CORES"

# ed25519
git clone https://github.com/hyperledger/iroha-ed25519.git
(cd ./iroha-ed25519;
git checkout e7188b8393dbe5ac54378610d53630bd4a180038)
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${IOS_TOOLCHAIN_ARGS[@]}" "${INSTALL_ARGS[@]}" -DTESTING=OFF -DBUILD=STATIC -H./iroha-ed25519 -B./iroha-ed25519/build
VERBOSE=1 cmake --build ./iroha-ed25519/build --target install -- -j"$CORES"
mv "$DEPS_DIR"/lib/static/libed25519.a "$DEPS_DIR"/lib;
rmdir "$DEPS_DIR"/lib/static/

# build iroha
sed -i.bak "s~find_library(protobuf_LIBRARY protobuf)~find_library(protobuf_LIBRARY ${PROTOBUF_LIB_NAME})~" ./iroha/cmake/Modules/Findprotobuf.cmake
sed -i.bak "s~find_program(protoc_EXECUTABLE protoc~set(protoc_EXECUTABLE \"${PWD}/protobuf/host_build/protoc\"~" ./iroha/cmake/Modules/Findprotobuf.cmake # use host protoc
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -H./iroha/shared_model -B./iroha/shared_model/build "${IOS_TOOLCHAIN_ARGS[@]}" -DTESTING=OFF -DSHARED_MODEL_DISABLE_COMPATIBILITY=ON -DCMAKE_PREFIX_PATH="$DEPS_DIR"
VERBOSE=1 cmake --build ./iroha/shared_model/build --target bindings -- -j"$CORES"

# copy artifacts
mkdir lib
mkdir include
cp "$DEPS_DIR"/lib/lib${PROTOBUF_LIB_NAME}.a \
 "$DEPS_DIR"/lib/libed25519.a \
 ./iroha/shared_model/build/amount/libiroha_amount.a \
 ./iroha/shared_model/build/generator/libgenerator.a \
 ./iroha/shared_model/build/bindings/libbindings.a \
 ./iroha/shared_model/build/cryptography/ed25519_sha3_impl/internal/libed25519_crypto.a \
 ./iroha/shared_model/build/cryptography/ed25519_sha3_impl/internal/libhash.a \
 ./iroha/shared_model/build/cryptography/ed25519_sha3_impl/libshared_model_cryptography.a \
 ./iroha/shared_model/build/cryptography/model_impl/libshared_model_cryptography_model.a \
 ./iroha/shared_model/build/validators/libshared_model_stateless_validation.a \
 ./iroha/shared_model/build/schema/libschema.a \
 lib
cp -R "$DEPS_DIR"/include/* include
(cd ./iroha/shared_model; find . -name '*.hpp' | cpio -pdm ../../include)
(cd ./iroha/libs; find . -name '*.hpp' | cpio -pdm ../../include)
(cd ./iroha/schema; find . -name '*.h' | cpio -pdm ../../include)
