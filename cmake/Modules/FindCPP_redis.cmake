find_path(CPP_redis_INCLUDE_DIR cpp_redis/cpp_redis)
mark_as_advanced(CPP_redis_INCLUDE_DIR)

find_library(CPP_redis_LIBRARY cpp_redis)
mark_as_advanced(CPP_redis_LIBRARY)

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(CPP_redis
  REQUIRED_VARS CPP_redis_INCLUDE_DIR CPP_redis_LIBRARY
  )

if (CPP_redis_FOUND)
  add_library(cpp_redis UNKNOWN IMPORTED)
  set_target_properties(cpp_redis PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${CPP_redis_INCLUDE_DIR}
    IMPORTED_LOCATION ${CPP_redis_LIBRARY}
    INTERFACE_LINK_LIBRARIES "tacopie;pthread"
    )
endif ()
