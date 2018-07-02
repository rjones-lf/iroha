#!/usr/bin/env groovy

def doReleaseBuild(coverageEnabled=false) {
  def setter = load ".jenkinsci/set-parallelism.groovy"
  def parallelism = setter.setParallelism(params.PARALLELISM)
  def scmVars = checkout scm
  env.IROHA_VERSION = "0x${scmVars.GIT_COMMIT}"
  env.IROHA_HOME = "/opt/iroha"
  env.IROHA_BUILD = "${env.IROHA_HOME}/build"

  sh """
    export CCACHE_DIR=${CCACHE_RELEASE_DIR}
    ccache --version
    ccache --show-stats
    ccache --zero-stats
    ccache --max-size=1G

    cmake -H. \
      -Bbuild \
      -DCOVERAGE=OFF \
      -DPACKAGE_TGZ=ON \
      -DCMAKE_BUILD_TYPE=${params.build_type} \
      -DIROHA_VERSION=${env.IROHA_VERSION}
    
    cmake --build build --target package -- -j${parallelism}
    mv ./build/iroha-${env.IROHA_VERSION}-*.tar.gz ./build/iroha.tar.gz
    ccache --show-stats
  """
}

return this
