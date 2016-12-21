set(CMAKE_BUILD_TYPE Debug)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -std=c++11 -Wl,-rpath $ENV{JAVA_HOME}/jre/lib/server")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -DDEBUG -O0 -std=c++11 -Wl,-rpath $ENV{JAVA_HOME}/jre/lib/server")
ELSE()
  SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -std=c++11 -Wl,-rpath $ENV{JAVA_HOME}/jre/lib/amd64")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -DDEBUG -O0 -std=c++11 -Wl,-rpath $ENV{JAVA_HOME}/jre/lib/amd64")
ENDIF()


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)
set(GTEST_INCLUDE_DIR ${PROJECT_BINARY_DIR}/gtest/src/googletest/googletest/include)

include_directories(
  ${GTEST_INCLUDE_DIR}
  $ENV{JAVA_HOME}/include
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  include_directories($ENV{JAVA_HOME}/include/darwin)
  link_directories(
    ${PROJECT_BINARY_DIR}/lib
    $ENV{JAVA_HOME}/jre/lib/server
  )
ELSE()
  include_directories($ENV{JAVA_HOME}/include/linux)
  link_directories(
    ${PROJECT_BINARY_DIR}/lib
    $ENV{JAVA_HOME}/jre/lib/amd64/server
  )
ENDIF()

add_executable(java_vm_test java_vm_test.cpp)
target_link_libraries(java_vm_test
  smart_contract_with_java
  jvm
  ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a pthread)
add_test(
  NAME java_vm_test
  COMMAND $<TARGET_FILE:java_vm_test>
)


