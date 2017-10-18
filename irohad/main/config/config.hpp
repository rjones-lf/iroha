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

#ifndef IROHA_CONFIG_HPP_
#define IROHA_CONFIG_HPP_

#include <boost/assert.hpp>
#include <sstream>
#include <string>
#include "common/byteutils.hpp"
#include "common/types.hpp"

namespace iroha {
  namespace config {

    /**
     * @class Config
     * @brief Configuration loader, storage and validator.
     */
    class Config {
     protected:
      /**
       * @struct Config::Service
       * @brief Network service.
       */
      struct Service {
        std::string host;
        uint16_t port;
      };

      /**
       * @struct Config::AuthService
       * @brief Network service with authentication.
       */
      struct AuthService : public Config::Service {
        std::string username;
        std::string password;
      };

     public:
      /**
       * @struct Config::BlockStore
       * @brief Config for block storage.
       */
      struct BlockStorage {
        std::string path;  ///< path to the block storage
      };

      /**
       * @struct Config::Cryptography
       * @brief Everything that is required for cryptography is here.
       */
      struct Cryptography {
        std::string public_key;   ///< content of the public key
        std::string private_key;  ///< content of the private key

        // TODO(@warchant): temporary solution. Refactor with Keypair object.
        keypair_t keypair() const noexcept;
      };

      /**
       * Config for Redis.
       */
      using Redis = Service;

      /**
       * @struct Config::Torii
       * @brief Config for torii gateway.
       */
      struct Torii : public Service {
        /**
         * Reurns preformatted "listen address" for torii:
         * host:port
         * @return
         */
        std::string listenAddress() const noexcept;
      };

      /**
       * @struct Config::Postgres
       * @brief Postgres config defined here.
       */
      struct Postgres : public AuthService {
        /**
         * Returns preformatted "options":
         * host=localhost port=$port user=$user password=$pwd
         * @return
         */
        std::string options() const noexcept;
      };

      /**
       * @struct Config::BlockchainOptions
       * @brief Control options, such as creation of new ledger (given genesis
       * block), or something else.
       */
      struct BlockchainOptions {
        std::string genesis_block;  ///< path to the file with "genesis block"
      };

      /**
       * Returns true, if config is loaded, false otherwise.
       * @return
       */
      bool loaded() const noexcept;

      /**
       * Reload config. Used to fill config with parsed data.
       * Derived classes define a load strategy.
       *
       * When config is loaded, this->loaded_ should be setted to true.
       * Clients should verify that config is loaded.
       */
      virtual void load() = 0;

      /// accessor for blockchain options
      inline const BlockchainOptions &blockchainOptions() const noexcept;

      /// accessor for redis options
      inline const Redis &redis() const noexcept;

      /// accessor for postgres options
      inline const Postgres &postgres() const noexcept;

      /// accessor for torii options
      inline const Torii &torii() const noexcept;

      /// accessor for block storage options
      inline const BlockStorage &blockStorage() const noexcept;

      /// accessor for peer cryptography options
      inline const Cryptography &cryptography() const noexcept;

     protected:
      // the reason why these private variables are here is that with new
      // implementation we do not change the logic of accessing parsed config,
      // we only change the way this config is loaded, e.g. load() method.
      bool loaded_{false};

      Redis redis_;
      Postgres pg_;
      Torii torii_;
      BlockStorage db_;
      Cryptography crypto_;
      BlockchainOptions options_;
    };

    /// IMPLEMENTATION
    // note: methods should be inline, otherwise there will be "redefinition"
    // error.

    inline const Config::Cryptography &Config::cryptography() const noexcept {
      return crypto_;
    }
    inline const Config::Redis &Config::redis() const noexcept {
      return redis_;
    }
    inline const Config::Postgres &Config::postgres() const noexcept {
      return pg_;
    }
    inline const Config::Torii &Config::torii() const noexcept {
      return torii_;
    }
    inline const Config::BlockStorage &Config::blockStorage() const noexcept {
      return db_;
    }
    inline const Config::BlockchainOptions &Config::blockchainOptions() const
        noexcept {
      return options_;
    }
    inline bool Config::loaded() const noexcept { return loaded_; }

    inline std::string Config::Postgres::options() const noexcept {
      std::stringstream ss;

      // host=localhost port=$port user=$user password=$pwd
      ss << "host=" << this->host << " port=" << this->port
         << " user=" << this->username << " password=" << this->password;

      return ss.str();
    }

    inline std::string Config::Torii::listenAddress() const noexcept {
      return this->host + ":" + std::to_string(this->port);
    }

    inline keypair_t Config::Cryptography::keypair() const noexcept {
      // TODO (@warchant): /17/10/2017 temp solution, will be removed when we
      // will use certificate
      auto pub = hexstringToArray<pubkey_t::size()>(public_key)
                     .value_or(pubkey_t::from_string(public_key));
      auto priv = hexstringToArray<privkey_t::size()>(private_key)
                      .value_or(privkey_t::from_string(private_key));
      return keypair_t{pub, priv};
    }
  }  // namespace config
}  // namespace iroha

#endif  //  IROHA_CONFIG_HPP_
