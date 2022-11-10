#pragma once
#include <cstddef>
#include <type_traits>

namespace ql
{

template<typename... Ts>
struct parameter_pack
{
};

template<typename T>
struct function_prototype {};

template<typename R, typename... Ts>
struct function_prototype<R(Ts...)> {};

template<typename T>
struct callable_type;

template<typename R, typename... Args>
struct callable_type<R( Args... )>
{
  using return_type    = R;
  using argument_types = parameter_pack<Args...>;
};

template<typename R, typename... Args>
struct callable_type<R ( * )( Args... )> : callable_type<R( Args... )>
{
};

template<typename R, typename... Args>
struct callable_type<R ( & )( Args... )> : callable_type<R( Args... )>
{
};

template<typename T>
using return_type_t = typename callable_type<T>::return_type;

template<typename T>
using argument_types_t = typename callable_type<T>::argument_types;

template<typename T, typename... Ts>
consteval auto get_first_type( parameter_pack<T, Ts...> )
{
  return std::type_identity<T>();
}

template<std::size_t N, std::size_t I, typename T, typename... Ts>
consteval auto remove_n_types( parameter_pack<T, Ts...> )
{
  if constexpr ( N == I )
  {
    return parameter_pack<T, Ts...>();
  }
  else
  {
    static_assert( sizeof...( Ts ) != 0 );
    return remove_n_types<N, I + 1>( parameter_pack<Ts...>() );
  }
}

template<std::size_t N, typename T, typename... Ts>
consteval auto remove_n_types( parameter_pack<T, Ts...> )
{
  // Don't remove anything.
  if constexpr ( N == 0 )
  {
    return parameter_pack<T, Ts...>();
  }
  else
  {
    static_assert( sizeof...( Ts ) > 0 );
    return remove_n_types<N, 1>( parameter_pack<Ts...>() );
  }
}

template<std::size_t I, typename... Ts>
consteval auto get_nth_type( parameter_pack<Ts...> p )
{
  return get_first_type( remove_n_types<I>( p ) );
}

} // namespace ql
