
Installing some packages on Ubuntu
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If your repository provides outdated versions of tools - install them from source code.

- CMake
   Before installation make sure to get all dependencies:

   - make
   - curl
   - zlib

  .. code:: sh

    git clone https://gitlab.kitware.com/cmake/cmake.git /tmp/cmake;
    (cd /tmp/cmake ; /tmp/cmake/bootstrap --system-curl --parallel=4 --enable-ccache);
    make -j4 -C /tmp/cmake;
    make -C /tmp/cmake install;
    ldconfig;
    rm -rf /tmp/cmake

- Boost

  .. code:: sh

    git clone https://github.com/boostorg/boost /tmp/boost;
    (cd /tmp/boost ; git submodule update --init --recursive);
    (cd /tmp/boost ; /tmp/boost/bootstrap.sh --with-libraries=system,filesystem);
    (cd /tmp/boost ; /tmp/boost/b2 headers);
    (cd /tmp/boost ; /tmp/boost/b2 cxxflags="-std=c++14" -j4 install);
    ldconfig;
    rm -rf /tmp/boost

- SWIG

  .. code:: sh

    wget http://prdownloads.sourceforge.net/swig/swig-3.0.12.tar.gz
    tar -xvf swig-3.0.12.tar.gz
    cd swig-3.0.12
    ./configure
    make -j4
    make install

- Protobuf

  .. code:: sh

    git clone https://github.com/google/protobuf /tmp/protobuf;
    (cd /tmp/protobuf ; git checkout 80a37e0782d2d702d52234b62dd4b9ec74fd2c95);
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_SHARED_LIBS=ON \
        -H/tmp/protobuf/cmake \
        -B/tmp/protobuf/.build;
    cmake --build /tmp/protobuf/.build --target install -- -j4;
    ldconfig;
    rm -rf /tmp/protobuf
