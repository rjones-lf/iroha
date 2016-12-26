# How to build `iroha`

Run build script and wait for completion. 
```
${IROHA_HOME}/docker/build.sh
``` 

How it works:
 1. pulls (or builds if can't pull) `iroha-dev` image. 
 2. creates `build/iroha.tar`, which consists of iroha binaries and libraries.  
 3. builds new docker image `warchantua/iroha`, which is production-ready container.


If you can't execute `build.sh` for some reason, just run docker commands from it.


Good luck!
