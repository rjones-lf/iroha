#!/bin/bash

total=0
for file in ${IROHA_HOME}/test_bin/*; do
  ${file}
  total=$((total + $?)) 
done
exit $total
