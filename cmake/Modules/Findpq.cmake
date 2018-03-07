add_library(pq UNKNOWN IMPORTED)
add_executable(pg_config IMPORTED)

find_path(pq_INCLUDE_DIR libpq-fe.h PATH_SUFFIXES postgresql)
mark_as_advanced(pq_INCLUDE_DIR)

find_path(postgres_INCLUDE_DIR postgres_ext.h PATH_SUFFIXES postgresql)
mark_as_advanced(postgres_INCLUDE_DIR)

find_library(pq_LIBRARY pq)
mark_as_advanced(pq_LIBRARY)

find_program(pg_config_EXECUTABLE pg_config)
mark_as_advanced(pg_config_EXECUTABLE)

find_package_handle_standard_args(pq DEFAULT_MSG
    pq_INCLUDE_DIR
    postgres_INCLUDE_DIR
    pq_LIBRARY
    pg_config_EXECUTABLE
    )


set(URL https://git.postgresql.org/git/postgresql.git)
set(VERSION 029386ccbddd0a33d481b94e511f5219b03e6636)
set_target_description(pq "C postgres client library" ${URL} ${VERSION})


if (NOT pq_FOUND)
  externalproject_add(postgres_postgres
      GIT_REPOSITORY  ${URL}
      GIT_TAG         ${VERSION}
      CONFIGURE_COMMAND ./configure --without-readline
      BUILD_IN_SOURCE 1
      BUILD_COMMAND ${MAKE} -C ./src/bin/pg_config && ${MAKE} -C ./src/interfaces/libpq
      BUILD_BYPRODUCTS ${EP_PREFIX}/src/postgres_postgres/src/interfaces/libpq/libpq.a
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND "" # remove test step
      UPDATE_COMMAND "" # remove update step
      )
  externalproject_get_property(postgres_postgres source_dir)
  set(postgres_INCLUDE_DIR ${source_dir}/src/include)
  set(pq_INCLUDE_DIR ${source_dir}/src/interfaces/libpq)
  set(pq_LIBRARY ${source_dir}/src/interfaces/libpq/libpq.a)
  set(pg_config_EXECUTABLE ${source_dir}/src/bin/pg_config/pg_config)
  file(MAKE_DIRECTORY ${pq_INCLUDE_DIR} ${postgres_INCLUDE_DIR})

  add_dependencies(pg_config postgres_postgres)
  add_dependencies(pq postgres_postgres pg_config)
endif ()

get_filename_component(pq_LIBRARY_DIR ${pq_LIBRARY} DIRECTORY)
mark_as_advanced(pq_LIBRARY_DIR)

set_target_properties(pq PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${pq_INCLUDE_DIR};${postgres_INCLUDE_DIR}"
    IMPORTED_LOCATION ${pq_LIBRARY}
    )

set_target_properties(pg_config PROPERTIES
    IMPORTED_LOCATION ${pg_config_EXECUTABLE}
    )

if(ENABLE_LIBS_PACKAGING)
  add_install_step_for_lib(${pq_LIBRARY})
endif()
