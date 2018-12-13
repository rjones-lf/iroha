#!/usr/bin/env groovy

def compilerMapping () {
  return ['gcc54': ['cxx_compiler':'g++-5', 'cc_compiler':'gcc-5'],
          'gcc7' : ['cxx_compiler':'g++-7', 'cc_compiler':'gcc-7'],
          'clang6': 'clang']

  }

enum PythonVersion {
  python2, python3
}

return this
