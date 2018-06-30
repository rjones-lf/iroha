#!/usr/bin/env bash

CURDIR="$(cd "$(dirname "$0")"; pwd)"
IROHA_HOME="$(dirname $(dirname $(dirname $(dirname "${CURDIR}"))))"

# Check if we inside Iroha repository
if [ -d "$IROHA_HOME/schema" ]; then
	echo "Generating Protobuf JS files..."
	
	./node_modules/.bin/grpc_tools_node_protoc --proto_path=$IROHA_HOME/schema \
	--plugin=protoc-gen-grpc=./node_modules/grpc-tools/bin/grpc_node_plugin \
	--js_out=import_style=commonjs,binary:./pb \
	--grpc_out=./pb \
	endpoint.proto yac.proto ordering.proto loader.proto block.proto primitive.proto commands.proto queries.proto responses.proto
fi
