iroha-standalone-deploy-node
=========

A role that runs iroha cluster by delivering previously generated `genesis.block`, 
keypair for each node, and newly generated `config.sample` to target hosts. 
It runs `iroha` and `postgres:9.5` in docker containers.

Requirements
------------

1. Pre-generated files (role `iroha-standalone-config-gen`):
    - `genesis.block`
    - `nodeX.pub`, `nodeX.priv` keypair for each node
stored at `filesDir` folder.

2. `docker` engine installed on target host (role `docker`)

Role Variables
--------------

- variables defined by this role:

    `defaults/main.yml` list of variables: 
  - `postgresName`: name of `postgres` docker container after running by `docker-compose` 
  - `postgresUser`: username on postgresql
  - `postgresPassword`: password on postgresql
  - `iroha_net`: name of docker network
  - `containerConfPath`: config dir prefix on target host (this directory is attached to running iroha docker container as a docker volume)

- variables required by playbook (see description in playbook's `group_vars` and `host_vars` files):
    - `filesDir`
    - `composeDir`
    - `confPath`
    - `internal_port`
    - `torii_port`


Example Playbook
----------------

```yaml
  - hosts: iroha-nodes
    gather_facts: true
    roles:
      - { role: iroha-standalone-deploy-node }
```
