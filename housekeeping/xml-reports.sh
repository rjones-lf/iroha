#!/bin/sh

BUILD_DIR=/opt/iroha/build
PARALLELISM=$(nproc --all)
IROHA_HOME=$(dirname $(realpath $0))/..

cd $IROHA_HOME

# all files with source code will be here
SOURCES=$(find $PWD -type f  | egrep "\.(hpp|cpp|h|c|cc)$" | egrep -v "external")

cd $BUILD_DIR

# run vera++
VERA_XML=${BUILD_DIR}/vera-report.xml
(vera++ - -s -c ${VERA_XML}) < $SOURCES


# run cppcheck
CPPCHECK_XML=${BUILD_DIR}/cppcheck-report.xml
cppcheck -j${PARALLELISM} -v \
  -Ilibs -Iirohad -Iiroha-cli \
  --enable=all \
  --xml --xml-version=2 ${SOURCES} 2> ${CPPCHECK_XML}

# run clang-tidy
# currently sonar can not parse clang-tidy output
#run-clang-tidy-3.8.py -p $BUILD_DIR -j${PARALLELISM} -header-filter=^${BUILD_DIR}/.* 2>

# run tests, valgrind
GTEST_DIR=${BUILD_DIR}/test_bin
for test in $(ls $GTEST_DIR); do
 VGXML=${BUILD_DIR}/valgrind-$test.xml
 # XUXML=${BUILD_DIR}/xunit-$test.xml

 bin=$GTEST_DIR/$test
 valgrind --xml=yes --xml-file=$VGXML \
    --leak-check=full --demangle=yes \
    $bin 
    # generate xunit files
    # disabled here, but enabled in circleci
    #--gtest_output=xml:$XUXML
done

# collect the coverage data 
gcovr -x -r ${IROHA_HOME} > ${BUILD_DIR}/gcovr-report.xml
