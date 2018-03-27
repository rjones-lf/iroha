// Overall pipeline looks like the following
//         
//   |--Linux-----|----Debug
//   |            |----Release 
//   |    OR
//   |           
//-- |--Linux ARM-|----Debug
//   |            |----Release
//   |    OR
//   |
//   |--MacOS-----|----Debug
//   |            |----Release
properties([parameters([
  choice(choices: 'Debug\nRelease', description: '', name: 'BUILD_TYPE'),
  booleanParam(defaultValue: true, description: '', name: 'Linux'),
  booleanParam(defaultValue: false, description: '', name: 'ARMv7'),
  booleanParam(defaultValue: false, description: '', name: 'ARMv8'),
  booleanParam(defaultValue: true, description: '', name: 'MacOS'),
  booleanParam(defaultValue: false, description: 'Whether build docs or not', name: 'Doxygen'),
  booleanParam(defaultValue: false, description: 'Whether build Java bindings', name: 'JavaBindings'),
  booleanParam(defaultValue: false, description: 'Whether build Python bindings', name: 'PythonBindings'),
  booleanParam(defaultValue: false, description: 'Whether build bindings only w/o Iroha itself', name: 'BindingsOnly'),
  string(defaultValue: '4', description: 'How much parallelism should we exploit. "4" is optimal for machines with modest amount of memory and at least 4 cores', name: 'PARALLELISM')])])


pipeline {
  environment {
    CCACHE_DIR = '/opt/.ccache'
    CCACHE_RELEASE_DIR = '/opt/.ccache-release'
    SORABOT_TOKEN = credentials('SORABOT_TOKEN')
    SONAR_TOKEN = credentials('SONAR_TOKEN')
    CODECOV_TOKEN = credentials('CODECOV_TOKEN')
    DOCKERHUB = credentials('DOCKERHUB')
    DOCKER_BASE_IMAGE_DEVELOP = 'hyperledger/iroha:develop'
    DOCKER_BASE_IMAGE_RELEASE = 'hyperledger/iroha:latest'

    IROHA_NETWORK = "iroha-0${CHANGE_ID}-${GIT_COMMIT}-${BUILD_NUMBER}"
    IROHA_POSTGRES_HOST = "pg-0${CHANGE_ID}-${GIT_COMMIT}-${BUILD_NUMBER}"
    IROHA_POSTGRES_USER = "pguser${GIT_COMMIT}"
    IROHA_POSTGRES_PASSWORD = "${GIT_COMMIT}"
    IROHA_POSTGRES_PORT = 5432
  }

  options {
    buildDiscarder(logRotator(numToKeepStr: '20'))
  }

  agent any
  stages {
    stage ('Stop same job builds') {
      agent { label 'master' }
      steps {
        script {
          // Stop same job running builds if any
          def builds = load ".jenkinsci/cancel-builds-same-job.groovy"
          builds.cancelSameJobBuilds()
        }
      }
    }
    stage('Build Debug') {
      when {
        allOf {
          expression { params.BUILD_TYPE == 'Debug' }
          expression { return !params.BindingsOnly }
        }        
      }
      parallel {
        stage ('Linux') {
          when { expression { return params.Linux } }
          agent { label 'x86_64' }
          steps {
            script {
              debugBuild = load ".jenkinsci/debug-build.groovy"
              debugBuild.doDebugBuild(true)
              if (BRANCH_NAME ==~ /(master|develop)/) {
                releaseBuild = load ".jenkinsci/release-build.groovy"
                releaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            always {
              script {
                timeout(time: 60, unit: "SECONDS") {
                  def cleanup = load ".jenkinsci/docker-cleanup.groovy"
                  cleanup.doDockerCleanup()
                  cleanWs()
                }
              }
            }
          }
        }
        stage('ARMv7') {
          when { expression { return params.ARMv7 } }
          agent { label 'armv7' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              if (!params.Linux && !params.ARMv8 && !params.MacOS) {
                debugBuild.doDebugBuild(true)
              }              
              else {
                debugBuild.doDebugBuild()
              }
              if (BRANCH_NAME ==~ /(master|develop)/) {
                releaseBuild = load ".jenkinsci/release-build.groovy"
                releaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            always {
              script {
                timeout(time: 60, unit: "SECONDS") {
                  def cleanup = load ".jenkinsci/docker-cleanup.groovy"
                  cleanup.doDockerCleanup()
                  cleanWs()
                }
              }
            }
          }
        }
        stage('ARMv8') {
          when { expression { return params.ARMv8 } }
          agent { label 'armv8' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              if (!params.Linux && !params.MacOS) {
                debugBuild.doDebugBuild(true)
              }
              else {
                debugBuild.doDebugBuild()
              }
              if (BRANCH_NAME ==~ /(master|develop)/) {
                releaseBuild = load ".jenkinsci/release-build.groovy"
                releaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            always {
              script {
                timeout(time: 60, unit: "SECONDS") {
                  def cleanup = load ".jenkinsci/docker-cleanup.groovy"
                  cleanup.doDockerCleanup()
                  cleanWs()
                }
              }
            }
          }
        }
        stage('MacOS'){
          when { expression { return params.MacOS } }
          agent { label 'mac' }
          steps {
            script {
              def coverageEnabled = false
              def cmakeOptions = ""
              if (!params.Linux) {
                coverageEnabled = true
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
              if ( coverageEnabled ) {
                sh "cmake --build build --target coverage.init.info"
              }
              sh """
                export IROHA_POSTGRES_PASSWORD=${IROHA_POSTGRES_PASSWORD}; \
                export IROHA_POSTGRES_USER=${IROHA_POSTGRES_USER}; \
                mkdir -p /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}; \
                initdb -D /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/ -U ${IROHA_POSTGRES_USER} --pwfile=<(echo ${IROHA_POSTGRES_PASSWORD}); \
                pg_ctl -D /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/ -o '-p 5433' -l /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/events.log start; \
                psql -h localhost -d postgres -p 5433 -U ${IROHA_POSTGRES_USER} --file=<(echo create database ${IROHA_POSTGRES_USER};)
              """
              def testExitCode = sh(script: 'IROHA_POSTGRES_HOST=localhost IROHA_POSTGRES_PORT=5433 cmake --build build --target test', returnStatus: true)
              if (testExitCode != 0) {
                currentBuild.result = "UNSTABLE"
              }
              if ( coverageEnabled ) {
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
                sh "cmake --build build --target coverage.info"
                sh "python /usr/local/bin/lcov_cobertura.py build/reports/coverage.info -o build/reports/coverage.xml"
                cobertura autoUpdateHealth: false, autoUpdateStability: false, coberturaReportFile: '**/build/reports/coverage.xml', conditionalCoverageTargets: '75, 50, 0', failUnhealthy: false, failUnstable: false, lineCoverageTargets: '75, 50, 0', maxNumberOfBuilds: 50, methodCoverageTargets: '75, 50, 0', onlyStable: false, zoomCoverageChart: false

              }
              
              // TODO: replace with upload to artifactory server
              // only develop branch
              if ( env.BRANCH_NAME == "develop" ) {
                //archive(includes: 'build/bin/,compile_commands.json')
              }
            }
          }
          post {
            always {
              script {
                timeout(time: 60, unit: "SECONDS") {
                  cleanWs()
                  sh """
                    pg_ctl -D /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/ stop && \
                    rm -rf /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/
                  """
                }
              }
            }
          }
        }
      }
    }
    stage('Build Release') {
      when {
        allOf {
          expression { params.BUILD_TYPE == 'Release' }
          expression { return ! params.BindingsOnly }
        }        
      }
      parallel {
        stage('Linux') {
          when { expression { return params.Linux } }
          agent { label 'linux && x86_64' }
          steps {
            script {
              def releaseBuild = load ".jenkinsci/release-build.groovy"
              releaseBuild.doReleaseBuild()
            }
          }
          post {
            always {
              script {
                timeout(time: 60, unit: "SECONDS") {
                  def cleanup = load ".jenkinsci/docker-cleanup.groovy"
                  cleanup.doDockerCleanup()
                  cleanWs()
                }
              }
            }
          }
        }
        stage('ARMv7') {
          when { expression { return params.ARMv7 } }
          agent { label 'armv7' }
          steps {
            script {
              def releaseBuild = load ".jenkinsci/release-build.groovy"
              releaseBuild.doReleaseBuild()
            }
          }
          post {
            always {
              script {
                timeout(time: 60, unit: "SECONDS") {
                  def cleanup = load ".jenkinsci/docker-cleanup.groovy"
                  cleanup.doDockerCleanup()
                  cleanWs()
                }
              }
            }
          }            
        }
        stage('ARMv8') {
          when { expression { return params.ARMv8 } }
          agent { label 'armv8' }
          steps {
            script {
              def releaseBuild = load ".jenkinsci/release-build.groovy"
              releaseBuild.doReleaseBuild()
            }
          }
          post {
            always {
              script {
                timeout(time: 60, unit: "SECONDS") {
                  def cleanup = load ".jenkinsci/docker-cleanup.groovy"
                  cleanup.doDockerCleanup()
                  cleanWs()
                }
              }
            }
          }            
        }
        stage('MacOS') {
          when { expression { return params.MacOS } }            
          steps {
            script {
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
                  -H. \
                  -Bbuild \
                  -DCMAKE_BUILD_TYPE=${params.BUILD_TYPE} \
                  -DIROHA_VERSION=${env.IROHA_VERSION}
              """
              sh "cmake --build build -- -j${params.PARALLELISM}"
              sh "ccache --show-stats"
              
              // TODO: replace with upload to artifactory server
              // only develop branch
              if ( env.BRANCH_NAME == "develop" ) {
                //archive(includes: 'build/bin/,compile_commands.json')
              }
            }
          }
        }
      }
    }
    stage('Build docs') {
      when {
        allOf {
          expression { return params.Doxygen }
          expression { BRANCH_NAME ==~ /(master|develop)/ }
        }
      }
      // build docs on any vacant node. Prefer `x86_64` over
      // others as nodes are more powerful
      agent { label 'x86_64 || arm' }
      steps {
        script {
          def doxygen = load ".jenkinsci/doxygen.groovy"
          docker.image("${env.DOCKER_IMAGE}").inside {
            def scmVars = checkout scm
            doxygen.doDoxygen()
          }
        }
      }
    }
    stage('Build bindings') {
      when {
        anyOf {
          expression { return params.BindingsOnly }
          expression { return params.PythonBindings }
          expression { return params.JavaBindings }
        }
      }
      agent { label 'x86_64' }
      steps {
        script {
          def bindings = load ".jenkinsci/bindings.groovy"
          def platform = sh(script: 'uname -m', returnStdout: true).trim()
          sh "curl -L -o /tmp/${env.GIT_COMMIT}/Dockerfile --create-dirs https://raw.githubusercontent.com/hyperledger/iroha/${env.GIT_COMMIT}/docker/develop/${platform}/Dockerfile"
          iC = docker.build("hyperledger/iroha-develop:${GIT_COMMIT}-${BUILD_NUMBER}", "-f /tmp/${env.GIT_COMMIT}/Dockerfile /tmp/${env.GIT_COMMIT} --build-arg PARALLELISM=${PARALLELISM}")
          sh "rm -rf /tmp/${env.GIT_COMMIT}"
          iC.inside {
            def scmVars = checkout scm
            bindings.doBindings()
          }
        }
      }
    }
  }
}
