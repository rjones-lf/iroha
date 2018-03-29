add_executable(swig IMPORTED)

find_program(SWIG_EXECUTABLE NAMES swig3.0 swig2.0 swig)

find_package_handle_standard_args(SWIG DEFAULT_MSG
  SWIG_EXECUTABLE
  )

if(NOT SWIG_EXECUTABLE)
  find_package(Git REQUIRED)
  set(URL ftp://www.mirrorservice.org/sites/ftp.sourceforge.net/pub/sourceforge/s/sw/swig/swig/swig-3.0.12/swig-3.0.12.tar.gz)
  set_target_description(swig "Simplified Wrapper and Interface Generator (SWIG)" ${URL} 3.0.12)

  ExternalProject_Add(swig_swig
      URL ${URL}
      URL_HASH SHA256=7cf9f447ae7ed1c51722efc45e7f14418d15d7a1e143ac9f09a668999f4fc94d
      PATCH_COMMAND patch -p1 < ${PROJECT_SOURCE_DIR}/../patch/add-nodejs8-support-to-swig.patch || true
      # We should install SWIG to properly access SWIG lib
      CONFIGURE_COMMAND ./autogen.sh COMMAND ./configure --without-pcre --prefix=${EP_PREFIX}/src/swig_swig
      BUILD_IN_SOURCE ON
      BUILD_COMMAND ${MAKE} swig
      TEST_COMMAND "" # remove test step
      )
  ExternalProject_Get_Property(swig_swig source_dir)

  # Predefined vars for local installed SWIG
  set(SWIG_EXECUTABLE ${source_dir}/swig)
  set(SWIG_DIR ${source_dir})

  add_dependencies(swig swig_swig)

  message(STATUS "Installed package SWIG not found. It will be pulled to " ${SWIG_DIR} " at compile time.")
endif()

set(SWIG_USE_FILE ${CMAKE_ROOT}/Modules/UseSWIG.cmake)

set_target_properties(swig PROPERTIES
    IMPORTED_LOCATION ${SWIG_EXECUTABLE}
    )

mark_as_advanced(SWIG_DIR)
