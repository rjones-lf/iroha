#!/bin/bash
cd "$(dirname "$0")"

echo "Usage: $0 [JAVA_PACKAGE]"
echo "Example: $0"
echo "Example: $0 org.iroha"
echo ""
if [[ $# == 1 ]]; then
    PACKAGE=$1
    echo PACKAGE=${PACKAGE}
	
    #replace line
    #    find_package(JNI REQUIRED)
    #to
    #    SET(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -package org.iroha)
    CMAKE_FILE=../../shared_model/bindings/CMakeLists.txt
    sed -i.bak "s~find_package(JNI REQUIRED)~SET(CMAKE_SWIG_FLAGS \${CMAKE_SWIG_FLAGS} -package ${PACKAGE})~" ${CMAKE_FILE}

    sed -i.bak "1i package ${PACKAGE};" TransactionExample.java
fi

# folder with bindings and native library
mkdir dist

# build native library
./prepare.sh

unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
    cp build/shared_model/bindings/libirohajava.so dist/libirohajava.so
elif [[ "$unamestr" == 'Darwin' ]]; then
    cp build/shared_model/bindings/libirohajava.jnilib dist/libirohajava.jnilib
fi

if [[ $# == 1 ]]; then
    #make java package folder path(org.iroha to org/iroha)
    FOLDER_PATH=`echo ${PACKAGE/\./\//}`
    mkdir -p build/shared_model/bindings/${FOLDER_PATH}
    mv build/shared_model/bindings/*.java build/shared_model/bindings/${FOLDER_PATH}
fi

# build jar
gradle jar
gradle javaDocJar

# combine to one jar
cp build/shared_model/bindings/libs/* dist/

cd build/shared_model/bindings
if [[ $# == 0 ]]; then
    jar -cvf iroha_lib.jar *.java
else
    jar -cvf iroha_lib.jar *.java ${FOLDER_PATH}/*.java
fi

cd ../../../
cp build/shared_model/bindings/iroha_lib.jar dist
