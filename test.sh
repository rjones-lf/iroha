#!/bin/sh

total=0
test_path=

if [ -d ${IROHA_HOME}/build ]; then 
    test_path=${IROHA_HOME}/build/test_bin/*;
else
    test_path=${IROHA_HOME}/test_bin/*;
fi

for file in $test_path; do
    # run test
    ${file}
    total=$((total + $?)) 
done
exit $total
