/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/gate_object.hpp"

using GateObject = iroha::consensus::GateObject;

template GateObject::~variant();
template GateObject::variant(GateObject &&);
template GateObject::variant(const GateObject &);
template void GateObject::destroy_content();
template int GateObject::which() const;
template void GateObject::indicate_which(int);
template bool GateObject::using_backup() const;
template GateObject::convert_copy_into::convert_copy_into(void *);
