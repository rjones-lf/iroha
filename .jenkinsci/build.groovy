#!/usr/bin/env groovy

def cmakeConfigure(String buildDir, String cmakeOptions, String sourceTreeDir=".") {
  sh "cmake -H${sourceTreeDir} -B${buildDir} ${cmakeOptions}"
}

def cmakeBuild(String buildDir, String cmakeOptions, int parallelism) {
  sh "cmake --build ${buildDir} ${cmakeOptions} -- -j${parallelism}"
}

def cmakeBuildWindows(String buildDir, String cmakeOptions) {
  sh "cmake --build ${buildDir} ${cmakeOptions}"
}

def cppCheck(String buildDir, int parallelism) {
  // github.com/jenkinsci/cppcheck-plugin/pull/36
  sh "cppcheck -j${parallelism} --enable=all --template='{file},,{line},,{severity},,{id},,{message}' ${buildDir} 2> cppcheck.txt"
  warnings (
    parserConfigurations: [[parserName: 'Cppcheck', pattern: "${buildDir}/cppcheck.txt"]], categoriesPattern: '',
    defaultEncoding: '', excludePattern: '', healthy: '', includePattern: '', messagesPattern: '', unHealthy: ''
  )
}

def sonarScanner(scmVars, environment) {
  withEnv(environment) {
    withCredentials([string(credentialsId: 'SONAR_TOKEN', variable: 'SONAR_TOKEN'), string(credentialsId: 'SORABOT_TOKEN', variable: 'SORABOT_TOKEN')]) {
      sh """
        sonar-scanner \
          -Dsonar.github.disableInlineComments \
          -Dsonar.github.repository='${env.DOCKER_REGISTRY_BASENAME}' \
          -Dsonar.analysis.mode=preview \
          -Dsonar.login=${SONAR_TOKEN} \
          -Dsonar.projectVersion=${BUILD_TAG} \
          -Dsonar.github.oauth=${SORABOT_TOKEN} \
          -Dsonar.github.pullRequest=${scmVars.CHANGE_ID}
      """
    }
  }
}

return this
