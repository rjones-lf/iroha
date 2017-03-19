#ifndef CORE_MODEL_TYPE_SIGNATURES_ADD_HPP
#define CORE_MODEL_TYPE_SIGNATURES_ADD_HPP

#include "tags.hpp"
#include "../objects.hpp"

namespace type_signatures {

  template<class... Ts>
  struct Add {
    using type = typename detail::head<Ts...>::type;
  };

  template<>
  struct Add<type_signatures::Add<Asset, To<Account>>> {
  };

}


#endif