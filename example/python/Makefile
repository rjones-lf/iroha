#!/usr/bin/env make

# generate proto files in current dir
PROTO_PATH = ../../shared_model/schema
PROTO_FILES = $(wildcard $(PROTO_PATH)/*.proto)
PYTHON_PROTO_LIBS_PATH = .
PYTHON_PROTO_LIBS_FILES = $(patsubst $(PROTO_PATH)/%.proto,$(PYTHON_PROTO_LIBS_PATH)/%_pb2.py,$(PROTO_FILES))

.PHONY: default
default: all

.PHONY: all
all: $(PYTHON_PROTO_LIBS_FILES)

$(PYTHON_PROTO_LIBS_PATH)/%_pb2.py: $(PROTO_PATH)/%.proto
	protoc --proto_path=$(PROTO_PATH) --python_out=$(PYTHON_PROTO_LIBS_PATH) $<
