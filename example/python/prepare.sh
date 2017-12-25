#!/usr/bin/env bash

# generate iroha lib
cmake -DSWIG_PYTHON=ON ../..
make _iroha

# generate proto files in current dir
protoc --proto_path=../../schema --python_out=. block.proto primitive.proto commands.proto queries.proto responses.proto endpoint.proto yac.proto ordering.proto loader.proto
python -m grpc_tools.protoc --proto_path=../../schema --python_out=. --grpc_python_out=. endpoint.proto yac.proto ordering.proto loader.proto
