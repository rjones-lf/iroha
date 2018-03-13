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
      -DCMAKE_BUILD_TYPE=Release \
      ${cmake_options}
  """
  sh "cd build; make -j${params.PARALLELISM} irohajava irohapy"
  archive(includes: 'build/shared_model/bindings/')
}

return this
