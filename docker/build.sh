#!/bin/bash

if [ -z ${IROHA_HOME} ]; then
  echo "[FATAL] Empty variable IROHA_HOME"
  exit 1
fi

error(){
  echo "[Error] $1"
  exit 1
}

# Print an informational script message.
# Examples:
# $ printinfo 'Quoted message'
# $ printinfo Unquoted message
printinfo() {
  # Bold, magenta message color.
  # "$*" expands to all arguments as a single string.
  echo "$(tput bold)$(tput setaf 5)[build.sh] "$*"$(tput sgr0)"
}

printinfo 'Building the Docker dev container (hyperledger/iroha-dev)'
docker build -t hyperledger/iroha-dev ${IROHA_HOME}/docker/dev
if [ ! $? ]; then
  error "can not build iroha-dev; exit code: $?"
fi

printinfo 'Build the project inside the Docker dev container'
docker run -i \
    -v ${IROHA_HOME}/docker/build:/build \
    -v ${IROHA_HOME}:/opt/iroha \
    hyperledger/iroha-dev \
    sh << COMMANDS
        # everything between COMMANDS will be executed inside a container
        cd /opt/iroha/docker/dev/scripts
        ./build-iroha.sh Release || (echo "[-] Can't build iroha" && exit 1)
        ./mktar-iroha.sh || (echo "[-] Can't make tarball" && exit 1)
        # at this step we have /tmp/iroha.tar
        (cp /tmp/iroha.tar /build/iroha.tar || (echo "[-] FAILED!" && exit 1))
COMMANDS
if [ ! $? ]; then 
  error "can not build iroha; exit code: $?"
fi

printinfo 'Building the Docker prod container (hyperledger/iroha-docker)'
docker build -t hyperledger/iroha-docker ${IROHA_HOME}/docker/build
if [ ! $? ]; then 
  error "can not build iroha; exit code: $?"
fi
