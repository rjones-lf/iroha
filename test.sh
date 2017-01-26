#!/bin/sh

total=0
for file in ${IROHA_HOME}/build/test_bin/*; do
  ${file}
  total=$((total + $?)) 
done
exit $total
