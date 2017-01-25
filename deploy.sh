#!/bin/bash
sudo apt-get install -yqq expect


if [ -z "$1" ]; then 
	echo "Supply first argument: docker hub tag"
fi


if [[ ! -e ~/.docker/config.json ]]; then
	expect -c '\
	set timeout 5
	spawn docker login
	expect "Username:*"
	send -- "$env(DOCKER_USER)\r"
	expect "Password:*"
	send -- "$env(DOCKER_PASS)\r"
	expect "Email:*"
	send -- "$env(DOCKER_EMAIL)\r"
	expect "*WARNING:*Login Succeeded*"
	expect eof' 1>/dev/null || exit 1
fi

docker push $1
