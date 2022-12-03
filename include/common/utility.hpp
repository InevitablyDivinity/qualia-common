#pragma once
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace ql
{

template<typename T>
constexpr T&& forward( std::remove_reference_t<T>& t )
{
  return static_cast<T&&>( t );
}

template<typename T>
constexpr T&& forward( std::remove_reference_t<T>&& t )
{
  return static_cast<T&&>( t );
}

template<typename T>
constexpr std::underlying_type_t<T> to_underlying( T e )
{
  return static_cast<std::underlying_type_t<T>>( e );
}

template<typename T>
constexpr std::remove_reference_t<T>&& move( T&& object )
{
  return static_cast<std::remove_reference_t<T>&&>( object );
}

constexpr auto exchange( auto& object, auto&& value )
{
  auto tmp = move( object );
  object = forward( value );
  return tmp;
}

template<typename FirstType, typename SecondType>
class Pair
{
public:

  using type = Pair<FirstType, SecondType>;

  bool operator==( const Pair& rhs ) const
  {
    return first == rhs.first && second == rhs.second;
  }

  FirstType  first;
  SecondType second;
};

} // namespace ql
