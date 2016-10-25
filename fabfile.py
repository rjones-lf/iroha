# -*- coding: utf-8 -*-
from config.fabric import myhosts, port, user, password, github, repo_name, deploy_hosts
from fabric.api import *
from fabric.colors import *

env.hosts = myhosts
env.port =  port
env.user =  user
env.password = password

@task
def status():
  print(magenta("+++++++++++++++++++"))
  print(magenta("+ *    *       *  +"))
  print(magenta("+  server status! +"))
  print(magenta("+  *            * +"))
  print(magenta("+++++++++++++++++++"))
  run("cmake --version")
  run("gcc --version")
  run("make --version")

  with shell_env(JAVA_HOME='/usr/lib/jvm/java-8-openjdk-amd64'):
    run("java -version")
    run("javac -version")
    run("gradle -v")

@task
def initalize_server():
  print(magenta("##########################"))
  print(magenta("# Setup server!! \(^o^)/ #"))
  print(magenta("##########################"))
  print(cyan("#  install usual"))
  sudo("apt update")
  sudo("apt -y upgrade")
  sudo("apt -y install gcc g++ make git")
  print(cyan("#  cmake"))
  sudo("apt -y install cmake")
  print(cyan("#  Java"))
  sudo("apt -y install default-jdk")
  sudo("apt -y install default-jre")
  print(cyan("# snappy"))
  sudo("apt -y install snappy")
  sudo("apt -y install libhdf5-serial-dev libleveldb-dev libsnappy-dev liblmdb-dev")
  print(cyan("# other libs"))
  sudo("apt -y install xsltproc")
  sudo("apt -y install libssl-dev")
  sudo("apt -y install unzip")
  run("curl -s https://get.sdkman.io | bash")
  run('source "/home/'+env.user+'/.sdkman/bin/sdkman-init.sh" && sdk install gradle 3.0')
  status()
  run('git config --global user.email ' + github.email)
  run('git config --global user.name  ' + github.name)
  sudo("mkdir -p /var/www")
  sudo("chown -R {}:{} /var/www".format(env.user,env.user))


def git_current_branch():
  res = local('git branch --contains', capture=True)
  return res.split(' ')[1]

@task
def check_key_github():
  with quiet():
    res = run("ssh -T git@github.com", warn_only=True)
    if not "successfully authenticated" in res:
      print(red("Githubにサーバーの公開鍵が登録されていないよ!!"))
      print(red("gitHubでssh接続する手順~公開鍵・秘密鍵の生成から~"))
      print("Ref:http://qiita.com/shizuma/items/2b2f873a0034839e47ce")
      run("ssh-keygen -f ${file_name} -t rsa -N \"\"")
      quit()

@task
def initialize_repository():
  #check_key_github()
  with cd("/var/www"):
    sudo("rm -rf *")
    run("git clone --recursive "+repo_name)
  with cd("/var/www/iroha"):
    with shell_env(JAVA_HOME='/usr/lib/jvm/java-8-openjdk-amd64'):

      with cd("core/vendor/Agrona"):
        run("./gradlew")

      with cd("core/vendor/Aeron"):
        run("./gradlew")
        run("mkdir -p cppbuild/Debug")
        with cd("cppbuild/Debug"):
          run("cmake ../..")
          run("cmake --build . --clean-first")
          run("ctest")

      with cd("core/vendor/leveldb"):
        run("make")

      with cd("core/vendor/ed25519"):
        run("make")

      with cd("core/vendor/KeccakCodePackage"):
        run(" make generic64/libkeccak.a")


      with cd("core/infra/crypto"):
        run("make")

@task
def connection_test_dev():
  run("cat welcome")
  print(git_current_branch())

@task
def connection_test_stage():
  run("cat welcome")

@task
def connection_test_production():
  run("cat welcome")

def git_push(branch = None):
  if not branch:
    branch = git_current_branch()
  local("git push origin "+branch)

def remake():
  with cd("/var/www/iroha"):
    run("mkdir -p build")
    with cd("build"):
      run("cmake ..")
      run("make")

def restart():
  # Why not work?
  #sudo('pkill -f "iroha-main"')
  pid = sudo("pgrep -f 'iroha-main'", warn_only=True)
  print(cyan(pid),"!!")
  run('kill -9 '+str(pid), warn_only=True)
  run('/var/www/iroha/build/bin/iroha-main &', pty=False)
  run('ps aux | grep iroha-main')

def curl_test():
  for host in myhosts:
    local("curl "+host)

@task
def test(branch = None):
  print(blue("#################"))
  print(blue("# test    (^o^) #"))
  print(blue("#################"))
  # connection_test_dev()
  if not branch:
    branch = git_current_branch()

    branch = branch.strip()
    with cd("/var/www/iroha"):
      res = run("git reset --hard")
      res = run("git checkout -b "+branch+" origin/"+branch, warn_only=True)
      if res.failed:
        run("git checkout "+branch)
      run("git pull origin "+branch+" --no-ff")

      remake()

  git_push(branch)

  with cd("/var/www/iroha"):
    res = run("git reset --hard")
    res = run("git checkout -b "+branch+" origin/"+branch, warn_only=True)
    if res.failed:
      run("git checkout "+branch)
    run("git pull origin "+branch+" --no-ff")

    remake()
    restart()

  curl_test()

@task
def deploy_stage_with_circle_ci():
  print(yellow("#################"))
  print(yellow("# Staging !!!!  #"))
  print(yellow("#################"))
  env.hosts = [deploy_hosts]
  env.port = '1225'
  env.user = 'deploy'
  connection_test_stage()

@task
def deploy_with_circle_ci():
  print(red("#################"))
  print(red("# Production !! #"))
  print(red("#################"))
  env.hosts = [deploy_hosts]
  env.port = '1225'
  env.user = 'deploy'
  connection_test_production()
