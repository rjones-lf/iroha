#!/usr/bin/env groovy

def doBindings() {

    def cmake_options = ""
    if (params.JavaBindings) {
        cmake_options += " -DSWIG_JAVA=ON "
    }
    if (params.PythonBindings) {
        cmake_options += " -DSWIG_PYTHON=ON "
    }
    // In case language specific options were not set,
    // build for every language
    if (!params.JavaBindings && !params.PythonBindings) {
        cmake_options += " -DSWIG_JAVA=ON -DSWIG_PYTHON=ON "
    }
    sh """
        cmake \
          -H. \
          -Bbuild \
          ${cmake_options}
    """
    sh "cmake --build build --target irohajava -- -j${params.PARALLELISM}"
    sh "cmake --build build --target irohapy -- -j${params.PARALLELISM}"
    archive(includes: 'build/shared_model/bindings/')
}

return this
