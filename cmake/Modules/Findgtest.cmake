find_path(gtest_INCLUDE_DIRS gtest/gtest.h)

find_library(gtest_LIBRARIES gtest)
find_library(gtest_MAIN_LIBRARIES gtest_main)

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(gtest
  REQUIRED_VARS gtest_INCLUDE_DIRS gtest_LIBRARIES gtest_MAIN_LIBRARIES
  )

