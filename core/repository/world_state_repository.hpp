#ifndef __WORLD_STATE_REPOSITORY_HPP_
#define __WORLD_STATE_REPOSITORY_HPP_

#include <vector>
#include <memory>
#include <string>

namespace repository{

  // I don't know model.

  namespace world_state_repository {

      bool add(const std::string &key, const std::string &value);

      bool update(std::string key, std::string value);

      bool remove(std::string key);

      std::string find(std::string key);

      std::string findOrElse(std::string key, std::string defaultVale);

      bool isExist(std::string key);

  };

}; // namespace repository

#endif // __WORLD_STATE_REPOSITORY_HPP_
