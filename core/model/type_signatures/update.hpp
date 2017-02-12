#ifndef CORE_MODEL_TYPE_SIGNATURES_UPDATE_HPP
#define CORE_MODEL_TYPE_SIGNATURES_UPDATE_HPP

#include "tags.hpp"

namespace type_signatures {

template <class... Ts>
struct Update {
  using type = typename detail::head< Ts... >::type;
  object::Object object;
  Update(type o): object(o) {}
};

template <>
struct Update<type_signatures::Add<object::Asset, To<object::Account>>> {
  object::Object object;
  Update(object::Account rhs): object(rhs) {}
};

}



#endif