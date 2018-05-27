// Overall pipeline looks like the following
//
// |--Pre-build jobs--|----BUILD----|--Pre-Coverage--|------Test------|-Post Coverage-|--Build rest--|
//                      |             |                |                |                |
//                      |-> Linux     |-> lcov         |-> Linux        |-> lcov         |-> gen docs
//                      |-> ARMv7                      |-> ARMv7        |-> SonarQube    |-> bindings
//                      |-> ARMv8                      |-> ARMv8
//                      |-> MacOS                      |-> MacOS
//
// NOTE: In build stage we differentiate only platforms in pipeline scheme. Build/Release is filtered inside the platform
// TODO: limit stage of pipeline for execution: 3 hours
// TODO: download and save docker images to amazon EFS: docker_base_image_{develop,release}
// TODO: pull saved docker images and refresh them in case they are old
// TODO: save docker images loaded previously if they are old (updated after docker pull command)
// (postponed) TODO: limit nightly build pipeline execution for 3 days max
// (pending) TODO: how to do all types of tests on all platforms (create stage in parallel for each platform and test - 4x5=20?!?!)
// TODO: upload artifacts at the post stage of each platform

properties([parameters([
  choice(choices: 'Debug\nRelease', description: '', name: 'BUILD_TYPE'),
  booleanParam(defaultValue: true, description: '', name: 'Linux'),
  booleanParam(defaultValue: false, description: '', name: 'ARMv7'),
  booleanParam(defaultValue: false, description: '', name: 'ARMv8'),
  booleanParam(defaultValue: false, description: '', name: 'MacOS'),
  booleanParam(defaultValue: false, description: 'Whether it is a triggered build', name: 'Nightly'),
  booleanParam(defaultValue: false, description: 'Whether to merge this PR to target after success build', name: 'Merge_PR'),
  booleanParam(defaultValue: false, description: 'Whether to force building coverage', name: 'Coverage'),
  booleanParam(defaultValue: false, description: 'Whether build docs or not', name: 'Doxygen'),
  booleanParam(defaultValue: false, description: 'Whether build Java bindings', name: 'JavaBindings'),
  choice(choices: 'Release\nDebug', description: 'Java Bindings Build Type', name: 'JBBuildType'),
  booleanParam(defaultValue: false, description: 'Whether build Python bindings', name: 'PythonBindings'),
  choice(choices: 'Release\nDebug', description: 'Python Bindings Build Type', name: 'PBBuildType'),
  choice(choices: 'python3\npython2', description: 'Python Bindings Version', name: 'PBVersion'),
  booleanParam(defaultValue: false, description: 'Whether build Android bindings', name: 'AndroidBindings'),
  choice(choices: '26\n25\n24\n23\n22\n21\n20\n19\n18\n17\n16\n15\n14', description: 'Android Bindings ABI Version', name: 'ABABIVersion'),
  choice(choices: 'Release\nDebug', description: 'Android Bindings Build Type', name: 'ABBuildType'),
  choice(choices: 'arm64-v8a\narmeabi-v7a\narmeabi\nx86_64\nx86', description: 'Android Bindings Platform', name: 'ABPlatform'),
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

    dockerAgentImage = ''
    dockerImageFile = ''
    workspace_path = ''
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
          GIT_COMMITER_EMAIL = sh(script: """git --no-pager show -s --format='%ae' ${env.GIT_COMMIT}""", returnStdout: true).trim()
          if (GIT_LOCAL_BRANCH != "develop") {
            def builds = load ".jenkinsci/cancel-builds-same-job.groovy"
            builds.cancelSameJobBuilds()
          }
        }
      }
    }
    stage('Build') {
      parallel {
        stage ('Linux') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.Linux }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'x86_64_aws_build' }
          steps {
            script {
              if ( params.BUILD_TYPE == 'Debug' || params.Merge_PR ) {
                def debugBuild = load ".jenkinsci/debug-build.groovy"
                def coverage = load ".jenkinsci/selected-branches-coverage.groovy"
                // if ( params.Merge_PR ) { debugBuild.doDebugBuild(true) }
                // else { debugBuild.doDebugBuild(coverage.selectedBranchesCoverage()) }
              }
              else {
                def releaseBuild = load ".jenkinsci/release-build.groovy"
                // releaseBuild.doReleaseBuild()
              }
            }
          }
        }
        stage('ARMv7') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.ARMv7 }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'armv7' }
          steps {
            script {
              if ( params.BUILD_TYPE == 'Debug' || params.Merge_PR ) {
                def debugBuild = load ".jenkinsci/debug-build.groovy"
                def coverage = load ".jenkinsci/selected-branches-coverage.groovy"
                // debugBuild.doDebugBuild( (!params.Linux && !params.MacOS && !params.ARMv8 || !params.Merge_PR) ? coverage.selectedBranchesCoverage() : false )
              }
              else {
                def releaseBuild = load ".jenkinsci/release-build.groovy"
                // releaseBuild.doReleaseBuild()
              }
            }
          }
        }
        stage('ARMv8') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.ARMv8 }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'armv8' }
          steps {
            script {
              if ( params.BUILD_TYPE == 'Debug' || params.Merge_PR ) {
                def debugBuild = load ".jenkinsci/debug-build.groovy"
                def coverage = load ".jenkinsci/selected-branches-coverage.groovy"
                // debugBuild.doDebugBuild( (!params.Linux && !params.MacOS || !params.Merge_PR) ? coverage.selectedBranchesCoverage() : false )
              }
              else {
                def releaseBuild = load ".jenkinsci/release-build.groovy"
                // releaseBuild.doReleaseBuild()
              }
            }
          }
        }
        stage('MacOS') {
          when {
            beforeAgent true
            anyOf {
              allOf {
                expression { return env.CHANGE_ID != null }
                not { expression { return env.GIT_PREVIOUS_COMMIT != null } }
              }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
                expression { return params.Merge_PR }
              }
              expression { return params.MacOS }
            }
          }
          agent { label 'mac' }
          steps {
            script {
              if ( params.BUILD_TYPE == 'Debug' || params.Merge_PR ) {
                def macDebugBuild = load ".jenkinsci/mac-debug-build.groovy"
                def coverage = load ".jenkinsci/selected-branches-coverage.groovy"
                // macDebugBuild.doDebugBuild( (!params.Linux || !params.Merge_PR) ? coverage.selectedBranchesCoverage() : false )
              }
              else {
                def macReleaseBuild = load ".jenkinsci/mac-release-build.groovy"
                // macReleaseBuild.doReleaseBuild()
              }
            }
          }
        }
      }
    }
    stage('Pre-Coverage') {
      when {
        anyOf {
          expression { params.Coverage }  // by request
          allOf {
            expression { return env.CHANGE_ID }
            not { expression { return env.GIT_PREVIOUS_COMMIT } }
          }
          allOf {
            expression { return env.CHANGE_ID != null}
            expression { return env.GIT_PREVIOUS_COMMIT != null }
            // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
            expression { return params.Merge_PR }
          }
          allOf {
            expression { return params.BUILD_TYPE == 'Debug' }
            expression { return env.GIT_LOCAL_BRANCH ==~ /master/ }
          }
        }
      }
      steps {
        script {
          def cov_platform = ''
          if (params.Linux) {
            cov_platform = 'x86_64_aws_cov'
          }
          if (!params.Linux && params.MacOS) {
            cov_platform = 'mac'
          }
          if (!params.Linux && !params.MacOS && params.ARMv8) {
            cov_platform = 'armv8'
          }
          else if (!params.Linux && !params.MacOS && !params.ARMv8) {
            cov_platform = 'armv7'
          }
          node(cov_platform) {
            if (cov_platform == 'mac') {
              coverage = load '.jenkinsci/mac-debug-build.groovy'
            }
            else {
              coverage = load '.jenkinsci/debug-build.groovy'
            }
            // coverage.doPreCoverageStep()
          }
        }
      }
    }
    stage('Tests') {
      when {
        beforeAgent true
        anyOf {
          expression { return params.BUILD_TYPE == "Debug" }
        }
      }
      parallel {
        stage('Linux') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.Linux }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'x86_64_aws_test' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              // debugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
        }
        stage('ARMv7') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.ARMv7 }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'armv7' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              // debugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
        }
        stage('ARMv8') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.ARMv8 }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          agent { label 'armv8' }
          steps {
            script {
              def debugBuild = load ".jenkinsci/debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              // debugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
        }
        stage('MacOS') {
          when {
            beforeAgent true
            anyOf {
              allOf {
                expression { return env.CHANGE_ID }
                not { expression { return env.GIT_PREVIOUS_COMMIT } }
              }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
                expression { return params.Merge_PR }
              }
              expression { return params.MacOS }
            }
          }
          agent { label 'mac' }
          steps {
            script {
              def macDebugBuild = load ".jenkinsci/mac-debug-build.groovy"
              def testSelect = load ".jenkinsci/test-launcher.groovy"
              // macDebugBuild.doTestStep(testSelect.chooseTestType())
            }
          }
        }
      }
    }
    stage('Post-Coverage') {
      when {
        anyOf {
          expression { params.Coverage }  // by request
          allOf {
            expression { return env.CHANGE_ID }
            not { expression { return env.GIT_PREVIOUS_COMMIT } }
          }
          allOf {
            expression { return env.CHANGE_ID != null}
            expression { return env.GIT_PREVIOUS_COMMIT != null }
            // // expression { return env.CHANGE_TARGET ==~ /(master|develop|trunk|temp)/ }
            expression { return params.Merge_PR }
          }
          allOf {
            expression { return params.BUILD_TYPE == 'Debug' }
            expression { return env.GIT_LOCAL_BRANCH ==~ /master/ }
          }
        }
      }
      parallel {
        stage('lcov_cobertura') {
          steps {
            script {
              def cov_platform = ''
              if (params.Linux) {
                cov_platform = 'x86_64_aws_cov'
              }
              if (!params.Linux && params.MacOS) {
                cov_platform = 'mac'
              }
              if (!params.Linux && !params.MacOS && params.ARMv8) {
                cov_platform = 'armv8'
              }
              else if (!params.Linux && !params.MacOS && !params.ARMv8) {
                cov_platform = 'armv7'
              }
              node(cov_platform) {
                if (cov_platform == 'mac') {
                  coverage = load '.jenkinsci/mac-debug-build.groovy'
                }
                else {
                  coverage = load '.jenkinsci/debug-build.groovy'
                }
                  // coverage.doPostCoverageCoberturaStep()
              }
            }
          }
        }
        stage('sonarqube') {
          when {
            anyOf {
              expression { params.Coverage }  // by request
              allOf {
                expression { return env.CHANGE_ID }
                not { expression { return env.GIT_PREVIOUS_COMMIT } }
              }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return params.Merge_PR }
              }
              allOf {
                expression { return params.BUILD_TYPE == 'Debug' }
                expression { return env.GIT_LOCAL_BRANCH ==~ /master/ }
              }
            }
          } 
          steps {
            script {
              def cov_platform = ''
              if (params.Linux) {
                cov_platform = 'x86_64_aws_cov'
              }
              if (!params.Linux && params.MacOS) {
                cov_platform = 'mac'
              }
              if (!params.Linux && !params.MacOS && params.ARMv8) {
                cov_platform = 'armv8'
              }
              else if (!params.Linux && !params.MacOS && !params.ARMv8) {
                cov_platform = 'armv7'
              }
              node(cov_platform) {
                if (cov_platform == 'mac') {
                  coverage = load '.jenkinsci/mac-debug-build.groovy'
                }
                else {
                  coverage = load '.jenkinsci/debug-build.groovy'
                }
                // coverage.doPostCoverageSonarStep()
              }
            }
          }
        }
      }
    }
    stage ('Build rest') {
      parallel {
        stage('Build release') {
          when {
            allOf {
              expression { return params.BUILD_TYPE == 'Debug' }
              expression { return env.GIT_LOCAL_BRANCH ==~ /(develop|master|trunk)/ }
            }
          }
          steps{
            script {
              def build_platform = ''
              if (params.Linux) {
                build_platform = 'x86_64_aws_build'
              }
              if (!params.Linux && params.MacOS) {
                build_platform = 'mac'
              }
              if (!params.Linux && !params.MacOS && params.ARMv8) {
                build_platform = 'armv8'
              }
              else if (!params.Linux && !params.MacOS && !params.ARMv8) {
                build_platform = 'armv7'
              }
              node(build_platform) {
                if (build_platform == 'mac') {
                  releaseBuild = load ".jenkinsci/mac-release-build.groovy"
                }
                else {
                  releaseBuild = load ".jenkinsci/release-build.groovy"
                }
                releaseBuild.doReleaseBuild()
              }
            }
          }
        }
        stage('Build docs') {
          when {
            beforeAgent true
            anyOf {
              allOf {
                expression { return params.Doxygen }
                expression { return GIT_LOCAL_BRANCH ==~ /(master|develop|trunk)/ }
              }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return env.CHANGE_TARGET ==~ /(master|develop)/ }
                expression { return params.Merge_PR }
              }
            }
          }
          // build docs on any vacant node. Prefer `x86_64` over
          // others as nodes are more powerful
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
        stage('Build bindings') {
          when {
            beforeAgent true
            anyOf {
              expression { return params.PythonBindings }
              expression { return params.JavaBindings }
              expression { return params.AndroidBindings }
              allOf {
                expression { return env.CHANGE_ID != null}
                expression { return env.GIT_PREVIOUS_COMMIT != null }
                expression { return env.CHANGE_TARGET ==~ /(master|develop)/ }
                expression { return params.Merge_PR }
              }
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
            always {
              timeout(time: 600, unit: "SECONDS") {
                script {
                  try {
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
                  finally {
                    sh "rm -rf /tmp/${env.GIT_COMMIT}"
                    // cleanWs()
                  }
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
        // merge pull request if everything is ok
        if ( params.MERGE_PR ) {
          def merge = load ".jenkinsci/github-api.groovy"
          currentBuild.result = merge.mergePullRequest() ? "SUCCESS" : "FAILURE"
        }
      }
    }
    cleanup {
      script {
        def notify = load ".jenkinsci/notifications.groovy"
        notify.notifyBuildResults()

        if ( params.Linux ) {
          node ('x86_64_aws_test') {
            def post = load ".jenkinsci/linux-post-step.groovy"
            post.linuxPostStep()
          }
        }
        if ( params.ARMv8 ) {
          node ('armv8') {
            def post = load ".jenkinsci/linux-post-step.groovy"
            post.linuxPostStep()
          }
        }
        if ( params.ARMv7 ) {
          node ('armv7') {
            def post = load ".jenkinsci/linux-post-step.groovy"
            post.linuxPostStep()
          }
        }
        if ( params.MacOS ) {
          node ('mac') {
            def post = load ".jenkinsci/linux-post-step.groovy"
            post.linuxPostStep()
          }
        }

        sh """echo "Build duration: ${currentBuild.durationString}" """
      }
    }
  }
}
