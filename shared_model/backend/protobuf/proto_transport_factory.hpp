/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_TRANSPORT_FACTORY_HPP
#define IROHA_PROTO_TRANSPORT_FACTORY_HPP

#include "interfaces/iroha_internal/abstract_transport_factory.hpp"

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

      explicit ProtoTransportFactory(ValidatorType validator)
          : validator_(std::move(validator)) {}

      iroha::expected::Result<std::unique_ptr<Interface>, Error> build(
          typename Proto::TransportType m) const override {
        std::unique_ptr<Interface> result =
            std::make_unique<Proto>(std::move(m));
        if (auto answer = validator_->validate(*result)) {
          return iroha::expected::makeError(
              Error{result->hash(), answer.reason()});
        }
        return iroha::expected::makeValue(std::move(result));
      }

     private:
      ValidatorType validator_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_TRANSPORT_FACTORY_HPP
