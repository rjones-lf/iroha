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

#ifndef IROHA_SHARED_MODEL_TRIVIAL_PROTO_HPP
#define IROHA_SHARED_MODEL_TRIVIAL_PROTO_HPP

#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    /**
     * Simple generic class for handling proto objects
     * @tparam Iface is interface to inherit from
     * @tparam Proto is protobuf container
     */
    template <typename Iface, typename Proto>
    class TrivialProto final : public Iface {
     public:
      /**
       * @tparm ProtoLoader generic param so it can be handled
       *                    in the load for the boost::variant
       */
      template <typename ProtoLoader>
      explicit TrivialProto(ProtoLoader &&ref)
          : proto_(std::forward<ProtoLoader>(ref)) {}

     protected:
      typename Iface::ModelType *clone() const override {
        return new TrivialProto(Proto(*proto_));
      }

     private:
      detail::ReferenceHolder<Proto> proto_;
    };

    /**
     * Simple generic class for handling proto objects
     * @tparam Iface is interface to inherit from
     * @tparam Proto is protobuf container
     * @tparam Impl is implementation of Iface
     */
    template <typename Iface, typename Proto, typename Impl>
    class CopyableProto : public Iface {
     public:
      /**
       * @tparm ProtoLoader generic param so it can be handled
       *                    in the load for the boost::variant
       */
      template <typename ProtoLoader>
      explicit CopyableProto(ProtoLoader &&ref)
          : proto_(std::forward<ProtoLoader>(ref)) {}

      using TransportType = Proto;

      const Proto &getTransport() const {
        return *proto_;
      }

     protected:
      typename Iface::ModelType *clone() const override final {
        return new Impl(Proto(*proto_));
      }
      detail::ReferenceHolder<Proto> proto_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_TRIVIAL_PROTO_HPP
