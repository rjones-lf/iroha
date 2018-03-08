set(FLAGS
    -O3           # maximum optimizations
    -DNDEBUG      # disable assertions

    CACHE INTERNAL ""
    )

# JOINED_FLAGS = " ".join([w for w in FLAGS])
JOIN("${FLAGS}" " " JOINED_FLAGS)

set(CMAKE_CXX_FLAGS_RELEASE
    ${JOINED_FLAGS}
    CACHE STRING "Flags used by the C++ compiler during RELEASE builds."
    FORCE
    )

set(CMAKE_C_FLAGS_RELEASE
    ${JOINED_FLAGS}
    CACHE STRING "Flags used by the C compiler during RELEASE builds."
    FORCE
    )
