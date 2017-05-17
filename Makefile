# Copyright 2017 Soramitsu Co., Ltd. All Rights Reserved.
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

.PHONY: all pwd build scripts

IROHA_HOME := $(if $(IROHA_HOME),$(IROHA_HOME),$(shell pwd))

DUMMY = .dummy

all: pwd build scripts iroha-dev iroha-build iroha

pwd:
	@echo "IROHA_HOME=$(IROHA_HOME)"

build:
	mkdir -p $@

scripts:
	rsync -av scripts build

iroha-dev:
	docker build -t hyperledger/iroha-dev docker/dev

iroha-build:
	docker run -t --rm --name iroha-build \
	  -v $(IROHA_HOME):/opt/iroha \
	  hyperledger/iroha-dev /opt/iroha/build/scripts/iroha-build.sh

iroha:
	rm -fr docker/tiny/iroha
	rsync -av ${IROHA_HOME}/build/iroha docker/tiny
	docker build --rm -t hyperledger/iroha docker/tiny

clean:
	rm -fr build external docker/tiny/iroha

run:
	docker run -d --name iroha -p 50051:50051 hyperledger/iroha

logs:
	docker logs -f iroha

stop:
	docker stop iroha
	docker rm iroha
