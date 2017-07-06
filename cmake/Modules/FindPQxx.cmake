find_path(PQxx_INCLUDE_DIR pqxx/pqxx)
mark_as_advanced(PQxx_INCLUDE_DIR)

find_library(PQxx_LIBRARY pqxx)
mark_as_advanced(PQxx_LIBRARY)

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(PQxx
  REQUIRED_VARS PQxx_INCLUDE_DIR PQxx_LIBRARY
  )

if (PQxx_FOUND)
  add_library(pqxx UNKNOWN IMPORTED)
  set_target_properties(pqxx PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${PQxx_INCLUDE_DIR}
    IMPORTED_LOCATION ${PQxx_LIBRARY}
    INTERFACE_LINK_LIBRARIES "pq"
    )
endif ()
