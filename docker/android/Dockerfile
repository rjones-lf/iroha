FROM ubuntu:18.04

# number of concurrent threads during build
# usage: docker build --build-arg PARALLELISM=8 -t name/name .
ARG PARALLELISM=1
ARG BUILD_TYPE_A
ENV BUILD_TYPE_A=${BUILD_TYPE_A:-Release}
ARG VERSION
ENV VERSION=${VERSION:-26}
ARG PACKAGE
ENV PACKAGE=${PACKAGE:-jp.co.soramitsu.iroha.android}
# valid platforms: armeabi, armeabi-v7a, arm64-v8a, x86, x86_64
ARG PLATFORM
ENV PLATFORM=${PLATFORM:-x86_64}

ENV NDK_PATH="/android-ndk/android-ndk-r16b"
ENV DEPS_DIR="/iroha/dependencies"

RUN apt-get update && \
    apt-get -y install --no-install-recommends git curl apt-utils software-properties-common libpthread-stubs0-dev libpcre3-dev \
    unzip zip build-essential automake libtool ca-certificates ccache zlib1g-dev libcurl4-openssl-dev libc6-dbg;  \
    rm -rf /var/lib/apt/lists/*

RUN set -e; mkdir -p $DEPS_DIR/include $DEPS_DIR/lib

# install cmake 3.11.4
RUN set -e; \
    git clone https://gitlab.kitware.com/cmake/cmake.git /tmp/cmake; \
    (cd /tmp/cmake ; git checkout 316bd45439ad8ced6b31bcb10303a788038387ef); \
    (cd /tmp/cmake ; /tmp/cmake/bootstrap --system-curl --parallel=${PARALLELISM} --enable-ccache); \
    make -j${PARALLELISM} -C /tmp/cmake; \
    make -C /tmp/cmake install; \
    ldconfig; \
    rm -rf /tmp/cmake

# boost 1.66
RUN set -e; \
	curl -L -o /tmp/boost_1_66_0.tar.gz https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz; \
	tar -zxf /tmp/boost_1_66_0.tar.gz -C /tmp; mv /tmp/boost_1_66_0/boost $DEPS_DIR/include; rm -f /tmp/boost_1_66_0.tar.gz

# install android-ndk-r16b
RUN set -e; \
	curl -L -o /tmp/android-ndk.zip https://dl.google.com/android/repository/android-ndk-r16b-linux-x86_64.zip; unzip -q /tmp/android-ndk.zip -d /android-ndk; rm -f /tmp/android-ndk.zip

# protobuf
RUN set -ex; \
	git clone https://github.com/google/protobuf; \
	(cd ./protobuf ; git checkout b5fbb742af122b565925987e65c08957739976a7); \
	cmake -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=$BUILD_TYPE_A -H./protobuf/cmake -B./protobuf/host_build; \
	VERBOSE=1 cmake --build ./protobuf/host_build -- -j$PARALLELISM; \
	sed -i.bak "s~COMMAND js_embed~COMMAND \"$PWD/protobuf/host_build/js_embed\"~" ./protobuf/cmake/libprotoc.cmake; \
	LDFLAGS="-llog -landroid" cmake -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=$VERSION -DCMAKE_ANDROID_ARCH_ABI=$PLATFORM -DANDROID_NDK=$NDK_PATH -DCMAKE_ANDROID_STL_TYPE=c++_static -DCMAKE_INSTALL_PREFIX=$DEPS_DIR -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE="$BUILD_TYPE_A" -H./protobuf/cmake -B./protobuf/.build; \
	VERBOSE=1 cmake --build ./protobuf/.build --target install -- -j$PARALLELISM

# ed25519
RUN set -e; \
	git clone git://github.com/hyperledger/iroha-ed25519; \
	(cd ./iroha-ed25519 ; git checkout f42953c631fae93011612f6b1ee33f1f88c3f8af); \
	cmake -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=$VERSION -DCMAKE_ANDROID_ARCH_ABI=$PLATFORM -DANDROID_NDK=$NDK_PATH -DCMAKE_ANDROID_STL_TYPE=c++_static -DCMAKE_INSTALL_PREFIX=$DEPS_DIR -DTESTING=OFF -DCMAKE_BUILD_TYPE=$BUILD_TYPE_A -DBUILD=STATIC -H./iroha-ed25519 -B./iroha-ed25519/build; \
	VERBOSE=1 cmake --build ./iroha-ed25519/build --target install -- -j$PARALLELISM; \
	mv "$DEPS_DIR"/lib/static/libed25519.a "$DEPS_DIR"/lib; rmdir "$DEPS_DIR"/lib/static/
