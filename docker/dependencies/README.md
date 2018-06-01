# Overview

Purpose of docker container, which is to be generated from this folder, is to test, if Iroha system can build, when all dependencies are installed into custom, separated directories.

# How to build:
1. Build docker image:
    - ```cd $IROHA_ROOT/docker/dependencies```
    - ```docker build --build-arg PARALLELISM=$DESIRED_THREADS_AMOUNT -t iroha-dpnd .```
    - Now, you have container named "iroha-dpnd", containing all necessary dependencies in /opt/dependencies folder
2. Get into the container:
    - for the first time:
        - run image: ```docker run -dit iroha-dpnd```
        - look up ID of the fresh container: ```docker container ls``` 
        - attach to it: ```docker attach $YOUR_CONTAINER_ID```
    - second time and furthermore:
        - look up ID of the stopped container (the one named iroha-dpnd): ```docker container ls --all```
        - run container: ```docker container start $YOUR_CONTAINER_ID```
        - attach to it: ```docker attach $YOUR_CONTAINER_ID```
3. Now, you are inside and ready to pull and build Iroha:
    - run: 
        - ```git clone https://github.com/hyperledger/iroha.git```
        - ```cd iroha && mkdir build && cd build```
        - ```cmake -D CMAKE_PREFIX_PATH="/opt/dependencies/boost;/opt/dependencies/boost/lib;/opt/dependencies/c-ares;/opt/dependencies/c-ares/lib;/opt/dependencies/ed25519;/opt/dependencies/ed25519/lib;/opt/dependencies/gflags;/opt/dependencies/gflags/lib;/opt/dependencies/grpc;/opt/dependencies/grpc/lib;/opt/dependencies/gtest;/opt/dependencies/gtest/lib;/opt/dependencies/libpq;/opt/dependencies/libpq/lib;/opt/dependencies/libpqxx;/opt/dependencies/libpqxx/lib;/opt/dependencies/protobuf;/opt/dependencies/protobuf/lib;/opt/dependencies/rapidjson;/opt/dependencies/rapidjson/lib;/opt/dependencies/rxcpp;/opt/dependencies/rxcpp/lib;/opt/dependencies/spdlog;/opt/dependencies/spdlog/lib;/opt/dependencies/tbb;/opt/dependencies/tbb/lib" ..```
        - ```make -j 'DESIRED_THREADS_AMOUNT'```
4. After performing those steps you will have a working develop build inside container
5. Exit container by ctrl+D