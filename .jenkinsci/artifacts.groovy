#!/usr/bin/env groovy

def uploadArtifacts(filePaths, uploadPath, artifactServers=['artifact.soramitsu.co.jp']) {
  def baseUploadPath = 'files'
  def filePathsConverted = []
  agentType = sh(script: 'uname', returnStdout: true).trim()
  uploadPath = baseUploadPath + uploadPath
  filePaths.each {
    fp = sh(script: "ls -d ${it} | tr '\n' ','", returnStdout: true).trim()
    filePathsConverted.addAll(fp.split(','))
  }
  def shaSumBinary = 'sha256sum'
  def md5SumBinary = 'md5sum'
  if (agentType == 'Darwin') {
    shaSumBinary = 'shasum -a 256'
    md5SumBinary = 'md5 -r'
  }
  sh "> \$(pwd)/batch.txt"
  filePathsConverted.each {
    sh "echo put ${it} $uploadPath >> \$(pwd)/batch.txt;"
    sh "$shaSumBinary ${it} | cut -d' ' -f1 > \$(pwd)/\$(basename ${it}).sha256"
    sh "$md5SumBinary ${it} | cut -d' ' -f1 > \$(pwd)/\$(basename ${it}).md5"
    sh "echo put \$(pwd)/\$(basename ${it}).sha256 $uploadPath >> \$(pwd)/batch.txt;"
    sh "echo put \$(pwd)/\$(basename ${it}).md5 $uploadPath >> \$(pwd)/batch.txt;"
  }
  // mkdirs recursively
  uploadPath = uploadPath.split('/')
  def p = ''
  sh "> \$(pwd)/mkdirs.txt"
  uploadPath.each {
    p += "/${it}"
    sh("echo -mkdir $p >> \$(pwd)/mkdirs.txt")
  }

  sshagent(['jenkins-artifact']) {
    sh "ssh-agent"
    artifactServers.each {
      sh "sftp -b \$(pwd)/mkdirs.txt jenkins@${it} || true"
      sh "sftp -b \$(pwd)/batch.txt jenkins@${it}"
    }
  }
}

return this
