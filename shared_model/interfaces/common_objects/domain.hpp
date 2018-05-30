/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_SHARED_MODEL_DOMAIN_HPP
#define IROHA_SHARED_MODEL_DOMAIN_HPP

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Domain object represents administrative unit within the system
     */
    class Domain : public ModelPrimitive<Domain> {
     public:
      /**
       * @return Identity of domain
       */
      virtual const types::DomainIdType &domainId() const = 0;

      /**
       * @return Default role of domain
       */
      virtual const types::RoleIdType &defaultRole() const = 0;
      /**
       * Stringify the data.
       * @return the content of asset.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Domain")
            .append("domainId", domainId())
            .append("defaultRole", defaultRole())
            .finalize();
      }

      /**
       * Checks equality of objects inside
       * @param rhs - other wrapped value
       * @return true, if wrapped objects are same
       */
      bool operator==(const ModelType &rhs) const override {
        return domainId() == rhs.domainId()
            and defaultRole() == rhs.defaultRole();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_DOMAIN_HPP
