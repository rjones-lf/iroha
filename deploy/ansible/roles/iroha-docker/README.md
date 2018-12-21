## Description
This role deploys multiple replicas of Iroha containers (one Iroha peer per container) on remote hosts. Each Iroha peer can communicate with others in two ways:
  - using public IP addresses or hostnames set in inventory list OR
  - using private IP addresses of the Docker overlay network

The first one is easier to implement since it does not require preliminary configuration of the remote hosts. Just make sure that network ports are not firewalled. You can check the port list in the generated Docker Compose file (`docker-compose.yml`) after deployment.

The second one can be used when there is an overlay network exists between the hosts. In short, overlay network allows for Docker containers to communicate using a single subnet. Such that each container would have a unique IP address in that subnet. Learn more in official Docker documentation (https://docs.docker.com/network/overlay). We recommend to use Calico for setting up Docker overlay network since it can be used as a network plugin (https://docs.projectcalico.org/v1.5/getting-started/docker/tutorials/basic).

The second way is also suitable for local-only deployments and enabled by default.

## Requirements
  Tested on Ubuntu 16.04, 18.04
  - Local:
    - python-3, python3-dev
    - PIP modules: future, ansible(>=2.4), sha3(for python<3.6)
  - Remote:
    - Docker (>=17.12)
    - PIP modules: docker, docker-compose
    There is a role for setting up a remote part of the dependencies named `docker`.

## Quick Start
`ansible-playbook -c local playbooks/iroha-docker/main.yml -b`

This will deploy 6 Iroha Docker containers along with 6 Postgres containers on a localhost. Torii port of each container is exposed to the local host. Iroha peer can be communicated over port defined in `iroha_torii_port` variable (50051 by default). Overall, each host will listen the following port range: `iroha_torii_port` + *number-of-containers*.

### Note:
> This command escalates privileges during the run. It is required to be able to spin up Docker containers. We recommend to run the playbook as a passwordless sudo user. You may also try to execute `sudo su` beforehand to temporarily grant passwordless sudo access to a regular user.

## Initial configuration

See `defaults/main.yml` file to get more details about available configuration options.

## Examples
### Example 1
<!-- TODO: Cover more example cases -->
Deploying 6 Iroha peers on two remote hosts communicating using public IP addresses. With 2 and 4 replicas on each host respectively.

1. Create inventory list containing IP address (or hostnames if they are mutually resolve-able on both hosts) of two hosts that will run Iroha peers

    **iroha.list**
    ```
    [all]
    192.168.122.109
    192.168.122.30
    ```

    Put this file into `../inventory/` directory.
2. Make sure you can SSH with a root account into either of these hosts using a private key.

    **Note**
    > You can also SSH with the user other than root. Make sure it can execute `sudo` without prompting a password. Set `-u` option for `ansible-playbook` command.

3. Create two YAML files in `../playbooks/iroha-docker/host_vars` directory:

    **192.168.122.109.yml**
    ```
    replicas: 2
    ```

    **192.168.122.30.yml**
    ```
    replicas: 4
    ```

4. Create a playbook:

    Replace the contents of `../playbooks/iroha-docker/main.yml` with this block:
    **main.yml**
    ```
    - hosts: all
      strategy: linear
      roles:
        # docker role only works for Linux hosts
        - { role: docker, tags: docker }
        - { role: iroha-docker, tags: iroha-docker }
    ```
5. Run the playbook
```
ansible-playbook -i inventory/iroha.list -b playbooks/iroha-docker/main.yml
```

### Example 2
Deploying 6 Iroha peers on two remote hosts communicating over overlay network (Calico) using custom hostnames.

**TBD**