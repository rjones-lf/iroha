#!/usr/bin/env groovy

def dockerManifestPush(dockerImageObj, String dockerTag, environment) {
  manifest = load ".jenkinsci/utils/docker-manifest.groovy"
  withEnv(environment) {
    if (manifest.manifestSupportEnabled()) {
      manifest.manifestCreate("${env.DOCKER_REGISTRY_BASENAME}:${dockerTag}",
        ["${env.DOCKER_REGISTRY_BASENAME}:x86_64-${dockerTag}"])
      manifest.manifestAnnotate("${env.DOCKER_REGISTRY_BASENAME}:${dockerTag}",
        [
          [manifest: "${env.DOCKER_REGISTRY_BASENAME}:x86_64-${dockerTag}",
           arch: 'amd64', os: 'linux', osfeatures: [], variant: ''],
        ])
      withCredentials([usernamePassword(credentialsId: 'docker-hub-credentials', usernameVariable: 'login', passwordVariable: 'password')]) {
        manifest.manifestPush("${env.DOCKER_REGISTRY_BASENAME}:${dockerTag}", login, password)
      }
    }
    else {
      echo('[WARNING] Docker CLI does not support manifest management features. Manifest will not be updated')
    }
  }
}

def testSteps(String buildDir, List environment, String testList) {
  withEnv(environment) {
    sh "cd ${buildDir}; ctest --output-on-failure --no-compress-output --tests-regex '${testList}'  --test-action Test || true"
    sh "python .jenkinsci/helpers/platform_tag.py 'Linux \$(uname -m)' \$(ls ${buildDir}/Testing/*/Test.xml)"
    // Mark build as UNSTABLE if there are any failed tests (threshold <100%)
    xunit testTimeMargin: '3000', thresholdMode: 2, thresholds: [passed(unstableThreshold: '100')], \
      tools: [CTest(deleteOutputFiles: true, failIfNotNew: false, \
      pattern: "${buildDir}/Testing/**/Test.xml", skipNoTestFiles: false, stopProcessingIfError: true)]
  }
}

def buildSteps(int parallelism, List compilerVersions, String build_type, boolean specialBranch, boolean coverage,
      boolean testing, String testList, boolean cppcheck, boolean sonar, boolean docs, boolean packagebuild, boolean packagePush, boolean sanitize, boolean fuzzing, List environment) {
  withEnv(environment) {
    scmVars = checkout scm
    build = load '.jenkinsci/build.groovy'
    vars = load ".jenkinsci/utils/vars.groovy"
    utils = load ".jenkinsci/utils/utils.groovy"
    dockerUtils = load ".jenkinsci/utils/docker-pull-or-build.groovy"
    doxygen = load ".jenkinsci/utils/doxygen.groovy"
    buildDir = 'build'
    compilers = vars.compilerMapping()
    cmakeBooleanOption = [ (true): 'ON', (false): 'OFF' ]
    platform = sh(script: 'uname -m', returnStdout: true).trim()
    cmakeBuildOptions = ""
    cmakeOptions = ""
    if (packagebuild){
      cmakeBuildOptions = " --target package "
    }
    if (sanitize){
      cmakeOptions += " -DSANITIZE='address;leak' "
    }
    sh "docker network create ${env.IROHA_NETWORK}"
    iC = dockerUtils.dockerPullOrBuild("${platform}-develop-build",
        "${env.GIT_RAW_BASE_URL}/${scmVars.GIT_COMMIT}/docker/develop/Dockerfile",
        "${env.GIT_RAW_BASE_URL}/${utils.previousCommitOrCurrent(scmVars)}/docker/develop/Dockerfile",
        "${env.GIT_RAW_BASE_URL}/dev/docker/develop/Dockerfile",
        scmVars,
        environment,
        ['PARALLELISM': parallelism])
    // enable prepared transactions so that 2 phase commit works
    // we set it to 100 as a safe value
    sh "docker run -td -e POSTGRES_USER=${env.IROHA_POSTGRES_USER} \
    -e POSTGRES_PASSWORD=${env.IROHA_POSTGRES_PASSWORD} --name ${env.IROHA_POSTGRES_HOST} \
    --network=${env.IROHA_NETWORK} postgres:9.5 -c 'max_prepared_transactions=100'"
    iC.inside(""
    + " -e IROHA_POSTGRES_HOST=${env.IROHA_POSTGRES_HOST}"
    + " -e IROHA_POSTGRES_PORT=${env.IROHA_POSTGRES_PORT}"
    + " -e IROHA_POSTGRES_USER=${env.IROHA_POSTGRES_USER}"
    + " -e IROHA_POSTGRES_PASSWORD=${env.IROHA_POSTGRES_PASSWORD}"
    + " --network=${env.IROHA_NETWORK}"
    + " -v /var/jenkins/ccache:${env.CCACHE_DEBUG_DIR}") {
      utils.ccacheSetup(5)
      for (compiler in compilerVersions) {
        stage ("build ${compiler}"){
          build.cmakeConfigure(buildDir, "-DCMAKE_CXX_COMPILER=${compilers[compiler]['cxx_compiler']} \
            -DCMAKE_C_COMPILER=${compilers[compiler]['cc_compiler']} \
            -DCMAKE_BUILD_TYPE=${build_type} \
            -DCOVERAGE=${cmakeBooleanOption[coverage]} \
            -DTESTING=${cmakeBooleanOption[testing]} \
            -DFUZZING=${cmakeBooleanOption[fuzzing]} \
            -DPACKAGE_DEB=${cmakeBooleanOption[packagebuild]} \
            -DPACKAGE_TGZ=${cmakeBooleanOption[packagebuild]} ${cmakeOptions}")
          build.cmakeBuild(buildDir, cmakeBuildOptions, parallelism)
        }
        if (testing) {
          stage("Test ${compiler}") {
            coverage ? build.initialCoverage(buildDir) : echo('Skipping initial coverage...')
            testSteps(buildDir, environment, testList)
            coverage ? build.postCoverage(buildDir, '/tmp/lcov_cobertura.py') : echo('Skipping post coverage...')
          }
        }
      }
      stage("Analysis") {
            cppcheck ? build.cppCheck(buildDir, parallelism) : echo('Skipping Cppcheck...')
            sonar ? build.sonarScanner(scmVars, environment) : echo('Skipping Sonar Scanner...')
      }
      stage('Build docs'){
        docs ? doxygen.doDoxygen(specialBranch, scmVars.GIT_LOCAL_BRANCH) : echo("Skipping Doxygen...")
      }
      stage ('DockerManifestPush'){
        if (specialBranch) {
          utils.dockerPush(iC, "${platform}-develop-build")
          dockerManifestPush(iC, "develop-build", environment)
        }
      }
    }
  }
}

def successPostSteps(scmVars, String build_type, boolean packagePush, String dockerTag, List environment) {
  stage('successPostSteps') {
    withEnv(environment) {
      artifacts = load ".jenkinsci/artifacts.groovy"
      //utils = load ".jenkinsci/utils/utils.groovy"
      filesToUpload = []
      platform = sh(script: 'uname -m', returnStdout: true).trim()
      if (packagePush) {
        // if we use several compiler only last build  will saved as iroha.deb and iroha.tar.gz
        sh """
          mv ./build/iroha-*.deb ./build/iroha.deb
          mv ./build/iroha-*.tar.gz ./build/iroha.tar.gz
          cp ./build/iroha.deb docker/release/iroha.deb
          mkdir -p build/artifacts
          mv ./build/iroha.deb ./build/iroha.tar.gz build/artifacts
        """
        // publish docker
        iCRelease = docker.build("${env.DOCKER_REGISTRY_BASENAME}:${scmVars.GIT_COMMIT}-${env.BUILD_NUMBER}-release", "--no-cache -f docker/release/Dockerfile ${WORKSPACE}/docker/release")
        utils.dockerPush(iCRelease, "${platform}-${dockerTag}")
        dockerManifestPush(iCRelease, dockerTag, environment)
        sh "docker rmi ${iCRelease.id}"

        // publish packages
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
      } else {
        archiveArtifacts artifacts: 'build/iroha*.tar.gz', allowEmptyArchive: true
        archiveArtifacts artifacts: 'build/iroha*.deb', allowEmptyArchive: true
      }
    }
  }
}

def alwaysPostSteps(List environment) {
  stage('alwaysPostSteps') {
    withEnv(environment) {
      sh "docker rm -f ${env.IROHA_POSTGRES_HOST} || true"
      sh "docker network rm ${env.IROHA_NETWORK}"
      cleanWs()
    }
  }
}

return this
