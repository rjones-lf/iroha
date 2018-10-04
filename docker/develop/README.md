# Description

This Dockerfile builds Docker image with all the dependencies for compiling Iroha source code

# Building Iroha in Docker image

1. Build Docker image:
    - ```docker build --build-arg PARALLELISM=\<NUM_OF_THREADS_FOR_MAKE\> -t hyperledger/iroha-develop .```
    - Now, you have an image named "hyperledger/iroha-develop", containing all necessary dependencies in `/opt/dependencies` directory
2. Create a container and get into it:
    ```docker run --rm -it hyperledger/iroha-develop```
3. Now, you are inside and ready to pull and build Iroha:
    - ```git clone https://github.com/hyperledger/iroha.git```
    - ```cd iroha && mkdir build && cd build```
    - ```cmake ..```
    - ```make -j $PARALLELISM```
4. You now have Iroha binaries inside the container
5. Exit the container by ctrl+D
