set(CMAKE_CXX_STANDARD 14)                    # force std=c++14
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_POSITION_INDEPENDENT_CODE    TRUE)  # -fPIC
SET(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)     # https://stackoverflow.com/a/40110752/1953079
SET(CMAKE_INSTALL_RPATH                "../lib")
set(CMAKE_EXPORT_COMPILE_COMMANDS      "ON")  # creates compile_database.json
set(CMAKE_COLOR_MAKEFILE               "ON")  # enables color output in Make


#add_compile_options("-Wall")
add_compile_options("-Wno-error=deprecated-declarations")
add_compile_options("-Wno-deprecated-declarations")
#add_compile_options("-Wno-error=potentially-evaluated-expression")
#add_compile_options("-Wno-error=inconsistent-missing-override")
