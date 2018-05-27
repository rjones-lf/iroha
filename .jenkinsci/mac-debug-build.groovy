def doDebugBuild(coverageEnabled=false) {
  def parallelism = params.PARALLELISM
  if (parallelism == null) {
    parallelism = 4
  }
  def cmakeOptions = ""
  if ( coverageEnabled ) {
    cmakeOptions = " -DCOVERAGE=ON "
  }
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
      -DCMAKE_BUILD_TYPE=${params.BUILD_TYPE} \
      -DIROHA_VERSION=${env.IROHA_VERSION} \
      ${cmakeOptions}
  """
  sh "cmake --build build -- -j${params.PARALLELISM}"
  sh "ccache --show-stats"
}

def doPreCoverageStep() {
  sh "cmake --build build --target coverage.init.info"
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
  def testExitCode = sh(script: """CTEST_OUTPUT_ON_FAILURE=1 IROHA_POSTGRES_HOST=localhost IROHA_POSTGRES_PORT=5433 cmake --build build --target test -- -R '${testList}' """, returnStatus: true)
  if (testExitCode != 0) {
    currentBuild.currentResult = "UNSTABLE"
  }
}
 
def doPostCoverageCoberturaStep() {
  sh "cmake --build build --target cppcheck"
    // Sonar
    if (env.CHANGE_ID != null) {
      sh """
        sonar-scanner \
          -Dsonar.github.disableInlineComments \
          -Dsonar.github.repository='hyperledger/iroha' \
          -Dsonar.analysis.mode=preview \
          -Dsonar.login=${SONAR_TOKEN} \
          -Dsonar.projectVersion=${BUILD_TAG} \
          -Dsonar.github.oauth=${SORABOT_TOKEN}
      """
    }
}

def doPostCoverageSonarStep() {
  sh "cmake --build build --target coverage.info"
  sh "python /usr/local/bin/lcov_cobertura.py build/reports/coverage.info -o build/reports/coverage.xml"
  cobertura autoUpdateHealth: false, autoUpdateStability: false, coberturaReportFile: '**/build/reports/coverage.xml', conditionalCoverageTargets: '75, 50, 0', failUnhealthy: false, failUnstable: false, lineCoverageTargets: '75, 50, 0', maxNumberOfBuilds: 50, methodCoverageTargets: '75, 50, 0', onlyStable: false, zoomCoverageChart: false
}

return this
