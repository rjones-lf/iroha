/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TEMPORARY_WSV_IMPL_HPP
#define IROHA_TEMPORARY_WSV_IMPL_HPP

#include <soci/soci.h>

#include "ametsuchi/temporary_wsv.hpp"
#include "execution/command_executor.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "logger/logger.hpp"

namespace iroha {

  namespace ametsuchi {
    class TemporaryWsvImpl : public TemporaryWsv {
     public:
      struct SavepointWrapperImpl : public TemporaryWsv::SavepointWrapper {
        SavepointWrapperImpl(const TemporaryWsvImpl &wsv,
                             std::string savepoint_name);

        void release() override;

        ~SavepointWrapperImpl() override;

       private:
        soci::session &sql_;
        std::string savepoint_name_;
        bool is_released_;
      };

      explicit TemporaryWsvImpl(
          soci::session &sql,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              factory);

      expected::Result<void, validation::CommandError> apply(
          const shared_model::interface::Transaction &,
          std::function<expected::Result<void, validation::CommandError>(
              const shared_model::interface::Transaction &, WsvQuery &)>
              function) override;

      std::unique_ptr<TemporaryWsv::SavepointWrapper> createSavepoint(
          const std::string &name) override;

      ~TemporaryWsvImpl() override;

     private:
      soci::session &sql_;
      std::shared_ptr<WsvQuery> wsv_;
      std::shared_ptr<WsvCommand> executor_;
      std::shared_ptr<CommandExecutor> command_executor_;
      std::shared_ptr<CommandValidator> command_validator_;

      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_TEMPORARY_WSV_IMPL_HPP
