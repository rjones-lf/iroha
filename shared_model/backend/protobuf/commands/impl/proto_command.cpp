/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/commands/proto_command.hpp"
#include "utils/variant_deserializer.hpp"

template <typename... T, typename Archive>
auto loadCommand(Archive &&ar) {
  int which = ar.GetDescriptor()->FindFieldByNumber(ar.command_case())->index();
  return shared_model::detail::variant_impl<T...>::template load<
      shared_model::interface::Command::CommandVariantType>(
      std::forward<Archive>(ar), which);
}

namespace shared_model {
  namespace proto {

    template <typename CommandType>
    Command::Command(CommandType &&command)
        : CopyableProto(std::forward<CommandType>(command)), variant_{[this] {
            return loadCommand<ProtoCommandListType>(*proto_);
          }} {}

    template Command::Command(iroha::protocol::Command &command);
    template Command::Command(const iroha::protocol::Command &command);

    Command::Command(const Command &o) : Command(o.proto_) {}

    Command::Command(Command &&o) noexcept : Command(std::move(o.proto_)) {}

    const Command::CommandVariantType &Command::get() const {
      return *variant_;
    }

  }  // namespace proto
}  // namespace shared_model
