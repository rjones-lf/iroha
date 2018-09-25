# Description
This directory contains Docker files for building different flavors of Docker images:
- *develop* contains Dockerfile with all the dependencies required for compiling Iroha source code. We use it in development process, thus all depencencies are kept up-to-date
- *release* contains Dockerifle with all the dependencies required for running Iroha binaries compiled with *Release* flag. Each time commit is pushed into `develop` or `master` branch, automated routine is executed to build Docker image containing Iroha binaries. It is then pushed to [Dockerhub](https://hub.docker.com/r/hyperledger/iroha/)
- *android* contains Dockerifle with all the dependencies required for compiling bindings for Android platform.

  **NOTE**: Python and Java bindings can be compiled using Dockerfile from *develop* directory
