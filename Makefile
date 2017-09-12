# Copyright 2017 Soramitsu Co., Ltd. All Rights Reserved.
#
#---------------------------------------------------------------
# This makefile defines the following targets
#
# - all (default) - buid iroha-build container, and build iroha
# - docker        - build iroha runtime container
# - docker-test   - build iroha runtime and test container
# - up            - running iroha container by docker-compose
# - down          - stop and remove iroha container by docker-compose
# - test-up       - running iroha-test container buy docker-compose
# - test-down     - stop and remove iroha-test container by docker-compose
# - test          - exec all test commands
# - iroha-develop - build iroha-docker-develop  container
# - iroha-release - build iroha by iroha-docker-develop
# - iroha         - build iroha runtime container
# - iroha-up      - running iroha container by docker-compose
# - iroha-down    - stop and remove iroha container by docker-compose
# - iroha-test    - exec all test commands
# - clean         - cleaning protobuf schemas and build directory
#---------------------------------------------------------------
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

.PHONY: all docker docker-test up test-up down test-down test

IROHA_HOME := $(shell pwd)

all: iroha-develop iroha-build iroha-release iroha

docker: iroha-release iroha

docker-test: iroha-release-test iroha-test

up: iroha-up

test-up: iroha-test-up

down: iroha-down

test-down: iroha-test-down

iroha-develop:
	cd docker/develop && docker build --rm  -t hyperledger/iroha-docker-develop .

iroha-build:
	docker run -t --rm --name iroha -v $(IROHA_HOME):/usr/local/iroha -w /usr/local/iroha hyperledger/iroha-docker-develop /usr/local/iroha/scripts/iroha-build.sh

iroha-release:
	docker run -t --rm --name iroha-release -v $(IROHA_HOME):/usr/local/iroha -v $(IROHA_HOME)/build/ccache-data:/tmp/ccache -w /usr/local/iroha hyperledger/iroha-docker-develop /usr/local/iroha/scripts/iroha-release.sh

iroha-release-test:
	docker run -t --rm --name iroha-release-test -v $(IROHA_HOME):/usr/local/iroha -v $(IROHA_HOME)/build/ccache-data:/tmp/ccache -w /usr/local/iroha hyperledger/iroha-docker-develop /usr/local/iroha/scripts/iroha-release-test.sh


iroha:
	cd docker/release && docker build --rm -t hyperledger/iroha .

iroha-test:
	cd docker/release && docker build --rm -t hyperledger/iroha-test -f Dockerfile.test .

iroha-up:
	cd docker && env COMPOSE_PROJECT_NAME=iroha docker-compose -p iroha up -d

iroha-down:
	cd docker && docker-compose -p iroha down

iroha-test-up:
	cd docker && env COMPOSE_PROJECT_NAME=iroha docker-compose -p iroha -f docker-compose-test.yml up -d

iroha-test-down:
	cd docker && env COMPOSE_PROJECT_NAME=iroha docker-compose -p iroha -f docker-compose-test.yml down

test:
	bash scripts/iroha-test.sh

clean:
	rm schema/*.{cc,h}
	rm -fr build
