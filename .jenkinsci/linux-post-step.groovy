def linuxPostStep() {
  timeout(time: 600, unit: "SECONDS") {
    try {
      if (currentBuild.currentResult == "SUCCESS" && GIT_LOCAL_BRANCH ==~ /(master|develop)/) {
        def artifacts = load ".jenkinsci/artifacts.groovy"
        def commit = env.GIT_COMMIT
        if ( env.NODE_NAME.contains('mac') ) {
          filePaths = [ '\$(pwd)/build/*.tar.gz' ]
          artifacts.uploadArtifacts(filePaths, sprintf('/iroha/macos/%1$s-%2$s-%3$s', [GIT_LOCAL_BRANCH, sh(script: 'date "+%Y%m%d"', returnStdout: true).trim(), commit.substring(0,6)]))
        }
        else {
          def platform = sh(script: 'uname -m', returnStdout: true).trim()
          filePaths = [ '/tmp/${GIT_COMMIT}-${BUILD_NUMBER}/*' ]
          artifacts.uploadArtifacts(filePaths, sprintf('/iroha/linux/%4$s/%1$s-%2$s-%3$s', [GIT_LOCAL_BRANCH, sh(script: 'date "+%Y%m%d"', returnStdout: true).trim(), commit.substring(0,6), platform]))
        }
        
      }
    }
    finally {
      if ( env.NODE_NAME.contains('mac') ) {
        sh """
          pg_ctl -D /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/ stop && \
          rm -rf /var/jenkins/${GIT_COMMIT}-${BUILD_NUMBER}/
        """
      }
      else {
        if ( ! env.NODE_NAME.contains('x86_64') ) {
          def cleanup = load ".jenkinsci/docker-cleanup.groovy"
          cleanup.doDockerCleanup()
        }
      }
      cleanWs()
    }
  }
}

return this
