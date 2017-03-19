#ifndef CORE_MODEL_TYPE_SIGNATURES_REMOVE_HPP
#define CORE_MODEL_TYPE_SIGNATURES_REMOVE_HPP

#include "tags.hpp"

namespace type_signatures {

  template<class... Ts>
  struct Remove {
    using type = typename detail::head<Ts...>::type;
  };

}


#endif