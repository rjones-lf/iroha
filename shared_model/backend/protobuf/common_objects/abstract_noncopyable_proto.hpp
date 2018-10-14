/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ABSTRACT_NONCOPYABLE_PROTO_HPP
#define IROHA_ABSTRACT_NONCOPYABLE_PROTO_HPP

/**
 * Generic class for handling proto objects with pimpl which are not intended to
 * be copied.
 * @tparam Iface is interface to inherit from
 * @tparam Proto is protobuf container
 * @tparam Impl is implementation of Iface
 */
template <typename Iface, typename Proto, typename Impl, typename Pimpl>
class AbstractNonCopyableProto : public Iface {
 public:
  using TransportType = Proto;

  /*
   * Construct object from transport. Transport can be moved or copied.
   */
  template <typename Transport>
  AbstractNonCopyableProto(Transport &&ref)
      : impl(std::make_unique<Pimpl>(std::forward<Transport>(ref))) {}
  AbstractNonCopyableProto(const AbstractNonCopyableProto &o) = delete;
  AbstractNonCopyableProto &operator=(const AbstractNonCopyableProto &o) =
      delete;

  const Proto &getTransport() const {
    return impl->getTransport();
  }

 protected:
  typename Iface::ModelType *clone() const override final {
    return new Impl(std::move(impl->getTransport()));
  }

  std::unique_ptr<Pimpl> impl;
};

#endif  // IROHA_ABSTRACT_NONCOPYABLE_PROTO_HPP
