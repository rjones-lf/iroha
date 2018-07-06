#!/usr/bin/env bash

cd $(dirname $0)

# generate iroha lib
CURDIR="$(cd "$(dirname "$0")"; pwd)"
IROHA_HOME="$(dirname $(dirname "${CURDIR}"))"
cmake -H$IROHA_HOME -Bbuild -DSWIG_PYTHON=ON -DSUPPORT_PYTHON2=ON;
cmake --build build/ --target irohapy -- -j"$(getconf _NPROCESSORS_ONLN)"

# generate proto files in current dir
protoc --proto_path=../../schema --python_out=. block.proto primitive.proto commands.proto queries.proto responses.proto endpoint.proto
python -m grpc_tools.protoc --proto_path=../../schema --python_out=. --grpc_python_out=. endpoint.proto yac.proto ordering.proto loader.proto
