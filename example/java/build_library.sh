#!/bin/bash
cd "$(dirname "$0")"

# folder with bindings and native library
mkdir dist

# build native library
./prepare.sh
cp build/shared_model/bindings/libirohajava.jnilib dist/libirohajava.jnilib


# build jar
gradle jar
gradle javaDocJar

# combine to one jar
cp build/shared_model/bindings/libs/* dist/

cd build/shared_model/bindings
jar -cvf iroha_lib.jar *.java

cd ../../../
cp build/shared_model/bindings/iroha_lib.jar dist
