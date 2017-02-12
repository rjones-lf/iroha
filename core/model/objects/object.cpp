/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>
#include "../../util/logger.hpp"

#include "account.hpp"
#include "simple_asset.hpp"
#include "asset.hpp"
#include "domain.hpp"
#include "message.hpp"

#include "object.hpp"
//#include "peer.hpp"


namespace object {

    // There is kind of Currency, Asset,Domain,Account,Message and Peer. Associate SmartContract with Asset.
    Object::Object() :
			type(ObjectValueT::null)
		{};	// ctor for ValueT::null

    Object::Object(const SimpleAsset& rhs):
			type(ObjectValueT::simpleAsset)
		{
        simpleAsset = new SimpleAsset(rhs);
    }

    Object::Object(const Asset& rhs):
			type(ObjectValueT::asset)
		{
        asset = new Asset(&rhs);
    }

    Object::Object(const Domain& rhs):
			type(ObjectValueT::domain)
		{
      	domain = new Domain(rhs);
    }

    Object::Object(const Account& rhs):
			type(ObjectValueT::account)
		{
      account = new Account(rhs);
    }

    Object::Object(const Message& rhs):
			type(ObjectValueT::message)
		{
        message = new Message(rhs);
    }
    Object::Object(const Peer& rhs):
			type(ObjectValueT::peer)
		{
        peer = new Peer(rhs);
    }

		SimpleAsset* Object::AsSimpleAsset(){
			return type == ObjectValueT::simpleAsset?
				simpleAsset : nullptr;
		}
		Asset*       Object::AsAsset(){
			return type == ObjectValueT::asset?
				asset : nullptr;
		}
		Domain*      Object::AsDomain(){
			return type == ObjectValueT::domain?
				domain : nullptr;
		}
		Account*     Object::AsAccount(){
			return type == ObjectValueT::account?
				account : nullptr;
		}
		Message*     Object::AsMessage(){
			return type == ObjectValueT::message?
				message : nullptr;
		}
		Peer*        Object::AsPeer(){
			return type == ObjectValueT::peer?
				peer : nullptr;
		}

}
