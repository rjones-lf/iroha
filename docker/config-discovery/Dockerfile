#-----------------------------------------------------------------------
# configdiscovery - Config discovery service for IROHA
#
# build : docker build -t warchantua/configdiscovery .
#
# Copyright (c) 2016 Soramitsu,Co.,Ltd.
# All Rights Reserved.
#-----------------------------------------------------------------------
FROM python:3

MAINTAINER Bogdan Vaneev <bogdan@soramitsu.co.jp>

LABEL BUILD "docker build -t warchantua/configdiscovery ."

# 4 is the number of nodes
LABEL RUN "docker run --name configdiscovery warchantua/configdiscovery 4" 


COPY ["config-server.py", "/server.py"]

EXPOSE 8000

ENTRYPOINT ["python3", "/server.py"]

CMD ["4"]
