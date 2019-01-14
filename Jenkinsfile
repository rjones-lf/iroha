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
        echo "Worker: ${env.NODE_NAME}"
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

// sanitise the string it should contain only 'key1=value1;key2=value2;...'
def cmd_sanitize(String cmd){
  if (cmd.contains("//"))
    return false

  for (i in cmd.split(";")){
    if (i.split("=").size() != 2 )
       return false
    if (i.split("=").any { it.trim().contains(" ")})
      return false
  }
  return true
}

stage('Prepare environment'){
timestamps(){

param_descriptions = """Default - will automatically chose the correct one based on branch name and build number
Branch commit - Linux/gcc v5;	Test: Smoke, Unit;
On open pr -  Linux/gcc v5, MacOS/appleclang; Test: Smoke, Unit; Coverage; Analysis: cppcheck, sonar;
Commit in Open PR - Same as Branch commit
Before merge to trunk - Linux/gcc v5 v7, Linux/clang v6 v7, MacOS/appleclang; Test: *; Coverage; Analysis: cppcheck, sonar; Build type: Debug when Release
Before merge develop - Not implemented
Before merge master - Not implemented
Nightly build - Not implemented
Custom command - enter command below
"""

properties([
    parameters([
        choice(choices: 'Default\nBranch commit\nOn open pr\nCommit in Open PR\nBefore merge to trunk\nBefore merge develop\nBefore merge master\nNightly build\nCustom command', description: param_descriptions, name: 'target'),
        string(defaultValue: '', description: '', name: 'custom_cmd', trim: true)
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

  //All variable and Default values
  x64linux_compiler= true
  x64linux_compiler_list = ['gcc5']
  mac_compiler = false
  mac_compiler_list = []

  testing = true
  testList = '(module)'

  sanitize = false
  cppcheck = false
  fuzzing = false // x64linux_compiler_list= ['clang6']  testing = true testList = "(None)"
  sonar = false
  coverage = false
  doxygen = false

  build_type = 'Debug'
  packageBuild = false
  pushDockerTag = 'not-supposed-to-be-pushed'
  packagePush = false
  specialBranch = false

  if (build_type == 'Release') {
    packageBuild = true
    testing = false
    if (scmVars.GIT_LOCAL_BRANCH ==~ /(master|develop|dev)/){
      packagePush = true
    }
  }

  if (scmVars.GIT_LOCAL_BRANCH ==~ /(develop|dev)/)
    pushDockerTag =  'develop'
  else if (scmVars.GIT_LOCAL_BRANCH == 'master')
    pushDockerTag = 'latest'
  else
    pushDockerTag = 'not-supposed-to-be-pushed'

  if (scmVars.GIT_LOCAL_BRANCH in ["master","develop","dev"] || scmVars.CHANGE_BRANCH_LOCAL in ["develop","dev"])
    specialBranch =  true
  else
    specialBranch = false

  if (params.target == 'Default')
    if ( scmVars.GIT_BRANCH.startsWith('PR-'))
      if (BUILD_NUMBER == '1')
        target='On open pr'
      else
        target='Commit in Open PR'
    else
      target='Branch commit'
  else
    target = params.target


  print("Selected Target '${target}'")
  switch(target) {
     case 'Branch commit':
        echo "All Default"
        break;
     case 'On Open PR':
        mac_compiler = true
        mac_compiler_list = ['appleclang']
        coverage = true
        cppcheck = true
        sonar = true
        break;
     case 'Commit in Open PR':
        echo "All Default"
        break;
     case 'Before merge to trunk':
        println("The value target=${target} is not implemented");
        sh "exit 1"
        break;
     case 'Custom command':
        if (cmd_sanitize(params.custom_cmd)){
          evaluate (params.custom_cmd)
        } else {
           println("Unable to parse '${params.custom_cmd}'")
           sh "exit 1"
        }
        break;
     default:
        println("The value target=${target} is not implemented");
        sh "exit 1"
        break;
  }

  echo "packageBuild=${packageBuild}, pushDockerTag=${pushDockerTag}, packagePush=${packagePush} "
  echo "testing=${testing}, testList=${testList}"
  echo "x64linux_compiler=${x64linux_compiler}, x64linux_compiler_list=${x64linux_compiler_list}"
  echo "mac_compiler=${mac_compiler}, mac_compiler_list=${mac_compiler_list}"
  echo "specialBranch=${specialBranch}"
  echo "sanitize=${sanitize}, cppcheck=${cppcheck}, fuzzing=${fuzzing}, sonar=${sonar}, coverage=${coverage}, doxygen=${doxygen}"
  print scmVars
  print environmentList


  // Load Scripts
  def x64LinuxBuildScript = load '.jenkinsci/builders/x64-linux-build-steps.groovy'
  def x64BuildScript = load '.jenkinsci/builders/x64-mac-build-steps.groovy'


  // Define Workers
  x64LinuxWorker = new Worker(label: 'x86_64', cpusAvailable: 4)
  x64MacWorker = new Worker(label: 'mac', cpusAvailable: 4)


  // Define all possible steps
  def x64LinuxBuildSteps
  def x64LinuxPostSteps = new Builder.PostSteps()
  if(x64linux_compiler){
    x64LinuxBuildSteps = [{x64LinuxBuildScript.buildSteps(
      x64LinuxWorker.cpusAvailable, x64linux_compiler_list, build_type, specialBranch, coverage,
      testing, testList, cppcheck, sonar, doxygen, packageBuild, packagePush, sanitize, fuzzing, environmentList)}]
    x64LinuxPostSteps = new Builder.PostSteps(
      always: [{x64LinuxBuildScript.alwaysPostSteps(environmentList)}],
      success: [{x64LinuxBuildScript.successPostSteps(scmVars, build_type, packagePush, pushDockerTag, environmentList)}])
  }
  def x64MacBuildSteps
  def x64MacBuildPostSteps = new Builder.PostSteps()
  if(mac_compiler){
    x64MacBuildSteps = [{x64BuildScript.buildSteps(x64MacWorker.cpusAvailable, mac_compiler_list, build_type, coverage, testing, testList, packageBuild,  environmentList)}]
    x64MacBuildPostSteps = new Builder.PostSteps(
      always: [{x64BuildScript.alwaysPostSteps(environmentList)}],
      success: [{x64BuildScript.successPostSteps(scmVars, build_type, packagePush, environmentList)}])
  }

  // Define builders
  x64LinuxBuilder = new Builder(buildSteps: x64LinuxBuildSteps, postSteps: x64LinuxPostSteps)
  x64MacBuilder = new Builder(buildSteps: x64MacBuildSteps, postSteps: x64MacBuildPostSteps )

  // Define Build
  x64LinuxBuild = new Build(name: "x86_64 Linux ${build_type}",
                                    type: build_type,
                                    builder: x64LinuxBuilder,
                                    worker: x64LinuxWorker)
  x64MacBuild = new Build(name: "Mac ${build_type}",
                                     type: build_type,
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