#ifndef CORE_MODEL_TYPE_SIGNATURES_TAGS_HPP
#define CORE_MODEL_TYPE_SIGNATURES_TAGS_HPP

namespace type_signatures {

namespace detail {
template <class T, class... Rest>
struct head {
  using type = T;
};
}

template<class T> struct To {};

}

#endif