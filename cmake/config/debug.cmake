include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-Og HAS_OG)

if(HAS_OG)
    set(OPTLVL -Og)
else()
    set(OPTLVL -O0)
endif()

set(FLAGS
    -Wextra
    ${OPTLVL}                       # optimization level
    -g                              # include debug symbols

    CACHE INTERNAL ""
    )

# JOINED_FLAGS = " ".join([w for w in FLAGS])
JOIN("${FLAGS}" " " JOINED_FLAGS)

set(CMAKE_CXX_FLAGS_DEBUG
    ${JOINED_FLAGS}
    CACHE STRING "Flags used by the C++ compiler during Debug builds."
    FORCE
    )

set(CMAKE_C_FLAGS_DEBUG
    ${JOINED_FLAGS}
    CACHE STRING "Flags used by the C compiler during Debug builds."
    FORCE
    )
