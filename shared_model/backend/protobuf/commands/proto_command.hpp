/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_COMMAND_HPP
#define IROHA_SHARED_MODEL_PROTO_COMMAND_HPP

#include "commands.pb.h"
#include "interfaces/commands/command.hpp"

namespace shared_model {
  namespace proto {

    class Command final : public interface::Command {
     public:
      using TransportType = iroha::protocol::Command;

      Command(Command &&o) noexcept;

      explicit Command(TransportType &ref);

      ~Command() override;

      const CommandVariantType &get() const override;

     protected:
      Command *clone() const override;

     private:
      struct Impl;
      std::unique_ptr<Impl> impl_;

      void logError(const std::string &message) const;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_COMMAND_HPP
