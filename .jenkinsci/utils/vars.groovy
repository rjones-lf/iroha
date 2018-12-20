#!/usr/bin/env groovy

def compilerMapping () {
  return ['gcc5': ['cxx_compiler':'g++-5', 'cc_compiler':'gcc-5'],
          'gcc7' : ['cxx_compiler':'g++-7', 'cc_compiler':'gcc-7'],
          'clang6': ['cxx_compiler':'clang++-6.0', 'cc_compiler':'clang-6.0'],
          'appleclang': ['cxx_compiler':'clang++', 'cc_compiler':'clang'],
         ]
  }


return this
