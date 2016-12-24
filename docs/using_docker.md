# How to use docker and docker-compose for `iroha` development

-----
Advantages:

 - usage of docker-compose allows folder `iroha` to be mounted into container. This allows you to change code in host machine, at the same time changing it inside container(s)
 - very easy to start development without dependencies conflicts

***Note***: permissions and ownership will be transfered 1-to-1 from host to container and backwards. It means, that file owned by `user:user` (**id=1000**) will be owned inside container by some other user (probably the same) **with id=1000**.

It means, that file created inside container by `root` will be owned by `root` on the host! 

-----

First of all, you should clone `iroha` repository with all submodules to your host machine:

```bash
git clone --recursive https://github.com/hyperledger/iroha
cd iroha
```

Then, build `iroha-dev` image using [latest docker-compose](https://github.com/docker/compose/releases) [^1]:
```bash
docker-compose build
```

Run container:
```bash
docker-compose run iroha-dev bash
```

You got shell access into container with all dependencies for `iroha`. To build `iroha`, go to `$IROHA_HOME` folder and run `build.sh`[^2]:
```bash
cd $IROHA_HOME
./build.sh
```

If you want to remove container for some reason, use [^1]:
```bash
docker-compose down
```


[^1]: You must be in the same folder with `docker-compose.yml` file.
[^2]: `build.sh` must be executed from `$IROHA_HOME` direcory inside docker container.

# How to use docker and docker-compose for production

***TBD***
