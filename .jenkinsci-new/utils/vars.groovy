#!/usr/bin/env groovy
/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

//
// vars to map compiler versions
//

def compilerMapping () {
  return ['gcc5': ['cxx_compiler':"${WORKSPACE}/.jenkinsci-new/helpers/compilers/g++-5.sh", 'cc_compiler':'gcc-5'],
          'gcc7' : ['cxx_compiler':"${WORKSPACE}/.jenkinsci-new/helpers/compilers/g++-7.sh", 'cc_compiler':'gcc-7',],
          'clang6': ['cxx_compiler':"${WORKSPACE}/.jenkinsci-new/helpers/compilers/clang++-6.0.sh", 'cc_compiler':'clang-6.0'],
          'clang7': ['cxx_compiler':"${WORKSPACE}/.jenkinsci-new/helpers/compilers/clang++-7.sh", 'cc_compiler':'clang-7'],
          'appleclang': ['cxx_compiler':"${WORKSPACE}/.jenkinsci-new/helpers/compilers/clang++.sh", 'cc_compiler':'clang'],
         ]
  }


return this
