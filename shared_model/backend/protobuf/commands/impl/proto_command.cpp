/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/commands/proto_command.hpp"
#include "utils/variant_deserializer.hpp"

using Variant = shared_model::proto::Command::ProtoCommandVariantType;
template Variant::~variant();
template Variant::variant(Variant &&);
template void Variant::destroy_content();
template int Variant::which() const;
template void Variant::indicate_which(int);
template bool Variant::using_backup() const;

namespace shared_model {
  namespace proto {

    template <typename CommandType>
    Command::Command(CommandType &&command)
        : CopyableProto(std::forward<CommandType>(command)),
          variant_{[this] {
            auto &&ar = *proto_;
            int which = ar.GetDescriptor()
                            ->FindFieldByNumber(ar.command_case())
                            ->index();
            return shared_model::detail::variant_impl<ProtoCommandListType>::
                template load<ProtoCommandVariantType>(
                    std::forward<decltype(ar)>(ar), which);
          }},
          ivariant_{detail::makeLazyInitializer(
              [this] { return CommandVariantType(*variant_); })} {}

    template Command::Command(Command::TransportType &);
    template Command::Command(const Command::TransportType &);
    template Command::Command(Command::TransportType &&);

    Command::Command(const Command &o) : Command(o.proto_) {}

    Command::Command(Command &&o) noexcept : Command(std::move(o.proto_)) {}

    const Command::CommandVariantType &Command::get() const {
      return *ivariant_;
    }

  }  // namespace proto
}  // namespace shared_model
