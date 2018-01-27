FROM ubuntu:16.04

RUN apt-get update;\
    apt-get upgrade -y; \
    apt-get install -y \
    libc-ares-dev

#Install iroha
COPY iroha.deb /tmp/iroha.deb
RUN dpkg -i /tmp/iroha.deb; rm -f /tmp/iroha.deb

WORKDIR /opt/iroha_data

COPY entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["/sbin/init"]