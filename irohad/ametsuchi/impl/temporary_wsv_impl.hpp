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

#ifndef IROHA_TEMPORARY_WSV_IMPL_HPP
#define IROHA_TEMPORARY_WSV_IMPL_HPP

#include <soci/soci.h>

#include "ametsuchi/command_executor.hpp"
#include "ametsuchi/temporary_wsv.hpp"
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
        std::shared_ptr<soci::session> sql_;
        std::string savepoint_name_;
        bool is_released_;
      };

      explicit TemporaryWsvImpl(std::unique_ptr<soci::session> sql,
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
      std::shared_ptr<soci::session> sql_;
      std::shared_ptr<WsvQuery> wsv_;
      std::unique_ptr<CommandExecutor> command_executor_;

      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_TEMPORARY_WSV_IMPL_HPP
