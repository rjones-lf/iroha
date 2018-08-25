#!/usr/bin/env groovy

def doPythonWheels(os, buildType=Release) {
  def version;
  def repo;
  def envs
  if (os ==~ /(mac|linux)/) { envs = (env.PBVersion == "python2") ? "2.7.15" : "3.5.5" }
  else if (os == 'windows') { envs = (env.PBVersion == "python2") ? "py2.7" : "py3.5" }

  version = sh(script: 'git describe --tags \$(git rev-list --tags --max-count=1)', returnStdout: true).trim()
  repo = 'release'

  if (env.GIT_LOCAL_BRANCH != "master") {
    repo = 'develop'
    version += ".dev"
    if (params.nightly == true) {
      repo += "-nightly"
      version += sh(script: 'date "+%Y%m%d"', returnStdout: true).trim()
    }
    else {
      version += env.BUILD_NUMBER
    } 
  }

  sh """
    mkdir -p wheels/iroha; \
    cp build/bindings/*.{py,dll,so,pyd,lib,dll,exp,mainfest} wheels/iroha &> /dev/null || true; \
    cp .jenkinsci/python_bindings/files/setup.{py,cfg} wheels &> /dev/null || true; \
    cp .jenkinsci/python_bindings/files/__init__.py wheels/iroha/; \
    sed -i.bak 's/{{ PYPI_VERSION }}/${version}/g' wheels/setup.py; \
    modules=(\$(find wheels/iroha -type f -not -name '__init__.py' | sed 's/wheels\\/iroha\\///g' | grep '\\.py\$' | sed -e 's/\\..*\$//')); \
    for f in wheels/iroha/*.py; do for m in "\${modules[@]}"; do sed -i.bak "s/import \$m/from . import \$m/g" \$f; done; done;
  """
  if (os ==~ /(mac|linux)/) {
    sh """
      pyenv global ${envs}; \
      pip wheel --no-deps wheels/; \
      pyenv global 3.5.5; \
    """
  }
  if (os == 'windows') {
    sh """
      source activate ${envs}; \
      pip wheel --no-deps wheels/; \
      source deactivate;
    """
  }

  if (env.PBBuildType == "Release")
    withCredentials([usernamePassword(credentialsId: 'ci_nexus', passwordVariable: 'CI_NEXUS_PASSWORD', usernameVariable: 'CI_NEXUS_USERNAME')]) {
        sh "twine upload --skip-existing -u ${CI_NEXUS_USERNAME} -p ${CI_NEXUS_PASSWORD} --repository-url https://nexus.soramitsu.co.jp/repository/pypi-upload-${repo}/ *.whl"
    }
}

return this
