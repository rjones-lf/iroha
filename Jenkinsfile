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
            print e
            currentBuild.currentResult = 'FAILURE'
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

properties([
    parameters([
        choice(choices: 'gcc54\ngcc54,gcc7,clang6', description: 'x64 Linux Compiler', name: 'x64linux_compiler'),
        choice(choices: '\ngcc54\ngcc54,gcc7,clang6', description: 'x32 Linux Compiler', name: 'x32linux_compiler'),
        choice(choices: '\nappleclang', description: 'MacOS Compiler', name: 'mac_compiler'),
        choice(choices: '\nmsvc', description: 'Windows Compiler', name: 'windows_compiler'),

        booleanParam(defaultValue: false, description: '', name: 'coverage'),
        booleanParam(defaultValue: true, description: '', name: 'cppcheck'),
        booleanParam(defaultValue: false, description: '', name: 'sonar'),

        booleanParam(defaultValue: true, description: 'Unit tests', name: 'test_module'),
        booleanParam(defaultValue: false, description: '', name: 'test_integration'),
        booleanParam(defaultValue: false, description: '', name: 'test_system'),
        booleanParam(defaultValue: false, description: '', name: 'test_cmake'),
        booleanParam(defaultValue: false, description: '', name: 'test_regression'),
        booleanParam(defaultValue: false, description: '', name: 'test_benchmark'),
        booleanParam(defaultValue: false, description: 'Build docs', name: 'Doxygen'),
    ]),
    buildDiscarder(logRotator(artifactDaysToKeepStr: '', artifactNumToKeepStr: '', daysToKeepStr: '', numToKeepStr: '30'))
])


timestamps(){
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

  // Load Scripts
  x64LinuxReleaseBuildScript = load '.jenkinsci/builders/x64-linux-release-build-steps.groovy'
  x64LinuxDebugBuildScript = load '.jenkinsci/builders/x64-linux-debug-build-steps.groovy'

  // Define Workers
  x64LinuxWorker = new Worker(label: 'x86_64', cpusAvailable: 4)
  // def x64MacWorker = new Worker(label: 'mac', cpusAvailable: 4)

  // Define Tests
  (testing,testList) = getTestList()
  echo "testing=${testing}, testList=${testList}"

  // Define all possible steps
  x64LinuxReleaseBuildSteps = [{x64LinuxReleaseBuildScript.buildSteps(
    x64LinuxWorker.cpusAvailable, 'gcc54', 'develop', false, environmentList)}]
  x64LinuxReleasePostSteps = new Builder.PostSteps(
    always: [{x64LinuxReleaseBuildScript.alwaysPostSteps(environmentList)}],
    success: [{x64LinuxReleaseBuildScript.successPostSteps(scmVars, environmentList)}])

  x64LinuxDebugBuildSteps = []
  for (compiler in params.x64linux_compiler.split(',')) {
    if(compiler){
      x64LinuxDebugBuildSteps += {x64LinuxDebugBuildScript.buildSteps(
        x64LinuxWorker.cpusAvailable, compiler, false, params.coverage, testing, testList, params.cppcheck, params.sonar, params.Doxygen, environmentList)}
    }
  }
  x64LinuxDebugPostSteps = new Builder.PostSteps(
    always: [{x64LinuxDebugBuildScript.alwaysPostSteps(environmentList)}])
  //def x64MacReleaseBuildSteps = x64LinuxReleaseBuildScript.buildSteps(x64MacWorker.label, x64MacWorker.cpusAvailable)

  // Define builders
  x64LinuxReleaseBuilder = new Builder(buildSteps: x64LinuxReleaseBuildSteps, postSteps: x64LinuxReleasePostSteps)
  x64LinuxDebugBuilder = new Builder(buildSteps: x64LinuxDebugBuildSteps, postSteps: x64LinuxDebugPostSteps)
  //def x64MacBuilder = new Builder(buildSteps: x64MacReleaseBuildSteps)




  // Define Build
  x64LinuxReleaseBuild = new Build(name: 'x86_64 Linux Release',
                                   type: 'Release',
                                   builder: x64LinuxReleaseBuilder,
                                   worker: x64LinuxWorker)
  x64LinuxDebugBuild = new Build(name: 'x86_64 Linux Debug',
                                    type: 'Debug',
                                    builder: x64LinuxDebugBuilder,
                                    worker: x64LinuxWorker)
  // def x64MacReleaseBuild = new Build(name: 'Mac Linux Release',
  //                                    type: 'Release',
  //                                    builder: x64MacBuilder,
  //                                    worker: x64MacWorker)

  //tasks[x64LinuxReleaseBuild.name] = build(x64LinuxReleaseBuild)
  tasks[x64LinuxDebugBuild.name] = build(x64LinuxDebugBuild)
  //tasks[x64MacReleaseBuild.name] = { x64MacReleaseBuild.build() }
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