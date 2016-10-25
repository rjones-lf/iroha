#ifndef __WORLD_STATE_REPOSITORY_HPP_
#define __WORLD_STATE_REPOSITORY_HPP_

#include <vector>
#include <memory>
#include <string>

namespace repository {

  // I don't know model.

  namespace world_state_repository {

      bool add(const std::string &key, const std::string &value);

      bool update(const std::string &key, const std::string &value);

      bool remove(const std::string &key);

      std::string find(const std::string &key);

      std::string findOrElse(
          const std::string &key,
          const std::string &defaultValue
      );

      bool isExist(const std::string &key);

  };

}; // namespace repository

#endif // __WORLD_STATE_REPOSITORY_HPP_
