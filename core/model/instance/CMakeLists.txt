SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# All domain instance is in libdomain.a
ADD_LIBRARY(domain  STATIC
  account.cpp
  sample_asset.cpp
)
