# Using ansible for iroha deployment

## 1. Overview 

There are 2 deployment scenarios supported which are implemented in the single ansible playbook -`iroha-nodes` (path `./deploy/ansible/playbooks/iroha-nodes`)

### 1.1 Steps to launch

Install ansible according to [official guide](https://docs.ansible.com/ansible/latest/installation_guide/intro_installation.html)

1. Fill inventory file (`deploy/ansible/inventory/inv.list`)
2. Type in your terminal 

```bash
cd deploy/ansible
ansible-playbook -i inventory/inv.list playbooks/iroha-nodes/playbook.yml --private-key=<path to your private key>
```

### 1.2 How it works in general


| Order | Phase name | Description |
| :--------: | -------- | -------- |
| 1 | Generation phase | Generate list of peers (`peers.csv`) with keypairs (`node{X}.pub`, `node{X}.priv`) to `playbooks/iroha-nodes/files/` directory |
| 2 | Configuration phase | Generate `genesis.block`, `config.docker` files to `playbooks/iroha-nodes/files/` directory |
| 3 | Deployment phase | First, Ansible generates `docker-compose.yml` file for each node and put it to the `{{ docker_dir }}`. Then, config files are delivered to the endpoint ( to `{{ iroha_conf }}` directory) and iroha is launched using `docker-compose` command |

Current playbook uses 3 roles:
- `docker` (`deploy/ansible/roles/docker/README.md`)
- `iroha-config-gen` (`deploy/ansible/roles/iroha-config-gen/README.md`)
- `iroha-nodes` (`deploy/ansible/roles/iroha-nodes/README.md`)

By configuring environment variables one can manage `iroha` nodes deployment over multiple nodes. The description of possible scenarios is provided below in the following sections.


### 1.3 Variables description

| Variable Name | Description | Default Value |
| :--------: | :--------: | :--------: |
| `pg_name` |postgres docker container name|`opt_postgres` |
|`pg_port`|postgres docker container port|`5432`|
|`pg_user`|postgres user to connect to DB|`psql`|
|`pg_pass`|user password to connect to DB|`psql`|
|`iroha_net_name`|name of docker network|`iroha_network`|
|`iroha_conf_docker`| path to folder with config files inside docker container|`/opt/iroha_data`|
|`iroha_conf`|config files directory on target host|`/opt/docker/iroha/conf`|
|`docker_dir`|directory to store configuration files and data |`/opt`|
|`iroha_image_name`|iroha docker image name|`hyperledger/iroha`|
|`iroha_image_tag`|iroha docker image tag|`latest`|
|`pg_image_name`|postgres docker image name|`postgres`|
|`pg_image_tag`|postgres docker image tag|`9.5`|
|`torii_port`|torii port start value|`50051`|
|`internal_port`|iroha port start value|`10001`|
|`nodes_in_region`|amount of iroha peers per node|3|

## 2. Multi-peers nodes 
### 2.1 Main ideas
This scenario allows to deploy multiple iroha peers per each node. 

For example, you want to run 21 iroha peers, but you have only 4 nodes (bare-metal servers, VMs, etc) that are in the same network (LAN/WAN). Then, for instance, you might choose this type of distribution: 
- 8 `iroha` nodes should be launched on the 1st host (or you can vary amount of `iroha` nodes per each host). 
- 5 `iroha` nodes on the 2nd host
- 4 `iroha` nodes on the 3rd host
- 4 `iroha` nodes on the 4th host

**Limitations**
> Amount of iroha peers is >0, but <30 (*due to docker networks max amount per host*)

**Important notes**
> NOTE: During the *Deployment phase* one can see the error messages during execution of task `stop and remove all docker-compose containers before operations`. 
> That means that you don't have launched `iroha` and `postgres` containers. This error is handled and will not affect playbook execution.

Let's discuss how it works in details.


### 2.2 Inventory 
 To continue with previous example, the `inventory/inv.list` has to be filled with the following data:
```
[iroha-east]
iroha-bench1 ansible_host=<node1 IP> ansible_user=<user> key=0

[iroha-west]
iroha-bench2 ansible_host=<node2 IP> ansible_user=<user> key=8

[iroha-south]
iroha-bench3 ansible_host=<node3 IP> ansible_user=<user> key=13

[iroha-north]
iroha-bench4 ansible_host=<node4 IP> ansible_user=<user> key=17
```

As you can see, basic host field in group contains `hostname`, `ansible_host <ip>`, `ansible_user`, and `key` field. 

`key` is a peer ID in a iroha network. This value is used for passing only node-specific keypair and configuration files to the `iroha` node to start.

In this particular playbook this value is used to the start of the count. 
`nodes_in_region` is an amount of `iroha` nodes running on each host. 
Values `key` and `nodes_in_region` are used in the following manner:
- for host `iroha-bench1` we have 8 iroha peers. First peer ID will be `key=0`, for second `key=1`, and so on up to `key=7`
- for host `iroha-bench2` we have another 5 iroha peers. Their IDs will start from 8 to 12.
- for host `iroha-bench3` we want to run 4 iroha peers. Their IDs will start from 13 to 16.
- for host `iroha-bench4` we want to run 4 iroha peers. Their IDs will start from 17 to 20.

> `nodes_in_region` variable could be set at `playbooks/iroha-nodes/group_vars/<group_name>.yml`, or default value from `playbooks/iroha-nodes/group_vars/all.yml` will be used. 


## 3. One peer per node 
### 3.1 Main ideas

This scenario is a particular case of the previous mode. If you set `nodes_in_region: 1`, you'll get only one iroha peer per each node. 

### 3.2 Inventory 
In this particular case the inventory file (`inventory/inv.list`) looks like below:
```
[iroha-nodes]
iroha-bench1 ansible_host=<node1 IP> ansible_user=<user> key=0
iroha-bench2 ansible_host=<node1 IP> ansible_user=<user> key=1
iroha-bench3 ansible_host=<node1 IP> ansible_user=<user> key=2
iroha-bench4 ansible_host=<node1 IP> ansible_user=<user> key=3
```

As you can see, basic host field in group contains `hostname`, `ansible_host <ip>`, `ansible_user`, and `key` field. 

`key` is a node ID in a iroha network. 
Values `key` is used in the following manner:
- for host iroha-1 host peer ID will be `key=0`
- for host iroha-2 host peer ID will be `key=1` 
- the following hosts in the list should increment this value

> you can use multiple group of hosts with different names. The only requirement is that value `key`
should be increased throughout the all list on hosts.  
