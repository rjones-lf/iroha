/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

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
#include "transaction_validator.hpp"

namespace transaction_validator {

    template<typename T>
    using Transaction = transaction::Transaction<T>;
    template<typename T>
    using ConsensusEvent = event::ConsensusEvent<T>;
    template<typename T>
    using Add = command::Add<T>;
    template<typename T>
    using Transfer = command::Transfer<T>;

    template<>
    bool isValid<ConsensusEvent<Transaction<Add<object::Asset>>>>(
        const std::unique_ptr<ConsensusEvent<Transaction<Add<object::Asset>>>>& tx
    ){
        // Write domain logic
        return true;
    }


    template<>
    bool isValid<event::Event>(
        const std::unique_ptr<event::Event>& tx
    ){
        // Write domain logic
        return true;
    }


};  