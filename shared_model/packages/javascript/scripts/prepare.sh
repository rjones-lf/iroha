#!/bin/bash

CURDIR="$(cd "$(dirname "$0")"; pwd)"
IROHA_HOME="$(dirname $(dirname $(dirname $(dirname "${CURDIR}"))))"

sh $CURDIR/clean.sh
sh $CURDIR/generate-protobuf.sh

echo "Copying compiled library files..."

mkdir -p $CURDIR/../lib
cp -u $IROHA_HOME/shared_model/build/bindings/irohalib.js $CURDIR/../lib

echo "Preparing has finished successfully!"
