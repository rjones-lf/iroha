#!/usr/bin/env groovy

def doDebugBuild() {
  def setter = load ".jenkinsci/set-parallelism.groovy"
  def parallelism = setter.setParallelism(params.PARALLELISM)
  def scmVars = checkout scm
  env.IROHA_VERSION = "0x${scmVars.GIT_COMMIT}"
  env.IROHA_HOME = "/opt/iroha"
  env.IROHA_BUILD = "${env.IROHA_HOME}/build"

  sh """
    ccache --version
    ccache --show-stats
    ccache --zero-stats
    ccache --max-size=5G
  """
  sh """
    cmake \
      -DTESTING=ON \
      -H. \
      -Bbuild \
      -DCMAKE_BUILD_TYPE=${params.build_type} \
      -DIROHA_VERSION=${env.IROHA_VERSION}
  """
  sh "cmake --build build -- -j${parallelism}"
  sh "ccache --show-stats"
}
  
def doTestStep(testList) {
  sh """
    export IROHA_POSTGRES_PASSWORD=${IROHA_POSTGRES_PASSWORD}; \
    export IROHA_POSTGRES_USER=${IROHA_POSTGRES_USER}; \
    mkdir -p /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}; \
    initdb -D /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/ -U ${IROHA_POSTGRES_USER} --pwfile=<(echo ${IROHA_POSTGRES_PASSWORD}); \
    pg_ctl -D /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/ -o '-p 5433' -l /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/events.log start; \
    psql -h localhost -d postgres -p 5433 -U ${IROHA_POSTGRES_USER} --file=<(echo create database ${IROHA_POSTGRES_USER};)
  """
  def testExitCode = sh(script: """cd build && IROHA_POSTGRES_HOST=localhost IROHA_POSTGRES_PORT=5433 ctest --output-on-failure -R '${testList}' """, returnStatus: true)
  if (testExitCode != 0) {
    currentBuild.result = "UNSTABLE"
  }
}

return this
