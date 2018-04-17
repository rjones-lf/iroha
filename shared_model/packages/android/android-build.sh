#!/bin/bash
set -ex
if [ -d lib ]; then
	echo "Please run this script from an empty directory"
	exit 1
fi
if [[ ( "$#" -ne 4 ) && ( "$#" -ne 5 ) ]]; then
    echo "Illegal number of parameters"
    echo "Usage: $0 <PLATFORM> <ANDROID_VERSION> <NDK_PATH> <PACKAGE> [BUILD_TYPE=Release]"
    echo "Example: $0 arm64-v8a 26 /Users/me/Downloads/android-ndk-r16b jp.co.soramitsu.iroha.android Debug"
    exit 1
fi

PLATFORM=$1
VERSION=$2
NDK_PATH=$3
PACKAGE=$4
BUILD_TYPE=$5

if ! [ -d "$NDK_PATH" ]; then
  echo "Path to NDK does not exist"
  exit 1
fi

LIBP=lib
case "$PLATFORM" in
  armeabi)
    ARCH=arch-arm
    ;;
  armeabi-v7a)
    ARCH=arch-arm
    ;;
  arm64-v8a)
    ARCH=arch-arm64
    ;;
  x86)
    ARCH=arch-x86
    ;;
  x86_64)
    ARCH=arch-x86_64
    LIBP=lib64
    ;;
  *)
	echo Wrong ABI name: "$PLATFORM"
    exit 1
    ;;
esac

if [[ ( -z ${BUILD_TYPE} ) || (${BUILD_TYPE} = Release) ]]; then
    PROTOBUF_LIB_NAME=protobuf
elif [[ ${BUILD_TYPE} = Debug ]]; then
    PROTOBUF_LIB_NAME=protobufd
else
    echo "Unknown build type: ${BUILD_TYPE}"
    exit 1
fi

ANDROID_TOOLCHAIN_ARGS=(-DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION="$VERSION" -DCMAKE_ANDROID_ARCH_ABI="$PLATFORM" -DANDROID_NDK="$NDK_PATH" -DCMAKE_ANDROID_STL_TYPE=c++_static)
DEPS_DIR="$PWD"/iroha/dependencies
INSTALL_ARGS=(-DCMAKE_INSTALL_PREFIX="$DEPS_DIR")

CORES=$(getconf _NPROCESSORS_ONLN)
if [ "$CORES" -gt 1 ]; then
    CORES=$((CORES-1))
fi

# iroha develop
git clone -b develop --depth=1 https://github.com/hyperledger/iroha
mkdir "$DEPS_DIR"
mkdir "$DEPS_DIR"/lib
mkdir "$DEPS_DIR"/include

# boost
BOOST_URL=https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz
if [ "$(uname)" == "Darwin" ]; then
    curl -OL "$BOOST_URL"
else
    wget "$BOOST_URL"
fi
tar xf ./boost_1_66_0.tar.gz
cp -R ./boost_1_66_0/boost "$DEPS_DIR"/include

# protobuf
git clone https://github.com/google/protobuf
(cd ./protobuf ; git checkout b5fbb742af122b565925987e65c08957739976a7)
cmake -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -H./protobuf/cmake -B./protobuf/host_build # build for host to get js_embed
VERBOSE=1 cmake --build ./protobuf/host_build -- -j"$CORES"
# to be able to run js_embed we need its host version
sed -i.bak "s~COMMAND js_embed~COMMAND \"$PWD/protobuf/host_build/js_embed\"~" ./protobuf/cmake/libprotoc.cmake

LDFLAGS="-llog -landroid" cmake "${ANDROID_TOOLCHAIN_ARGS[@]}" "${INSTALL_ARGS[@]}" -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -H./protobuf/cmake -B./protobuf/.build
VERBOSE=1 cmake --build ./protobuf/.build --target install -- -j"$CORES"

# ed25519
git clone https://github.com/hyperledger/iroha-ed25519.git
(cd ./iroha-ed25519 ; git checkout e7188b8393dbe5ac54378610d53630bd4a180038)
cmake "${ANDROID_TOOLCHAIN_ARGS[@]}" "${INSTALL_ARGS[@]}" -DTESTING=OFF -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DBUILD=STATIC -H./iroha-ed25519 -B./iroha-ed25519/build
VERBOSE=1 cmake --build ./iroha-ed25519/build --target install -- -j"$CORES"
mv "$DEPS_DIR"/lib/static/libed25519.a "$DEPS_DIR"/lib; rmdir "$DEPS_DIR"/lib/static/

# SWIG fixes
sed -i.bak "s~find_package(JNI REQUIRED)~#find_package(JNI REQUIRED)~" ./iroha/shared_model/bindings/CMakeLists.txt
sed -i.bak "s~include_directories(${JAVA_INCLUDE_PATH})~#include_directories(${JAVA_INCLUDE_PATH})~" ./iroha/shared_model/bindings/CMakeLists.txt
sed -i.bak "s~include_directories(${JAVA_INCLUDE_PATH2})~#include_directories(${JAVA_INCLUDE_PATH2})~" ./iroha/shared_model/bindings/CMakeLists.txt
sed -i.bak "s~# the include path to jni.h~SET(CMAKE_SWIG_FLAGS \${CMAKE_SWIG_FLAGS} -package ${PACKAGE})~" ./iroha/shared_model/bindings/CMakeLists.txt
sed -i.bak "s~swig_link_libraries(irohajava~swig_link_libraries(irohajava \"${PWD}/protobuf/.build/lib${PROTOBUF_LIB_NAME}.a\" \"${NDK_PATH}/platforms/android-${VERSION}/${ARCH}/usr/${LIBP}/liblog.so\"~" ./iroha/shared_model/bindings/CMakeLists.txt

# build iroha
sed -i.bak "s~find_library(protobuf_LIBRARY protobuf)~find_library(protobuf_LIBRARY ${PROTOBUF_LIB_NAME})~" ./iroha/cmake/Modules/Findprotobuf.cmake
sed -i.bak "s~find_program(protoc_EXECUTABLE protoc~set(protoc_EXECUTABLE \"${PWD}/protobuf/host_build/protoc\"~" ./iroha/cmake/Modules/Findprotobuf.cmake # use host protoc
cmake -H./iroha/shared_model -B./iroha/shared_model/build "${ANDROID_TOOLCHAIN_ARGS[@]}" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DTESTING=OFF -DSHARED_MODEL_DISABLE_COMPATIBILITY=ON -DSWIG_JAVA=ON -DCMAKE_PREFIX_PATH="$DEPS_DIR"
VERBOSE=1 cmake --build ./iroha/shared_model/build --target irohajava -- -j"$CORES"

# copy artifacts
mkdir lib
zip ./lib/bindings.zip ./iroha/shared_model/build/bindings/*.java
cp ./iroha/shared_model/build/bindings/libirohajava.so ./lib
