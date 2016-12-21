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
#include <gtest/gtest.h>

#include <iostream>
#include <vector>
#include <tuple>

#include "../../../core/crypto/base64.hpp"

#include "../../../core/crypto/signature.hpp"
#include "../../../core/infra/protobuf/convertor.hpp"
#include "../../../core/consensus/consensus_event.hpp"
#include "../../../core/repository/consensus/transaction_repository.hpp"
#include "../../../core/repository/world_state_repository.hpp"

using namespace command;
using namespace transaction;
using namespace event;
using namespace object;

auto publicKey1 = "xrXCjvMwFf3LGMZ6w//rB6JHtvL0iQpiwaiVYWMIE9k=";
auto privateKey1 = "sMrFd1JzbfJJ4LOffj6R6mVw2nTadzjYgs2L+KCPe3oVgO558ZiDMP38xXWUNstTgV7M1rXAuceiTFiPaoxjPA==";
auto publicKey2 = "FZ7KQcL88eBkhMk41jNmQeKBBT0+jvpCMFXKowtDf8o=";
auto privateKey2 = "0PpZ8x/hT5uUQ6y2e/Mq3Pk+j90+q1Zch3yDzBY42ny9bO6aQNKkn90JuwzigUTpRtjpyRWYpq3ZNDtwTdSSPQ==";

auto HASH = "7c4f8b3fceae31610ff1b683eb7412be65053477b56c79a37eb632177a9370e9";
auto IROHA_DOMAIN = "test_domain";
auto ASSET = "DummyAsset";
auto NAME = "MizukiSonoko";

TEST(convertor, convertAddTransaction) {

    auto event = ConsensusEvent<Transaction<Add<Asset>>>(
        publicKey1,
        IROHA_DOMAIN,
        ASSET,
        100,
        0
    );

    event.addSignature(
        publicKey1,
        signature::sign( HASH, publicKey1, privateKey1).c_str()
    );

    Event::ConsensusEvent encodedEvent = convertor::encode(event);
    ConsensusEvent<Transaction<Add<Asset>>> reDecEvent = convertor::decode<Add<Asset>>(encodedEvent);

    ASSERT_TRUE(reDecEvent.eventSignatures().size() == 1);
    ASSERT_TRUE(std::get<0>(reDecEvent.eventSignatures()[0]) == publicKey1);
    ASSERT_TRUE(std::get<1>(reDecEvent.eventSignatures()[0]) == signature::sign( HASH, publicKey1, privateKey1).c_str());

    ASSERT_STREQ(reDecEvent.getCommandName(), "Add");

}


TEST(convertor, convertAddTransactionMultiEventSig) {

    auto event = ConsensusEvent<Transaction<Add<Asset>>>(
            publicKey1,
            IROHA_DOMAIN,
            ASSET,
            100,
            0
    );

    event.addSignature(
            publicKey1,
            signature::sign( HASH, publicKey1, privateKey1).c_str()
    );
    event.addSignature(
            publicKey2,
            signature::sign( HASH, publicKey2, privateKey2).c_str()
    );
    event.addSignature(
            publicKey1,
            signature::sign( HASH, publicKey1, privateKey1).c_str()
    );

    Event::ConsensusEvent encodedEvent = convertor::encode(event);
    ConsensusEvent<Transaction<Add<Asset>>> reDecEvent = convertor::decode<Add<Asset>>(encodedEvent);

    ASSERT_TRUE(reDecEvent.eventSignatures().size() == 3);
    ASSERT_TRUE(std::get<0>(reDecEvent.eventSignatures()[0]) == publicKey1);
    ASSERT_TRUE(std::get<1>(reDecEvent.eventSignatures()[0]) == signature::sign( HASH, publicKey1, privateKey1).c_str());
    ASSERT_TRUE(std::get<0>(reDecEvent.eventSignatures()[1]) == publicKey2);
    ASSERT_TRUE(std::get<1>(reDecEvent.eventSignatures()[1]) == signature::sign( HASH, publicKey2, privateKey2).c_str());
    ASSERT_TRUE(std::get<0>(reDecEvent.eventSignatures()[2]) == publicKey1);
    ASSERT_TRUE(std::get<1>(reDecEvent.eventSignatures()[2]) == signature::sign( HASH, publicKey1, privateKey1).c_str());

    ASSERT_STREQ(reDecEvent.getCommandName(), "Add");
    ASSERT_STREQ(reDecEvent.domain.c_str(), IROHA_DOMAIN);
    ASSERT_STREQ(reDecEvent.name.c_str(), ASSET);
    ASSERT_TRUE(reDecEvent.value == 100);
    ASSERT_TRUE(reDecEvent.precision == 0);
}

TEST(convertor, convertTransferTransaction) {

    auto event = ConsensusEvent<Transaction<Transfer<Asset>>>(
        publicKey1,
        publicKey1,
        publicKey2,
        ASSET,
        723
    );

    event.addSignature(
        publicKey1,
        signature::sign( HASH, publicKey1, privateKey1).c_str()
    );

    Event::ConsensusEvent encodedEvent = convertor::encode(event);
    ConsensusEvent<Transaction<Transfer<Asset>>> reDecEvent = convertor::decode<Transfer<Asset>>(encodedEvent);

    ASSERT_TRUE(reDecEvent.eventSignatures().size() == 1);
    ASSERT_TRUE(std::get<0>(reDecEvent.eventSignatures()[0]) == publicKey1);
    ASSERT_TRUE(std::get<1>(reDecEvent.eventSignatures()[0]) == signature::sign( HASH, publicKey1, privateKey1).c_str());

    ASSERT_STREQ(reDecEvent.getCommandName(), "Transfer");
    ASSERT_STREQ(reDecEvent.name.c_str(), ASSET);
    ASSERT_TRUE(reDecEvent.value == 723);
}

TEST(convertor, convertUpdateTransaction) {

    auto event = ConsensusEvent<Transaction<Update<Asset>>>(
            publicKey1,
            publicKey1,
            ASSET,
            1204
    );

    event.addSignature(
            publicKey1,
            signature::sign( HASH, publicKey1, privateKey1).c_str()
    );

    Event::ConsensusEvent encodedEvent = convertor::encode(event);
    ConsensusEvent<Transaction<Update<Asset>>> reDecEvent = convertor::decode<Update<Asset>>(encodedEvent);

    ASSERT_TRUE(reDecEvent.eventSignatures().size() == 1);
    ASSERT_TRUE(std::get<0>(reDecEvent.eventSignatures()[0]) == publicKey1);
    ASSERT_TRUE(std::get<1>(reDecEvent.eventSignatures()[0]) == signature::sign( HASH, publicKey1, privateKey1).c_str());

    ASSERT_STREQ(reDecEvent.getCommandName(), "Update");
    ASSERT_STREQ(reDecEvent.name.c_str(), ASSET);
    ASSERT_TRUE(reDecEvent.value == 1204);
}


TEST(convertor, convertAddAccountTransaction) {

    std::vector<std::tuple<std::string, long>> assets;
    assets.push_back(std::make_pair("Sample1",  311));
    assets.push_back(std::make_pair("Sample2", 1204));
    assets.push_back(std::make_pair("Sample3",  324));
    auto event = ConsensusEvent<Transaction<Add<Account>>>(
        publicKey1,
        publicKey2,
        NAME,
        std::move(assets)
    );

    event.addSignature(
        publicKey1,
        signature::sign( HASH, publicKey1, privateKey1).c_str()
    );

    Event::ConsensusEvent encodedEvent = convertor::encode(event);
    ConsensusEvent<Transaction<Add<Account>>> reDecEvent = convertor::decode<Add<Account>>(encodedEvent);

    ASSERT_TRUE(reDecEvent.eventSignatures().size() == 1);
    ASSERT_TRUE(std::get<0>(reDecEvent.eventSignatures()[0]) == publicKey1);
    ASSERT_TRUE(std::get<1>(reDecEvent.eventSignatures()[0]) == signature::sign( HASH, publicKey1, privateKey1).c_str());

    ASSERT_STREQ(reDecEvent.getCommandName(), "Add");
    ASSERT_STREQ(reDecEvent.name.c_str(), NAME);
    ASSERT_TRUE(reDecEvent.assets.size() == 3);
    ASSERT_STREQ(std::get<0>(reDecEvent.assets[1]).c_str(), "Sample2");
    ASSERT_TRUE(std::get<1>(reDecEvent.assets[1]) == 1204);

}


TEST(convertor, convertSerialize) {

    std::vector<std::tuple<std::string, long>> assets;
    assets.push_back(std::make_pair("Sample1",  311));
    assets.push_back(std::make_pair("Sample2", 1204));
    assets.push_back(std::make_pair("Sample3",  324));
    auto event = ConsensusEvent<Transaction<Add<Account>>>(
            publicKey1,
            publicKey2,
            NAME,
            std::move(assets)
    );

    event.addSignature(
            publicKey1,
            signature::sign( HASH, publicKey1, privateKey1).c_str()
    );

    Event::ConsensusEvent encodedEvent = convertor::encode(event);


    std::string strEvent;
    encodedEvent.SerializeToString(&strEvent);

    Event::ConsensusEvent encodedEvent2;

    encodedEvent2.ParseFromString(strEvent);
    ConsensusEvent<Transaction<Add<Account>>> reDecEvent = convertor::decode<Add<Account>>(encodedEvent2);

    ASSERT_TRUE(reDecEvent.eventSignatures().size() == 1);
    ASSERT_TRUE(std::get<0>(reDecEvent.eventSignatures()[0]) == publicKey1);
    ASSERT_TRUE(std::get<1>(reDecEvent.eventSignatures()[0]) == signature::sign( HASH, publicKey1, privateKey1).c_str());

    ASSERT_STREQ(reDecEvent.getCommandName(), "Add");
    ASSERT_STREQ(reDecEvent.name.c_str(), NAME);
    ASSERT_TRUE(reDecEvent.assets.size() == 3);
    ASSERT_STREQ(std::get<0>(reDecEvent.assets[1]).c_str(), "Sample2");
    ASSERT_TRUE(std::get<1>(reDecEvent.assets[1]) == 1204);
}
