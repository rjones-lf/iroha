#!/usr/bin/env groovy

def doDoxygen() {

  sh "/usr/bin/doxygen Doxyfile"

  if (env.GIT_LOCAL_BRANCH ==~ /(master|develop)/) {
    sshagent(['jenkins-artifact']) {
      sh "ssh-agent"
      sh """
        rsync \
        -e 'ssh -vv -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' \
        -ruzc \
        docs/doxygen/html/* \
        ubuntu@nexus.soramitsu.co.jp:/var/nexus-efs/doxygen/${env.GIT_LOCAL_BRANCH}
      """
    }
  }
}

return this