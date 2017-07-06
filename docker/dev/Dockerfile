FROM ubuntu:16.04

RUN apt-get update && apt-get -y upgrade && \
    apt-get -y install build-essential python-software-properties \
        automake libtool \
        # dev dependencies
        libssl-dev libc-ares-dev \
        # CircleCI dependencies
        git ssh tar gzip ca-certificates \
        # PostgreSQL
        libpq-dev \
        # other
        wget cmake file && apt-get -y clean

# install protobuf 3.3.0
RUN cd /tmp; \
    wget https://github.com/google/protobuf/releases/download/v3.3.0/protobuf-cpp-3.3.0.tar.gz; \
    tar xfo protobuf-cpp-3.3.0.tar.gz; \
    cd protobuf-3.3.0; \
    ./configure && make -j$(nproc) && make install && ldconfig; \
    rm /tmp/protobuf-cpp-3.3.0.tar.gz

# install grpc 1.3.2
RUN cd /tmp; \
    wget https://github.com/grpc/grpc/archive/v1.3.2.tar.gz; \
    tar xfo v1.3.2.tar.gz; \
    cd grpc-1.3.2; \
    rmdir third_party/protobuf; \
    ln -s /tmp/protobuf-3.3.0 third_party/protobuf; \
    make -j$(nproc) && make install; \
    rm -rf /tmp/protobuf-3.3.0 /tmp/grpc-1.3.2; \
    rm /tmp/v1.3.2.tar.gz

CMD ["/bin/bash"]
