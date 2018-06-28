properties([parameters([
  booleanParam(defaultValue: true, description: '', name: 'x86_64_linux'),
  booleanParam(defaultValue: false, description: '', name: 'armv7_linux'),
  booleanParam(defaultValue: false, description: '', name: 'armv8_linux'),
  booleanParam(defaultValue: false, description: '', name: 'x86_64_macos'),
  booleanParam(defaultValue: false, description: '', name: 'x86_64_win'),
  booleanParam(defaultValue: false, description: 'Build coverage', name: 'coverage'),
  booleanParam(defaultValue: false, description: 'Merge this PR to target after success build', name: 'merge_pr'),
  booleanParam(defaultValue: false, description: 'Scheduled nightly build', name: 'nightly'),
  choice(choices: 'Debug\nRelease', description: 'Iroha build type', name: 'build_type'),
  booleanParam(defaultValue: false, description: 'Build `bindings`', name: 'bindings'),
  booleanParam(defaultValue: false, description: 'Build Java bindings', name: 'JavaBindings'),
  choice(choices: 'Release\nDebug', description: 'Java bindings build type', name: 'JBBuildType'),
  booleanParam(defaultValue: false, description: 'Build Python bindings', name: 'PythonBindings'),
  choice(choices: 'Release\nDebug', description: 'Python bindings build type', name: 'PBBuildType'),
  choice(choices: 'python3\npython2', description: 'Python bindings version', name: 'PBVersion'),
  booleanParam(defaultValue: false, description: 'Build Android bindings', name: 'AndroidBindings'),
  choice(choices: '26\n25\n24\n23\n22\n21\n20\n19\n18\n17\n16\n15\n14', description: 'Android Bindings ABI Version', name: 'ABABIVersion'),
  choice(choices: 'Release\nDebug', description: 'Android bindings build type', name: 'ABBuildType'),
  choice(choices: 'arm64-v8a\narmeabi-v7a\narmeabi\nx86_64\nx86', description: 'Android bindings platform', name: 'ABPlatform'),
  booleanParam(defaultValue: false, description: 'Build docs', name: 'Doxygen'),
  string(defaultValue: '4', description: 'How much parallelism should we exploit. "4" is optimal for machines with modest amount of memory and at least 4 cores', name: 'PARALLELISM')])])

pipeline {
  environment {
    CCACHE_DIR = '/opt/.ccache'
    CCACHE_RELEASE_DIR = '/opt/.ccache-release'
    SORABOT_TOKEN = credentials('SORABOT_TOKEN')
    SONAR_TOKEN = credentials('SONAR_TOKEN')
    GIT_RAW_BASE_URL = "https://raw.githubusercontent.com/hyperledger/iroha"
    DOCKER_REGISTRY_BASENAME = "hyperledger/iroha"
    JENKINS_DOCKER_IMAGE_DIR = '/tmp/docker'
    GIT_COMMITER_EMAIL = ''

    IROHA_NETWORK = "iroha-0${CHANGE_ID}-${GIT_COMMIT}-${BUILD_NUMBER}"
    IROHA_POSTGRES_HOST = "pg-0${CHANGE_ID}-${GIT_COMMIT}-${BUILD_NUMBER}"
    IROHA_POSTGRES_USER = "pguser${GIT_COMMIT}"
    IROHA_POSTGRES_PASSWORD = "${GIT_COMMIT}"
    IROHA_POSTGRES_PORT = 5432

    DOCKER_AGENT_IMAGE = ''
    DOCKER_IMAGE_FILE = ''
    WORKSPACE_PATH = ''
    MERGE_CONDITIONS_SATISFIED = ''
    REST_PR_CONDITIONS_SATISFIED = ''
    INITIAL_COMMIT_PR = ''
  }

  options {
    buildDiscarder(logRotator(numToKeepStr: '20'))
    timestamps()
  }

  agent any
  stages {
    stage ('Pre-build') {
      agent { label 'master' }
      steps {
        script {
          load ".jenkinsci/enums.groovy"
          def preBuildRoutine = load ".jenkinsci/pre-build.groovy"
          preBuildRoutine.prepare()
          if (GIT_LOCAL_BRANCH != "develop") {
            def builds = load ".jenkinsci/cancel-builds-same-job.groovy"
            builds.cancelSameJobBuilds()
          }
        }
      }
    }
    stage('Build') {
      parallel {
        stage ('x86_64_linux') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.x86_64_linux }
              expression { return MERGE_CONDITIONS_SATISFIED == "true" }
            }
          }
          agent { label 'x86_64_aws_build' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def coverage = load ".jenkinsci/build-coverage.groovy"
              if (params.build_type == 'Debug') {
                debugBuild.doDebugBuild(coverage.checkCoverageConditions())
              }
              if (params.build_type == 'Release') {
                def releaseBuild = load ".jenkinsci/release-build.groovy"
                releaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            success {
              script {
                if (params.build_type == 'Release') {
                  def post = load ".jenkinsci/post-step.groovy"
                  post.postStep()
                }
              }
            }
          }
        }
        stage('armv7_linux') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.armv7_linux }
              // expression { return MERGE_CONDITIONS_SATISFIED == "true" }
            }
          }
          agent { label 'armv7' }
          steps {
            script {
              if (params.build_type == 'Debug') {
                def debugBuild = load ".jenkinsci/debug-build.groovy"
                debugBuild.doDebugBuild()
              }
              else {
                def releaseBuild = load ".jenkinsci/release-build.groovy"
                releaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            success {
              script {
                if (params.build_type == 'Release') {
                  def post = load ".jenkinsci/post-step.groovy"
                  post.postStep()
                }
              }
            }
          }
        }
        stage('armv8_linux') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.armv8_linux }
              // expression { return MERGE_CONDITIONS_SATISFIED == "true" }
            }
          }
          agent { label 'armv8' }
          steps {
            script {
              if ( params.build_type == 'Debug') {
                def debugBuild = load ".jenkinsci/debug-build.groovy"
                debugBuild.doDebugBuild()
              }
              else {
                def releaseBuild = load ".jenkinsci/release-build.groovy"
                releaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            success {
              script {
                if (params.build_type == 'Release') {
                  def post = load ".jenkinsci/post-step.groovy"
                  post.postStep()
                }
              }
            }
          }
        }
        stage('x86_64_macos') {
          when {
            beforeAgent true
            anyOf {
              expression { return INITIAL_COMMIT_PR == "true" }
              expression { return MERGE_CONDITIONS_SATISFIED == "true" }
              expression { return params.x86_64_macos }
            }
          }
          agent { label 'mac' }
          steps {
            script {
              if (params.build_type == 'Debug') {
                def macDebugBuild = load ".jenkinsci/mac-debug-build.groovy"
                macDebugBuild.doDebugBuild()
              }
              else {
                def macReleaseBuild = load ".jenkinsci/mac-release-build.groovy"
                macReleaseBuild.doReleaseBuild()
              }
            }
          }
          post {
            success {
              script {
                if (params.build_type == 'Release') {
                  def post = load ".jenkinsci/post-step.groovy"
                  post.macPostStep()
                }
              }
            }
          }
        }
      }
    }
    stage('Pre-Coverage') {
      when {
        beforeAgent true
        anyOf {
          expression { params.coverage }  // by request
          expression { return INITIAL_COMMIT_PR == "true" }
          expression { return MERGE_CONDITIONS_SATISFIED == "true" }
          allOf {
            expression { return params.build_type == 'Debug' }
            expression { return env.GIT_LOCAL_BRANCH ==~ /master/ }
          }
        }
      }
      agent { label 'x86_64_aws_cov'}
      steps {
        script {
          def coverage = load '.jenkinsci/debug-build.groovy'
          coverage.doPreCoverageStep()
        }
      }
    }
    stage('Tests') {
      when {
        beforeAgent true
        expression { return params.build_type == "Debug" }
      }
      parallel {
        stage('x86_64_linux') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.x86_64_linux }
              expression { return MERGE_CONDITIONS_SATISFIED == "true" }
            }
          }
          agent { label 'x86_64_aws_test' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              debugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
          post {
            cleanup {
              script {
                def clean = load ".jenkinsci/docker-cleanup.groovy"
                clean.doDockerNetworkCleanup()
              }
            }
          }
        }
        stage('armv7_linux') {
          when {
            beforeAgent true
            allOf {
              expression { return params.armv7_linux }
              // expression { return MERGE_CONDITIONS_SATISFIED == "false" }
            }
          }
          agent { label 'armv7' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              debugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
          post {
            cleanup {
              script {
                def clean = load ".jenkinsci/docker-cleanup.groovy"
                clean.doDockerNetworkCleanup()
              }
            }
          }
        }
        stage('armv8_linux') {
          when {
            beforeAgent true
            allOf {
              expression { return params.armv8_linux }
              // expression { return MERGE_CONDITIONS_SATISFIED == "false" }
            }
          }
          agent { label 'armv8' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              debugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
          post {
            cleanup {
              script {
                def clean = load ".jenkinsci/docker-cleanup.groovy"
                clean.doDockerNetworkCleanup()
              }
            }
          }
        }
        stage('x86_64_macos') {
          when {
            beforeAgent true
            allOf {
              expression { return INITIAL_COMMIT_PR == "false" }
              expression { return MERGE_CONDITIONS_SATISFIED == "false" }
              expression { return params.x86_64_macos }
            }
          }
          agent { label 'mac' }
          steps {
            script {
              def macDebugBuild = load ".jenkinsci/mac-debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              macDebugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
          post {
            cleanup {
              script {
                sh "pg_ctl -D /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/ stop"
              }
            }
          }
        }
      }
    }
    stage('Post-Coverage') {
      when {
        beforeAgent true
        anyOf {
          expression { params.coverage }  // by request
          expression { return INITIAL_COMMIT_PR == "true" }
          expression { return MERGE_CONDITIONS_SATISFIED == "true" }
          allOf {
            expression { return params.build_type == 'Debug' }
            expression { return env.GIT_LOCAL_BRANCH ==~ /master/ }
          }
        }
      }
      parallel {
        stage('lcov_cobertura') {
          agent { label 'x86_64_aws_cov' }
          steps {
            script {
              def coverage = load '.jenkinsci/debug-build.groovy'
              coverage.doPostCoverageCoberturaStep()
            }
          }
        }
        stage('sonarqube') {
          agent { label 'x86_64_aws_cov' }
          steps {
            script {
              def coverage = load '.jenkinsci/debug-build.groovy'
              coverage.doPostCoverageSonarStep()
            }
          }
        }
      }
    }
    stage ('Build rest') {
      parallel {
        stage('linux_release') {
          when {
            beforeAgent true
            anyOf {
              allOf {
                expression { return params.x86_64_linux }
                expression { return params.build_type == 'Debug' }
                expression { return env.GIT_LOCAL_BRANCH ==~ /(develop|master|trunk)/ }
              }
              expression { return MERGE_CONDITIONS_SATISFIED == "true" }
            }
          }
          agent { label 'x86_64_aws_build' }
          steps {
            script {
              def releaseBuild = load '.jenkinsci/release-build.groovy'
              releaseBuild.doReleaseBuild()
            }
          }
          post {
            success {
              script {
                if (params.build_type == 'Release') {
                  def post = load ".jenkinsci/post-step.groovy"
                  post.postStep()
                }
              }
            }
          }
        }
        stage('docs') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.Doxygen }
              expression { return GIT_LOCAL_BRANCH ==~ /(master|develop)/ }
              expression { return REST_PR_CONDITIONS_SATISFIED == "true" }
            }
          }
          agent { label 'x86_64_aws_cov' }
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
        stage('bindings') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.bindings }
              expression { return REST_PR_CONDITIONS_SATISFIED == "true" }
            }
          }
          agent { label 'x86_64_aws_build' }
          environment {
            JAVA_HOME = '/usr/lib/jvm/java-8-oracle'
          }
          steps {
            script {
              def bindings = load ".jenkinsci/bindings.groovy"
              def dPullOrBuild = load ".jenkinsci/docker-pull-or-build.groovy"
              def platform = sh(script: 'uname -m', returnStdout: true).trim()
              if (params.JavaBindings) {
                iC = dPullOrBuild.dockerPullOrUpdate("$platform-develop",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/develop/${platform}/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_PREVIOUS_COMMIT}/docker/develop/${platform}/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/develop/docker/develop/x86_64/Dockerfile",
                                                     ['PARALLELISM': params.PARALLELISM])
                iC.inside("-v /tmp/${env.GIT_COMMIT}/bindings-artifact:/tmp/bindings-artifact") {
                  bindings.doJavaBindings(params.JBBuildType)
                }
              }
              if (params.PythonBindings) {
                iC = dPullOrBuild.dockerPullOrUpdate("$platform-develop",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/develop/${platform}/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_PREVIOUS_COMMIT}/docker/develop/${platform}/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/develop/docker/develop/x86_64/Dockerfile",
                                                     ['PARALLELISM': params.PARALLELISM])
                iC.inside("-v /tmp/${env.GIT_COMMIT}/bindings-artifact:/tmp/bindings-artifact") {
                  bindings.doPythonBindings(params.PBBuildType)
                }
              }
              if (params.AndroidBindings) {
                iC = dPullOrBuild.dockerPullOrUpdate("android-${params.ABPlatform}-${params.ABBuildType}",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/android/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/${env.GIT_PREVIOUS_COMMIT}/docker/android/Dockerfile",
                                                     "${env.GIT_RAW_BASE_URL}/develop/docker/android/Dockerfile",
                                                     ['PARALLELISM': params.PARALLELISM, 'PLATFORM': params.ABPlatform, 'BUILD_TYPE': params.ABBuildType])
                sh "curl -L -o /tmp/${env.GIT_COMMIT}/entrypoint.sh ${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/android/entrypoint.sh"
                sh "chmod +x /tmp/${env.GIT_COMMIT}/entrypoint.sh"
                iC.inside("-v /tmp/${env.GIT_COMMIT}/entrypoint.sh:/entrypoint.sh:ro -v /tmp/${env.GIT_COMMIT}/bindings-artifact:/tmp/bindings-artifact") {
                  bindings.doAndroidBindings(params.ABABIVersion)
                }
              }
            }
          }
          post {
            success {
              script {
                if (currentBuild.currentResult == "SUCCESS") {
                  def artifacts = load ".jenkinsci/artifacts.groovy"
                  def commit = env.GIT_COMMIT
                  if (params.JavaBindings) {
                    javaBindingsFilePaths = [ '/tmp/${GIT_COMMIT}/bindings-artifact/java-bindings-*.zip' ]
                    artifacts.uploadArtifacts(javaBindingsFilePaths, '/iroha/bindings/java')
                  }
                  if (params.PythonBindings) {
                    pythonBindingsFilePaths = [ '/tmp/${GIT_COMMIT}/bindings-artifact/python-bindings-*.zip' ]
                    artifacts.uploadArtifacts(pythonBindingsFilePaths, '/iroha/bindings/python')
                  }
                  if (params.AndroidBindings) {
                    androidBindingsFilePaths = [ '/tmp/${GIT_COMMIT}/bindings-artifact/android-bindings-*.zip' ]
                    artifacts.uploadArtifacts(androidBindingsFilePaths, '/iroha/bindings/android')
                  }
                }
              }
            }
          }
        }
        stage ('windows_bindings') {
          when {
            beforeAgent true
            expression { return params.x86_64_win }
            expression { return REST_PR_CONDITIONS_SATISFIED == "true" }
          }
          agent { label 'win' }
          steps {
            script {
              def bindings = load ".jenkinsci/bindings.groovy"
              if (params.JavaBindings) {
                bindings.doJavaBindings('windows', params.JBBuildType)
              }
              if (params.PythonBindings) {
                bindings.doPythonBindings('windows', params.PBBuildType)
              }
            }
          }
          post {
            success {
              script {
                def artifacts = load ".jenkinsci/artifacts.groovy"
                def commit = env.GIT_COMMIT
                if (params.JavaBindings) {
                  javaBindingsFilePaths = [ '/tmp/${GIT_COMMIT}/bindings-artifact/java-bindings-*.zip' ]
                  artifacts.uploadArtifacts(javaBindingsFilePaths, '/iroha/bindings/java')
                }
                if (params.PythonBindings) {
                  pythonBindingsFilePaths = [ '/tmp/${GIT_COMMIT}/bindings-artifact/python-bindings-*.zip' ]
                  artifacts.uploadArtifacts(pythonBindingsFilePaths, '/iroha/bindings/python')
                }
              }
            }
          }
        }
      }
    }
  }
  post {
    success {
      script {
        // merge pull request if everything is ok and clean stale docker images stored on EFS
        if ( params.merge_pr ) {
          def merge = load ".jenkinsci/github-api.groovy"
          if (merge.mergePullRequest()) {
            currentBuild.result = "SUCCESS"
            def clean = load ".jenkinsci/docker-cleanup.groovy"
            clean.doStaleDockerImagesCleanup()
          }
          else {
            currentBuild.result = "FAILURE"
          }
        }
      }
    }
    cleanup {
      script {
        def post = load ".jenkinsci/post-step.groovy"
        def clean = load ".jenkinsci/docker-cleanup.groovy"
        def notify = load ".jenkinsci/notifications.groovy"
        notify.notifyBuildResults()

        if (params.x86_64_linux || params.merge_pr) {
          node ('x86_64_aws_test') {
            post.cleanUp()
          }
        }
        if (params.armv8_linux) {
          node ('armv8') {
            post.cleanUp()
            clean.doDockerCleanup()
          }
        }
        if (params.armv7_linux) {
          node ('armv7') {
            post.cleanUp()
            clean.doDockerCleanup()
          }
        }
        if (params.x86_64_macos || params.merge_pr) {
          node ('mac') {
            post.macCleanUp()
          }
        }
        if (params.x86_64_win || params.merge_pr) {
          node ('win') {
            post.cleanUp()
          }
        }
      }
    }
  }
}
