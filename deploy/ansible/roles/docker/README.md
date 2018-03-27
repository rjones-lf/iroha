docker
=========

A role that installs docker engine and docker-compose including python modules.

Requirements
------------

none

Role Variables
--------------

- variables defined by this role:

    `defaults/main.yml` list of variables: 
    - `dockerVersion` - version of docker that is going to be installed
    - `dockerComposeVersion` -  version of docker compose that is going to be installed
    - `execPath` - path for `docker-compose` binary to be placed


Example Playbook
----------------

```yaml
  - hosts: locals
    gather_facts: true
    roles:
      - { role: docker, tags: docker }
```
