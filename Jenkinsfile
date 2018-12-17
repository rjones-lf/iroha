def tasks = [:]

class Worker {
  String label
  int cpusAvailable
}

class Builder {
  // can't get to work without 'static'
  static class PostSteps {
    List success
    List failure
    List unstable
    List always
    List aborted
  }
  List buildSteps
  PostSteps postSteps
}

class Build {
  String name
  String type
  Builder builder
  Worker worker
}

def build(Build build) {
  return {
    node(build.worker.label) {
      try {
        build.builder.buildSteps.each {
          it()
        }
        build.builder.postSteps.success.each {
          it()
        }
      } catch(Exception e) {
        if (currentBuild.currentResult == 'SUCCESS') {
            print "Error: " + e
            currentBuild.result = 'FAILURE'
        }
        else if(currentBuild.currentResult == 'UNSTABLE') {
          build.builder.postSteps.unstable.each {
            it()
          }
        }
        else if(currentBuild.currentResult == 'FAILURE') {
          build.builder.postSteps.failure.each {
            it()
          }
        }
        else if(currentBuild.currentResult == 'ABORTED') {
          build.builder.postSteps.aborted.each {
            it()
          }
        }
      }
      // ALWAYS
      finally {
        build.builder.postSteps.always.each {
          it()
        }
      }
    }
  }
}

def getTestList() {
    list = []
    prefix = 'test_'
    for (i in  params){
        if(i.key.startsWith(prefix) && i.value.getClass() == Boolean &&  i.value){
            list += i.key.minus(prefix)
        }
    }
    status = !list.isEmpty()
    line = ( "(" +list.findAll({it != ''}).join('|') + ")")
    return [status, line]
}


stage('Prepare environment'){
timestamps(){

properties([
    parameters([
        choice(choices: 'gcc5\ngcc7\nclang6\ngcc5,gcc7,clang6\n', description: 'x64 Linux Compiler', name: 'x64linux_compiler'),
        //TODO add x32 Linux Machine
        //choice(choices: '\ngcc5\ngcc5,gcc7,clang6', description: 'x32 Linux Compiler', name: 'x32linux_compiler'),
        choice(choices: '\nappleclang', description: 'MacOS Compiler', name: 'mac_compiler'),
        //TODO Write pipeline for Windows
        //choice(choices: '\nmsvc', description: 'Windows Compiler', name: 'windows_compiler'),
        choice(choices: 'Debug\nRelease', description: 'Iroha build type', name: 'build_type'),

        booleanParam(defaultValue: false, description: '', name: 'coverage'),
        booleanParam(defaultValue: true, description: '', name: 'cppcheck'),
        booleanParam(defaultValue: false, description: '', name: 'sonar'),

        booleanParam(defaultValue: true, description: 'Unit tests', name: 'test_module'),
        booleanParam(defaultValue: false, description: '', name: 'test_integration'),
        booleanParam(defaultValue: false, description: '', name: 'test_system'),
        booleanParam(defaultValue: false, description: '', name: 'test_cmake'),
        booleanParam(defaultValue: false, description: '', name: 'test_regression'),
        booleanParam(defaultValue: false, description: '', name: 'test_benchmark'),
        booleanParam(defaultValue: false, description: 'Sanitize address;leak', name: 'sanitize'),
        booleanParam(defaultValue: false, description: 'Build fuzzing, but do not run tests, use only clang', name: 'fuzzing'),
        booleanParam(defaultValue: false, description: 'Build docs', name: 'Doxygen'),
        //TODO in build_type:Debug params.package do NOT work properly, need fix in Cmake(deb/tar.gz is empty, tests empty)
        booleanParam(defaultValue: false, description: 'Build package, for build type Debug, for Release always true', name: 'package'),
    ]),
    buildDiscarder(logRotator(artifactDaysToKeepStr: '', artifactNumToKeepStr: '', daysToKeepStr: '', numToKeepStr: '30'))
])

node ('master') {
  scmVars = checkout scm
  environmentList = []
  environment = [:]
  environment = [
    "CCACHE_DEBUG_DIR": "/opt/.ccache",
    "CCACHE_RELEASE_DIR": "/opt/.ccache",
    "DOCKER_REGISTRY_BASENAME": "hyperledger/iroha",
    "IROHA_NETWORK": "iroha-${scmVars.CHANGE_ID}-${scmVars.GIT_COMMIT}-${env.BUILD_NUMBER}",
    "IROHA_POSTGRES_HOST": "pg-${scmVars.CHANGE_ID}-${scmVars.GIT_COMMIT}-${env.BUILD_NUMBER}",
    "IROHA_POSTGRES_USER": "pguser${scmVars.GIT_COMMIT}",
    "IROHA_POSTGRES_PASSWORD": "${scmVars.GIT_COMMIT}",
    "IROHA_POSTGRES_PORT": "5432",
    "GIT_RAW_BASE_URL": "https://raw.githubusercontent.com/hyperledger/iroha"
  ]
  environment.each { e ->
    environmentList.add("${e.key}=${e.value}")
  }

  // Define variable and params
  (testing,testList) = getTestList()

  if (params.build_type == 'Release') {
    packageBuild = true
  } else {
    packageBuild = params.package
  }
  if (params.fuzzing){
    x64linux_compiler= 'clang6'
    testing = false
  } else {
    x64linux_compiler =  params.x64linux_compiler
  }
  echo "packageBuild=${packageBuild}"
  echo "testing=${testing}, testList=${testList}"

  // Load Scripts
  x64LinuxBuildScript = load '.jenkinsci/builders/x64-linux-build-steps.groovy'
  x64BuildScript = load '.jenkinsci/builders/x64-mac-build-steps.groovy'

  // Define Workers
  x64LinuxWorker = new Worker(label: 'x86_64', cpusAvailable: 4)
  x64MacWorker = new Worker(label: 'mac', cpusAvailable: 4)


  // Define all possible steps
  def x64LinuxBuildSteps
  def x64LinuxPostSteps = new Builder.PostSteps()
  if(params.x64linux_compiler){
    x64LinuxBuildSteps = [{x64LinuxBuildScript.buildSteps(
      x64LinuxWorker.cpusAvailable, x64linux_compiler, params.build_type, false, 'Not_used', params.coverage, testing, testList, params.cppcheck, params.sonar, params.Doxygen, packageBuild, params.sanitize, params.fuzzing, environmentList)}]
    x64LinuxPostSteps = new Builder.PostSteps(
      always: [{x64LinuxBuildScript.alwaysPostSteps(environmentList)}],
      success: [{x64LinuxBuildScript.successPostSteps(scmVars, params.build_type, environmentList)}])
  }
  def x64MacBuildSteps
  def x64MacBuildPostSteps = new Builder.PostSteps()
  if(params.mac_compiler){
    x64MacBuildSteps = [{x64BuildScript.buildSteps(x64MacWorker.cpusAvailable, params.mac_compiler, params.build_type, params.coverage, testing, testList, packageBuild, environmentList)}]
    x64MacBuildPostSteps = new Builder.PostSteps(
      always: [{x64BuildScript.alwaysPostSteps(environmentList)}],
      success: [{x64BuildScript.successPostSteps(scmVars, params.build_type, environmentList)}])
  }

  // Define builders
  x64LinuxBuilder = new Builder(buildSteps: x64LinuxBuildSteps, postSteps: x64LinuxPostSteps)
  x64MacBuilder = new Builder(buildSteps: x64MacBuildSteps, postSteps: x64MacBuildPostSteps )

  // Define Build
  x64LinuxBuild = new Build(name: "x86_64 Linux ${params.build_type}",
                                    type: params.build_type,
                                    builder: x64LinuxBuilder,
                                    worker: x64LinuxWorker)
  x64MacBuild = new Build(name: "Mac ${params.build_type}",
                                     type: params.build_type,
                                     builder: x64MacBuilder,
                                     worker: x64MacWorker)

  tasks[x64LinuxBuild.name] = build(x64LinuxBuild)
  tasks[x64MacBuild.name] = build(x64MacBuild)
  cleanWs()
  parallel tasks

  // if (currentBuild.currentResult == 'SUCCESS') {
  //   if (scmVars.CHANGE_ID) {
  //     if(scmVars.CHANGE_BRANCH == 'feature/ready-dev-experimental') {
  //       sh 'echo PUSH!'
  //     }
  //   }
  // }
}

}
}