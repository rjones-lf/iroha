#!/usr/bin/env bash
set -e

CURDIR="$(cd "$(dirname "$0")"; pwd)"
IROHA_HOME="$(dirname $(dirname "${CURDIR}"))"
cmake -H$IROHA_HOME -Bbuild -DSWIG_JAVA=ON -DSWIG_JAVA_PKG="jp.co.soramitsu.iroha";
cmake --build build/ --target irohajava -- -j"$(getconf _NPROCESSORS_ONLN)"
