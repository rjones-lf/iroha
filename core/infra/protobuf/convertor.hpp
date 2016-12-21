//
// Created by 五十嵐太清 on 2016/12/16.
//

#ifndef IROHA_CONVERTOR_HPP
#define IROHA_CONVERTOR_HPP

#include "../../consensus/consensus_event.hpp"
#include "../../model/transaction.hpp"

#include "../../model/commands/add.hpp"
#include "../../model/commands/transfer.hpp"
#include "../../model/commands/update.hpp"

#include "../../model/objects/account.hpp"
#include "../../model/objects/asset.hpp"
#include "../../model/objects/domain.hpp"

#include "event.grpc.pb.h"

#include <iostream>
#include <type_traits>

namespace convertor{

    using namespace transaction;
    using namespace command;
    using namespace event;

    namespace detail{

        Event::Asset encodeObject(object::Asset aAsset);
        Event::Domain encodeObject(object::Domain aDomain);
        Event::Account encodeObject(object::Account aAccount);

        template <typename T>
        Event::Transaction encodeTransaction(Transaction<Add<T>> aTx){
            Event::Transaction tx;
            auto obj = encodeObject(static_cast<T>(aTx));
            tx.set_type("Add");
            tx.set_senderpubkey(aTx.senderPubkey);
            tx.set_timestamp(aTx.timestamp);
            tx.set_hash(aTx.getHash());
            if(std::is_same<T,object::Asset>::value){
                tx.mutable_asset()->CopyFrom(obj);
            }else if(std::is_same<T,object::Account>::value){
                tx.mutable_account()->CopyFrom(obj);
            }else if(std::is_same<T,object::Domain>::value) {
                tx.mutable_domain()->CopyFrom(obj);
            }
            return tx;
        }

        template <typename T>
        Event::Transaction encodeTransaction(Transaction<Transfer<T>> aTx){
            Event::Transaction tx;
            tx.set_type("Transfer");
            tx.set_senderpubkey(aTx.senderPublicKey);
            tx.set_receivepubkey(aTx.receiverPublicKey);
            tx.set_timestamp(aTx.timestamp);
            tx.set_hash(aTx.getHash());
            tx.mutable_asset()->CopyFrom(encodeObject(static_cast<T>(aTx)));
            return tx;
        }

        template <typename T>
        Event::Transaction encodeTransaction(Transaction<Update<T>> aTx){
            Event::Transaction tx;
            tx.set_type("Update");
            tx.set_senderpubkey(aTx.ownerPublicKey);
            tx.set_timestamp(aTx.timestamp);
            tx.set_hash(aTx.getHash());
            tx.mutable_asset()->CopyFrom(encodeObject(static_cast<T>(aTx)));
            return tx;
        }

        // ====== decode ======

        object::Asset decodeObject(Event::Asset aAsset);

        object::Account decodeObject(Event::Account aAccount);


        template <typename T>
        ConsensusEvent<Transaction<T>> decodeTransaction2ConsensusEvent(Event::Transaction tx){
            throw "Not Implements";
        }

        template <>
        ConsensusEvent<Transaction<Transfer<object::Asset>>> decodeTransaction2ConsensusEvent(Event::Transaction tx);

        template <>
        ConsensusEvent<Transaction<Add<object::Asset>>> decodeTransaction2ConsensusEvent(Event::Transaction tx);

        template <>
        ConsensusEvent<Transaction<Add<object::Account>>> decodeTransaction2ConsensusEvent(Event::Transaction tx);

        template <>
        ConsensusEvent<Transaction<Update<object::Asset>>> decodeTransaction2ConsensusEvent(Event::Transaction tx);

    }

    template <typename T>
    Event::ConsensusEvent encode(const event::ConsensusEvent<transaction::Transaction<T>>& event){
        auto encTx = detail::encodeTransaction(static_cast<transaction::Transaction<T>>(event));
        Event::ConsensusEvent consensusEvent;
        consensusEvent.set_order(event.order);
        for(auto&& esig: event.eventSignatures()){
            Event::EventSignature es;
            es.set_publickey(std::get<0>(esig));
            es.set_signature(std::get<1>(esig));
            consensusEvent.add_eventsignatures()->CopyFrom(es);
        }
        consensusEvent.mutable_transaction()->CopyFrom(encTx);

        return consensusEvent;
    }

    template <typename T>
    event::ConsensusEvent<transaction::Transaction<T>> decode(const Event::ConsensusEvent& event){
        auto consensusEvent = detail::decodeTransaction2ConsensusEvent<T>(event.transaction());
        for(const auto& esig: event.eventsignatures()){
            consensusEvent.addSignature(esig.publickey(), esig.signature());
        }
        for(const auto& txsig: event.transaction().txsignatures()){
            consensusEvent.addTxSignature(txsig.publickey(), txsig.signature());
        }
        return consensusEvent;
    }

};

#endif //IROHA_CONVERTOR_HPP
