iroha-cluster-deploy-node
=========

A role that runs iroha cluster by delivering previously generated `genesis.block`, 
keypair for each node, and newly generated `config.sample` to target hosts. It creates `docker-compose.yml` file
with iroha docker and `postgres:9.5` configured in separated docker networks, but in one P2P network.

This role allows you to run X nodes of iroha on each target host, where X [0,1,..,31], e.g:

- target host1: node-0, node-1, node-2, node-3, node-4
- target host2: node-5, node-6, node-7
- target host3: node-8, node-9, node-10, node-11, node-12, node-13

Requirements
------------

1. Pre-generated files (role `iroha-standalone-config-gen`):
    - `genesis.block`
    - `nodeX.pub`, `nodeX.priv` keypair for each node
stored at `filesDir` folder.

2. `docker-compose` binary available via `PATH` variable (role `docker`)

Role Variables
--------------

- variables defined by this role:

    `defaults/main.yml` list of variables: 
  - `postgresName`: name of `postgres` docker container after running by `docker-compose` 
  - `postgresUser`: username on postgresql
  - `postgresPassword`: password on postgresql
  - `iroha_net`: name prefix of docker network
  - `containerConfPath`: config dir prefix on target host (this directory is attached to running iroha docker container as a docker volume)

- variables required by playbook (see description in playbook's `group_vars` files):
    - `filesDir`
    - `composeDir`
    - `confPath`
    - `nodes_in_region`
    - `internal_port`
    - `torii_port`


Example Playbook
----------------

```yaml
  - hosts:
    - iroha-east
    - iroha-west
    gather_facts: False
    pre_tasks:
    - name: install python 2
      raw: test -e /usr/bin/python || (apt -y update && apt install -y python)
      changed_when: False
    roles:
      - { role: iroha-cluster-deploy-node, tags: ["deliver", "deploy"] }
```
tags can be used for separaring the execution, e.g. if you exclude tag "deploy", command
`docker-compose up -d` will not be executed and your iroha cluster will not start, but all files will be deployed.
