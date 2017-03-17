FROM ubuntu:16.04

COPY iroha /iroha

ENV IROHA_HOME=/iroha
ENV LD_LIBRARY_PATH=/iroha/lib

WORKDIR /iroha

RUN useradd -ms /bin/bash iroha; \
	chown -R iroha:iroha $IROHA_HOME


USER iroha

CMD /iroha/scripts/run.sh
