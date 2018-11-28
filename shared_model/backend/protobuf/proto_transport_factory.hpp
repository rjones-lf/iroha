/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_TRANSPORT_FACTORY_HPP
#define IROHA_PROTO_TRANSPORT_FACTORY_HPP

#include "interfaces/iroha_internal/abstract_transport_factory.hpp"

#include "backend/protobuf/util.hpp"
#include "cryptography/hash_providers/sha3_256.hpp"
#include "validators/abstract_validator.hpp"

namespace shared_model {
  namespace proto {

    template <typename Interface, typename Proto>
    class ProtoTransportFactory : public interface::AbstractTransportFactory<
                                      Interface,
                                      typename Proto::TransportType> {
     public:
      using typename interface::AbstractTransportFactory<
          Interface,
          typename Proto::TransportType>::Error;
      using ValidatorType = std::unique_ptr<
          shared_model::validation::AbstractValidator<Interface>>;
      using ProtoValidatorType =
          std::unique_ptr<shared_model::validation::AbstractValidator<
              typename Proto::TransportType>>;

      ProtoTransportFactory(ValidatorType interface_validator,
                            ProtoValidatorType proto_validator)
          : interface_validator_(std::move(interface_validator)),
            proto_validator_{std::move(proto_validator)} {}

      iroha::expected::Result<std::unique_ptr<Interface>, Error> build(
          typename Proto::TransportType m) const override {
        if (auto answer = proto_validator_->validate(m)) {
          return iroha::expected::makeError(Error{
              HashProvider::makeHash(makeBlob(m.payload())), answer.reason()});
        }

        std::unique_ptr<Interface> result =
            std::make_unique<Proto>(std::move(m));
        if (auto answer = interface_validator_->validate(*result)) {
          return iroha::expected::makeError(
              Error{result->hash(), answer.reason()});
        }

        return iroha::expected::makeValue(std::move(result));
      }

     private:
      using HashProvider = shared_model::crypto::Sha3_256;

      ValidatorType interface_validator_;
      ProtoValidatorType proto_validator_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_TRANSPORT_FACTORY_HPP
