cmake_minimum_required(VERSION 3.7)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK    "${CCACHE_PROGRAM}")
  message(STATUS "ccache enabled (${CCACHE_PROGRAM})")
endif()

##################################
## setup Hunter and its modules ##
##################################
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
SET(HUNTER_STATUS_PRINT ON  CACHE BOOL "Verbose info")
SET(HUNTER_STATUS_DEBUG OFF CACHE BOOL "Debug info"  )

# hunter binary cache server
set(
    HUNTER_CACHE_SERVERS
    "https://github.com/soramitsu/hunter-binary-cache;https://github.com/elucideye/hunter-cache;https://github.com/ingenue/hunter-cache"
    CACHE
    STRING
    "Hunter cache servers"
)

# enable this option to upload your binary cache
# GITHUB_USER=<github_user> environment variable must be set
# GITHUB_USER_PASSWORD=<github_personal_token> environment variable must be set
option(HUNTER_RUN_UPLOAD "Upload cache binaries" OFF)
if(HUNTER_RUN_UPLOAD)
  if(NOT DEFINED "ENV{GITHUB_USER}")
    message(FATAL_ERROR "HUNTER_RUN_UPLOAD=ON, but GITHUB_USER env variable is empty")
  endif()
  if(NOT DEFINED "ENV{GITHUB_USER_PASSWORD}")
    message(FATAL_ERROR "HUNTER_RUN_UPLOAD=ON, but GITHUB_USER_PASSWORD env variable is empty, set to personal access token from github")
  endif()
endif()


set(
    HUNTER_PASSWORDS_PATH
    "${CMAKE_SOURCE_DIR}/cmake/hunter-binary-cache.cmake"
    CACHE
    FILEPATH
    "Hunter passwords"
)

#########################
## configuration types ##
#########################
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Coverage" CACHE STRING "" FORCE)

# we do not gather coverage information for dependencies, so we do not need
# Coverage configuration type here
set(HUNTER_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "")


include(HunterGate)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.20.3.tar.gz"
    SHA1 "32f280cd3883f25446a578a39bfab702d6b1c60c"
    FILEPATH ${CMAKE_SOURCE_DIR}/cmake/hunter-config.cmake
)

######################
## project settings ##
######################

PROJECT(
    iroha
    LANGUAGES C CXX
)


option(BENCHMARKING "Build benchmarks"               OFF)
option(TESTING      "Build tests"                    ON )
option(COVERAGE     "Enable coverage"                OFF)
option(FUZZING      "Build fuzzing binaries"         OFF)
option(PACKAGE_ZIP  "Create ZIP package"             OFF)
option(PACKAGE_TGZ  "Create TGZ package"             OFF)
option(PACKAGE_RPM  "Create RPM package"             OFF)
option(PACKAGE_DEB  "Create DEB package"             OFF)
option(ENABLE_LIBS_PACKAGING "Enable libs packaging" ON)
option(SWIG_PYTHON  "Generate Swig Python bindings"  OFF)
option(SWIG_JAVA    "Generate Swig Java bindings"    OFF)
option(SUPPORT_PYTHON2 "ON if Python2, OFF if python3" OFF)
option(SWIG_CSHARP  "Generate Swig C# bindings"      OFF)
option(SHARED_MODEL_DISABLE_COMPATIBILITY "Disable backward compatibility in shared model" OFF)


if(PACKAGE_TGZ OR PACKAGE_ZIP OR PACKAGE_RPM OR PACKAGE_DEB)
  message(STATUS "Packaging enabled: forcing non-packaging options to OFF")
  set(BENCHMARKING OFF)
  set(TESTING OFF)
  set(COVERAGE OFF)
  set(FUZZING OFF)
  set(SWIG_PYTHON OFF)
  set(SWIG_JAVA OFF)
endif()

message(STATUS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "-DTESTING=${TESTING}")
message(STATUS "-DBENCHMARKING=${BENCHMARKING}")
message(STATUS "-DFUZZING=${FUZZING}")
message(STATUS "-DCOVERAGE=${COVERAGE}")
message(STATUS "-DPACKAGE_ZIP=${PACKAGE_ZIP}")
message(STATUS "-DPACKAGE_TGZ=${PACKAGE_TGZ}")
message(STATUS "-DPACKAGE_RPM=${PACKAGE_RPM}")
message(STATUS "-DPACKAGE_DEB=${PACKAGE_DEB}")
message(STATUS "-DENABLE_LIBS_PACKAGING=${ENABLE_LIBS_PACKAGING}")
message(STATUS "-DSWIG_PYTHON=${SWIG_PYTHON}")
message(STATUS "-DSWIG_JAVA=${SWIG_JAVA}")
message(STATUS "-DSUPPORT_PYTHON2=${SUPPORT_PYTHON2}")
message(STATUS "-DSWIG_CSHARP=${SWIG_CSHARP}")
message(STATUS "-DSHARED_MODEL_DISABLE_COMPATIBILITY=${SHARED_MODEL_DISABLE_COMPATIBILITY}")

SET(IROHA_SCHEMA_DIR "${PROJECT_SOURCE_DIR}/schema")
include_directories(
  ${PROJECT_SOURCE_DIR}/irohad
  ${PROJECT_SOURCE_DIR}/shared_model
  ${PROJECT_SOURCE_DIR}/libs
  ${IROHA_SCHEMA_DIR}
)

# main setup stage
# must be at this place
include(cmake/setup.cmake)

if(PACKAGE_TGZ OR PACKAGE_ZIP OR PACKAGE_RPM OR PACKAGE_DEB)
  include(cmake/release.cmake)
endif()

if(SHARED_MODEL_DISABLE_COMPATIBILITY)
    add_definitions(-DDISABLE_BACKWARD)
endif()

add_subdirectory(schema)
add_subdirectory(libs)
add_subdirectory(irohad)
add_subdirectory(iroha-cli)
add_subdirectory(shared_model)

if(TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

