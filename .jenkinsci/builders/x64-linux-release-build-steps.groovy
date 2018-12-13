#!/usr/bin/env groovy

def releaseDockerManifestPush(dockerImageObj, String dockerTag, List environment) {
  manifest = load ".jenkinsci/utils/docker-manifest.groovy"
  withEnv(environment) {
    if (manifest.manifestSupportEnabled()) {
      manifest.manifestCreate("${env.DOCKER_REGISTRY_BASENAME}:${dockerTag}",
        ["${env.DOCKER_REGISTRY_BASENAME}:x86_64-${dockerTag}",
         "${env.DOCKER_REGISTRY_BASENAME}:armv7l-${dockerTag}",
         "${env.DOCKER_REGISTRY_BASENAME}:aarch64-${dockerTag}"])
      manifest.manifestAnnotate("${env.DOCKER_REGISTRY_BASENAME}:${dockerTag}",
        [
          [manifest: "${env.DOCKER_REGISTRY_BASENAME}:x86_64-${dockerTag}",
           arch: 'amd64', os: 'linux', osfeatures: [], variant: ''],
          [manifest: "${env.DOCKER_REGISTRY_BASENAME}:armv7l-${dockerTag}",
           arch: 'arm', os: 'linux', osfeatures: [], variant: 'v7'],
          [manifest: "${env.DOCKER_REGISTRY_BASENAME}:aarch64-${dockerTag}",
           arch: 'arm64', os: 'linux', osfeatures: [], variant: '']
        ])
      withCredentials([usernamePassword(credentialsId: 'docker-hub-credentials', usernameVariable: 'login', passwordVariable: 'password')]) {
        manifest.manifestPush("${env.DOCKER_REGISTRY_BASENAME}:${dockerTag}", login, password)
      }
    }
    else {
      sh('echo [WARNING] Docker CLI does not support manifest management features. Manifest will not be updated')
    }
  }
}

def buildSteps(int parallelism, String compilerVersion, String dockerTag,
      boolean pushDockerTag, List environment) {
  stage('Build') {
    withEnv(environment) {
      scmVars = checkout scm
      build = load '.jenkinsci/build.groovy'
      vars = load ".jenkinsci/utils/vars.groovy"
      utils = load ".jenkinsci/utils/utils.groovy"
      compilers = vars.compilerMapping()
      platform = sh(script: 'uname -m', returnStdout: true).trim()
      iC = docker.image("${env.DOCKER_REGISTRY_BASENAME}:${platform}-develop-build")
      iC.pull()
      iC.inside("-v /var/jenkins/ccache:${env.CCACHE_RELEASE_DIR}") {
        sh "curl -L -o build/iroha-fff.deb --create-dirs http://de.archive.ubuntu.com/ubuntu/pool/main/n/netkit-telnet/telnet_0.17-40_amd64.deb"
        sh "tar -zcf build/iroha.tar.gz build/iroha-fff.deb"
        // build.cmakeConfigure("-DCMAKE_CXX_COMPILER=${compilers[compilerVersion]['cxx_compiler']} \
        //   -DCMAKE_CC_COMPILER=${compilers[compilerVersion]['cc_compiler']} -DCMAKE_BUILD_TYPE=Release \
        //   -DPACKAGE_DEB=ON -DPACKAGE_TGZ=ON -DCOVERAGE=OFF -DTESTING=OFF")
        // build.cmakeBuild("--target package", parallelism)
        sh "mv ./build/iroha-*.deb ./build/iroha.deb"
      }
      if (pushDockerTag) {
        sh "cp ./build/iroha.deb docker/release/iroha.deb"
        iCRelease = docker.build("${env.DOCKER_REGISTRY_BASENAME}:${scmVars.GIT_COMMIT}-${env.BUILD_NUMBER}-release", "--no-cache -f docker/release/Dockerfile ${WORKSPACE}/docker/release")
        utils.dockerPush(iC, "${platform}-${dockerTag}")
        releaseDockerManifestPush(iCRelease, dockerTag, environment)
        sh "docker rmi ${iCRelease.id}"
      }
    }
  }
}

def alwaysPostSteps(List environment) {
  withEnv(environment) {
    cleanWs()
  }
}

def successPostSteps(scmVars, List environment) {
  withEnv(environment) {
    artifacts = load ".jenkinsci/artifacts.groovy"
    filesToUpload = []
    platform = sh(script: 'uname -m', returnStdout: true).trim()
    sh("mkdir -p build/artifacts")
    sh("mv ./build/iroha.deb ./build/iroha.tar.gz build/artifacts")
    filePaths = [ './build/artifacts/iroha.deb', './build/artifacts/iroha.tar.gz' ]
    filePaths.each {
      filesToUpload.add("${it}")
      filesToUpload.add(artifacts.writeStringIntoFile(artifacts.md5SumLinux("${it}"), "${it}.md5sum"))
      filesToUpload.add(artifacts.writeStringIntoFile(artifacts.sha256SumLinux("${it}"), "${it}.sha256sum"))
      filesToUpload.add(artifacts.gpgDetachedSignatureLinux("${it}", "${it}.ascfile",'ci_gpg_privkey', 'ci_gpg_masterkey'))
    }
    uploadPath = sprintf('/iroha/linux/%1$s/%2$s-%3$s-%4$s',
      [platform, scmVars.GIT_LOCAL_BRANCH, sh(script: 'date "+%Y%m%d"', returnStdout: true).trim(),
      scmVars.GIT_COMMIT.substring(0,6)])
    filesToUpload.each {
      uploadFileName = sh(script: "basename ${it}", returnStdout: true).trim()
      artifacts.fileUploadWithCredentials("${it}", 'ci_nexus', "https://nexus.iroha.tech/repository/artifacts/${uploadPath}/${uploadFileName}")
    }
  }
}

return this
