#!/bin/bash
export LIBP=lib
case "$PLATFORM" in
  armeabi)
    export ARCH=arch-arm
    ;;
  armeabi-v7a)
    export ARCH=arch-arm
    ;;
  arm64-v8a)
    export ARCH=arch-arm64
    ;;
  x86)
    export ARCH=arch-x86
    ;;
  x86_64)
    export ARCH=arch-x86_64
    export LIBP=lib64
    ;;
  *)
    echo Wrong ABI name: "$PLATFORM"
    exit 1
    ;;
esac

if [ "$BUILD_TYPE_A" = "Release" ]; then
    export PROTOBUF_LIB_NAME=protobuf
elif [ "$BUILD_TYPE_A" = "Debug" ]; then
    export PROTOBUF_LIB_NAME=protobufd
else
    echo "Unknown build type: $BUILD_TYPE_A"
    exit 1
fi
