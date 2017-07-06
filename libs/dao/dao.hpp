/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "block.hpp"
#include "proposal.hpp"
#include "transaction.hpp"
#include "transaction_response.hpp"
#include "asset.hpp"
#include "account.hpp"
#include "wallet.hpp"
#include "peer.hpp"
#include "domain.hpp"
#include "client.hpp"
#include "peer.hpp"
#include "dao_crypto_provider.hpp"
#include "dao_hash_provider.hpp"

// commands
#include "command.hpp"
#include <dao/commands/add_peer.hpp>

// query
#include "query.hpp"
#include "query_response.hpp"

// blocks
#include <dao/queries/get_blocks.hpp>
#include <dao/queries/responses/blocks_response.hpp>

// answer error
#include <dao/queries/responses/error_response.hpp>


/**
 * DAO - Data Access Object.
 * DAO module provides objects that are useful for all other modules in system.
 * DAO objects do not depend on transport, such as protobuf.
 * DAO objects in general are structures with public immutable fileds.
*/