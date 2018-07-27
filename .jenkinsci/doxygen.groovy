#!/usr/bin/env groovy

def doDoxygen() {
  if (env.GIT_LOCAL_BRANCH ==~ /(master|develop)/) {
    sh "/usr/bin/doxygen Doxyfile"
    sshagent(['jenkins-artifact']) {
      sh "ssh-agent"
      sh """
        rsync \
        -e 'ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' \
        -rzcv --delete \
        docs/doxygen/html/* \
        ubuntu@nexus.soramitsu.co.jp:/var/nexus-efs/doxygen/${env.GIT_LOCAL_BRANCH}/
      """
    }
  }
}

return this