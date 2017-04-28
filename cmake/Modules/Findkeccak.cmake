find_path(keccak_INCLUDE_DIRS KeccakCodePackage.h 
  PATH_SUFFIXES generic64/libkeccak.a.headers libkeccak.a.headers
  )

find_library(keccak_LIBRARIES keccak 
  PATH_SUFFIXES generic64
  )

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(keccak
  REQUIRED_VARS keccak_INCLUDE_DIRS keccak_LIBRARIES
  )
