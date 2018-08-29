#!/usr/bin/env bash
# This script build and install source-based dependencies.
# By default it builds static versions of used libs, so it CAN BE used for NPM package publishing.

# CMake 3.7
git clone https://gitlab.kitware.com/cmake/cmake.git /tmp/cmake; \
    (cd /tmp/cmake ; git checkout 64130a7e793483e24c1d68bdd234f81d5edb2d51); \
    (cd /tmp/cmake ; /tmp/cmake/bootstrap --parallel="$(getconf _NPROCESSORS_ONLN)" --enable-ccache); \
    make -j"$(getconf _NPROCESSORS_ONLN)" -C /tmp/cmake; \
    make -C /tmp/cmake install; \
    rm -rf /tmp/cmake

# Boost (static)
git clone https://github.com/boostorg/boost /tmp/boost; \
    (cd /tmp/boost ; git checkout 436ad1dfcfc7e0246141beddd11c8a4e9c10b146); \
    (cd /tmp/boost ; git submodule init); \
    (cd /tmp/boost ; git submodule update --recursive -j "$(getconf _NPROCESSORS_ONLN)"; \
    (cd /tmp/boost ; /tmp/boost/bootstrap.sh --with-libraries=thread,system,filesystem); \
    (cd /tmp/boost ; /tmp/boost/b2 headers); \
    (cd /tmp/boost ; sudo /tmp/boost/b2 link=static cxxflags="-std=c++14" -j "$(getconf _NPROCESSORS_ONLN)" install); \
    rm -rf /tmp/boost

# Protobuf (static) v3.5.1
git clone https://github.com/google/protobuf
cd protobuf
git checkout 106ffc04be1abf3ff3399f54ccf149815b287dd9
cmake -Hcmake/ -Bbuild -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_BUILD_TYPE=Release
sudo cmake --build build/ --target install -- -j"$(getconf _NPROCESSORS_ONLN)"
