[![Build Status](https://travis-ci.org/singleplatform-eng/ansible-users.svg?branch=master)](https://travis-ci.org/singleplatform-eng/ansible-users)

# ansible-users

Role to manage users on a system.

## Role configuration

* users_create_per_user_group (default: true) - when creating users, also
  create a group with the same username and make that the user's primary
  group.
* users_group (default: users) - if users_create_per_user_group is _not_ set,
  then this is the primary group for all created users.
* users_default_shell (default: /bin/bash) - the default shell if none is
  specified for the user.
* users_create_homedirs (default: true) - create home directories for new
  users. Set this to false is you manage home directories separately.

## Creating users

Add a users variable containing the list of users to add. A good place to put
this is in `group_vars/all` or `group_vars/groupname` if you only want the
users to be on certain machines.

The following attributes are required for each user:

* username - The user's username.
* name - The full name of the user (gecos field)
* home - the home directory of the user to create (optional, defaults to /home/username)
* uid - The numeric user id for the user (optional). This is required for uid consistency
  across systems.
* gid - The numeric group id for the group (optional). Otherwise, the
  uid will be used
* password - If a hash is provided then that will be used, but otherwise the
  account will be locked
* update_password - This can be either 'always' or 'on_create'
  - 'always' will update passwords if they differ. (default)
  - 'on_create' will only set the password for newly created users.
* group - optional primary group override
* groups - a list of supplementary groups for the user.
* profile - a string block for setting custom shell profiles
* ssh_key - This should be a list of ssh keys for the user (optional). Each ssh key
  should be included directly and should have no newlines.
* generate_ssh_key - Whether to generate a SSH key for the user (optional, defaults to no).

In addition, the following items are optional for each user:

* shell - The user's shell. This defaults to /bin/bash. The default is
  configurable using the users_default_shell variable if you want to give all
  users the same shell, but it is different than /bin/bash.

Example:

    ---
    users:
      - username: foo
        name: Foo Barrington
        groups: ['wheel','systemd-journal']
        uid: 1001
        home: /local/home/foo
        profile: |
          alias ll='ls -lah'
        ssh_key:
          - "ssh-rsa AAAAA.... foo@machine"
          - "ssh-rsa AAAAB.... foo2@machine"
    groups_to_create:
      - name: developers
        gid: 10000
    users_deleted:
      - username: bar
        name: Bar User
        uid: 1002

## Deleting users

The `users_deleted` variable contains a list of users who should no longer be
in the system, and these will be removed on the next ansible run. The format
is the same as for users to add, but the only required field is `username`.
However, it is recommended that you also keep the `uid` field for reference so
that numeric user ids are not accidentally reused.

You can optionally choose to remove the user's home directory and mail spool with
the `remove` parameter, and force removal of files with the `force` parameter.

    users_deleted:
      - username: bar
        uid: 1002
        remove: yes
        force: yes
