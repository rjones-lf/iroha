iroha-cluster-config-gen
=========

A role that generates `genesis.block` and keypair for each node in deployable iroha cluster based on inventory. 

Requirements
------------

`iroha-cli` binary should be accessible via `PATH` variable on your system.

Role Variables
--------------

- variables defined by this role:

    `defaults/main.yml` list of variables: 
    - `filesDir`: directory to store generated files
    
- variables required by playbook (see description in playbook's `group_vars` files):
    - `nodes_in_region`
    - `internal_port`


Example Playbook
----------------

```yaml
  - hosts: locals
    gather_facts: true
    roles:
      - { role: iroha-cluster-config-gen }
```
