#!/usr/bin/env bash

cd $(dirname $0)

# generate iroha lib
if [[ $1 -eq 0 && ${IROHA_HOME} -eq 0 ]] ; then
    echo 'Neither IROHA_HOME nor path to CMakeLists.txt is set'
    exit 0
fi
cmake_path=${1,-${IROHA_HOME}}
cmake -DSWIG_JAVA=ON $1
make -j"$(getconf _NPROCESSORS_ONLN)" irohajava
