#!/usr/bin/env bash

cd $(dirname $0)

# generate iroha lib
CURDIR="$(cd "$(dirname "$0")"; pwd)"
IROHA_HOME="$(dirname $(dirname "${CURDIR}"))"
cmake -H$IROHA_HOME -Bbuild -DSWIG_PYTHON=ON -DSUPPORT_PYTHON2=ON;
cmake --build build/ --target irohapy -- -j"$(getconf _NPROCESSORS_ONLN)"

# generate proto files in current dir
protoc --proto_path=../../shared_model/schema --python_out=. ../../shared_model/schema/*.proto
python -m grpc_tools.protoc --proto_path=../../shared_model/schema --python_out=. --grpc_python_out=. ../../shared_model/schema/endpoint.proto
