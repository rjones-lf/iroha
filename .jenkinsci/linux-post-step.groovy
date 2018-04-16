def linuxPostStep() {
  timeout(time: 600, unit: "SECONDS") {
    try {
      if (currentBuild.result != "UNSTABLE" && BRANCH_NAME ==~ /(master|develop)/) {
        def artifacts = load ".jenkinsci/artifacts.groovy"
        def commit = env.GIT_COMMIT
        def platform = sh(script: 'uname -m', returnStdout: true).trim()
        filePaths = [ '/tmp/${GIT_COMMIT}-${BUILD_NUMBER}/*' ]
        artifacts.uploadArtifacts(filePaths, sprintf('/iroha/linux/%4$s/%1$s-%2$s-%3$s', [BRANCH_NAME, sh(script: 'date "+%Y%m%d"', returnStdout: true).trim(), commit.substring(0,6), platform]))
      }
    }
    finally {
      if (env.BUILD_TYPE == 'Debug') {
        def cleanup = load ".jenkinsci/docker-cleanup.groovy"
        cleanup.doDockerCleanup()
      }
      cleanWs()
    }
  }
}

return this
