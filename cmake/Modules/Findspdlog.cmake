find_path(spdlog_INCLUDE_DIRS spdlog/spdlog.h)

if(spdlog_INCLUDE_DIRS)
  file(STRINGS "${spdlog_INCLUDE_DIRS}/spdlog/spdlog.h" spdlog_version_str REGEX "#define SPDLOG_VERSION ")
  string(REGEX REPLACE "#define SPDLOG_VERSION \"([.0-9]+)\"" "\\1" spdlog_VERSION "${spdlog_version_str}")
endif()

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(spdlog
  REQUIRED_VARS spdlog_INCLUDE_DIRS
  VERSION_VAR spdlog_VERSION
)