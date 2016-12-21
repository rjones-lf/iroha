
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -std=c++11 -Wl,-rpath $ENV{JAVA_HOME}/jre/lib/server")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -DDEBUG -O0 -std=c++11 -Wl,-rpath $ENV{JAVA_HOME}/jre/lib/server")
ELSE()
  SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -std=c++11 -Wl,-rpath $ENV{JAVA_HOME}/jre/lib/amd64/server")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -DDEBUG -O0 -std=c++11 -Wl,-rpath $ENV{JAVA_HOME}/jre/lib/amd64/server")
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  include_directories(
    $ENV{JAVA_HOME}/include/darwin/
    $ENV{JAVA_HOME}/include/
  )
  link_directories(
    $ENV{JAVA_HOME}/jre/lib/server
    $ENV{JAVA_HOME}/jre/lib/amd64/server
  )
ELSE()
  include_directories(
    $ENV{JAVA_HOME}/include/linux
    $ENV{JAVA_HOME}/include
  )
  link_directories(
    $ENV{JAVA_HOME}/jre/lib/amd64/server/
  )
ENDIF()

ADD_LIBRARY(java_virtual_machine STATIC java_virtual_machine.cpp)
target_link_libraries(java_virtual_machine
  $ENV{JAVA_HOME}/jre/lib/amd64/server/libjvm.so
)  

