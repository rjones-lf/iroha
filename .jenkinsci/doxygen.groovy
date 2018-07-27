#!/usr/bin/env groovy

def doDoxygen() {
  try {
    if (env.GIT_LOCAL_BRANCH ==~ /(master|develop)/ || env.CHANGE_BRANCH == 'develop') {
      def branch = env.CHANGE_BRANCH == 'develop' ? env.CHANGE_BRANCH : env.GIT_LOCAL_BRANCH
      sh "doxygen Doxyfile"
      sshagent(['jenkins-artifact']) {
        sh "ssh-agent"
        sh """
          rsync \
          -e 'ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' \
          -rzcv --delete \
          docs/doxygen/html/* \
          ubuntu@nexus.soramitsu.co.jp:/var/nexus-efs/doxygen/${branch}/
        """
      }
    }
  }
}

return this
