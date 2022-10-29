#pragma once
#include <initializer_list>
#include <utility>

namespace ql {

  template<typename T>
  constexpr std::remove_reference_t<T>&& move(T&& object)
  {
    return static_cast<std::remove_reference_t<T>&&>(object);
  }

  template<typename FirstType, typename SecondType>
  class Pair
  {
  public:

    using type = Pair<FirstType, SecondType>;

    bool operator==(const Pair &rhs) const
    {
      return first == rhs.first && second == rhs.second;
    }

    FirstType first;
    SecondType second;
  };

}
