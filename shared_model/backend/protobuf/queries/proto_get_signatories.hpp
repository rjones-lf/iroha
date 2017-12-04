//
// Created by Dumitru Savva on 04/12/2017.
//

#ifndef IROHA_PROTO_GET_SIGNATORIES_H
#define IROHA_PROTO_GET_SIGNATORIES_H

//
// Created by Dumitru Savva on 04/12/2017.
//

#ifndef IROHA_PROTO_GET_ACCOUNT_ASSETS_H
#define IROHA_PROTO_GET_ACCOUNT_ASSETS_H

#include "interfaces/queries/get_signatories.hpp"

#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class GetSignatories final : public CopyableProto<interface::GetSignatories,
                                                      iroha::protocol::Query,
                                                      GetSignatories> {
     public:
      template <typename QueryType>
      explicit GetSignatories(QueryType &&query)
          : CopyableProto(std::forward<QueryType>(query)),
            account_id_(detail::makeReferenceGenerator(
                &proto_->payload(),
                &iroha::protocol::Query::Payload::get_account_signatories))
      {}

      GetSignatories(const GetSignatories &o)
          : GetSignatories(o.proto_) {}

      GetSignatories(GetSignatories &&o) noexcept
          : GetSignatories(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return account_id_->account_id();
      }


     private:
      // ------------------------------| fields |-------------------------------

      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<const iroha::protocol::GetSignatories &> account_id_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_ACCOUNT_ASSETS_H

#endif  // IROHA_PROTO_GET_SIGNATORIES_H
