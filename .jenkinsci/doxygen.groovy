#!/usr/bin/env groovy

def doDoxygen() {

    sh "doxygen Doxyfile"
    sshagent(['jenkins-artifact']) {
    sh "ssh-agent"
        sh "rsync -auzc docs/doxygen/html/* ubuntu@nexus.soramitsu.co.jp:/var/nexus-efs/doxygen/"
  }
}

return this
