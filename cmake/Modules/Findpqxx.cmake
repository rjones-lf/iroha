add_library(pqxx UNKNOWN IMPORTED)

find_path(pqxx_INCLUDE_DIR pqxx/pqxx)
mark_as_advanced(pqxx_INCLUDE_DIR)

find_library(pqxx_LIBRARY pqxx)
mark_as_advanced(pqxx_LIBRARY)

find_package_handle_standard_args(pqxx DEFAULT_MSG
    pqxx_INCLUDE_DIR
    pqxx_LIBRARY
    )

set(URL https://github.com/jtv/libpqxx.git)
set(VERSION 5b17abce5ac2b1a2f8278718405b7ade8bb30ae9)
set_target_description(pqxx "C++ bindings for postgres client library" ${URL} ${VERSION})

if (NOT pqxx_FOUND)
  externalproject_add(jtv_libpqxx
      GIT_REPOSITORY ${URL}
      GIT_TAG        ${VERSION}
      CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env CXXFLAGS=${CMAKE_CXX_FLAGS} CPPFLAGS=-I${postgres_INCLUDE_DIR} PG_CONFIG=${pg_config_EXECUTABLE} ./configure --disable-documentation --with-postgres-include=${pq_INCLUDE_DIR} --with-postgres-lib=${pq_INCLUDE_DIR}
      BUILD_IN_SOURCE 1
      BUILD_COMMAND $(MAKE)
      BUILD_BYPRODUCTS ${EP_PREFIX}/src/jtv_libpqxx/src/.libs/libpqxx.a
                       ${EP_PREFIX}/src/jtv_libpqxx/src/libpqxx.la
                       ${EP_PREFIX}/src/jtv_libpqxx/src/.libs/libpqxx.la
                       ${EP_PREFIX}/src/jtv_libpqxx/src/.libs/libpqxx.lai
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND "" # remove test step
      UPDATE_COMMAND "" # remove update step
      )
  externalproject_get_property(jtv_libpqxx source_dir)
  set(pqxx_INCLUDE_DIR ${source_dir}/include)
  set(pqxx_LIBRARY ${source_dir}/src/.libs/libpqxx.a)
  file(MAKE_DIRECTORY ${pqxx_INCLUDE_DIR})

  add_dependencies(jtv_libpqxx pq)
  add_dependencies(pqxx jtv_libpqxx)
endif ()

set_target_properties(pqxx PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${pqxx_INCLUDE_DIR}
    IMPORTED_LOCATION ${pqxx_LIBRARY}
    INTERFACE_LINK_LIBRARIES "pq"
    )
