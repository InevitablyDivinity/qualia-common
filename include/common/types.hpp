#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace ql
{

template<typename T>
struct type_identity
{
  using type = T;
};


template<typename... Ts>
struct parameter_pack
{
};


template<typename T>
struct function_prototype
{
};

template<typename R, typename... Ts>
struct function_prototype<R( Ts... )>
{
};

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
struct current_type
{
  using type = T;
};

template<typename T, typename... Ts>
using current_type_t = typename current_type<T>::type;

template<typename T, typename... Ts>
consteval auto get_current_type( parameter_pack<T, Ts...> )
{
  return type_identity<T>();
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

template<std::size_t N, std::size_t I, typename T, typename... Ts>
consteval auto get_n_types( parameter_pack<T, Ts...> p )
{
  if constexpr ( N == I )
  {
    return p;
  }
  else
  {
    return get_n_types<N, I + 1>( parameter_pack<Ts...>() );
  }
}

template<std::size_t N, typename T, typename... Ts>
consteval auto get_n_types( parameter_pack<T, Ts...> p )
{
  if constexpr ( N == 0 )
  {
    return parameter_pack<void>();
  }
  else if constexpr ( N == 1 )
  {
    return parameter_pack<T>();
  }
  else
  {
    return get_n_types<N, 2>( parameter_pack<Ts...>() );
  }
}

template<std::size_t I, typename... Ts>
consteval auto get_nth_type( parameter_pack<Ts...> p )
{
  return get_current_type( remove_n_types<I>( p ) );
}

template<typename T>
consteval auto get_largest_type( parameter_pack<T> )
{
  return type_identity<T>();
}

template<typename T, typename U, typename... Ts>
consteval auto get_largest_type( parameter_pack<T, U, Ts...> )
{
  if constexpr ( sizeof( T ) > sizeof( U ) )
  {
    return get_largest_type( parameter_pack<T, Ts...>() );
  }
  else
  {
    return get_largest_type( parameter_pack<U, Ts...>() );
  }
}


// Returns the index of a type from an array of unique types
template<std::size_t I, typename T, typename U, typename... Ts>
consteval std::size_t get_index_for_type( parameter_pack<U, Ts...> p )
{
  if constexpr ( std::is_same_v<T, U> )
  {
    return I;
  }
  else
  {
    return get_index_for_type<I + 1, T>( parameter_pack<Ts...>() );
  }
}

template<typename T, typename U, typename... Ts>
consteval std::size_t get_index_for_type( parameter_pack<U, Ts...> p )
{
  if constexpr ( std::is_same_v<T, U> )
  {
    return 0;
  }
  else
  {
    return get_index_for_type<1, T>( parameter_pack<Ts...>() );
  }
}

template<std::size_t I, typename... Ts>
consteval auto get_type_for_index()
{
  return get_nth_type<I>( parameter_pack<Ts...>() );
}

template<typename T, typename... Ts>
static constexpr std::size_t index_for_type_v = get_index_for_type<T>( parameter_pack<Ts...>() );

template<std::size_t I, typename... Ts>
using type_for_index_t = typename decltype( get_type_for_index<I, Ts...>() )::type;

template<typename T, typename... Ts>
concept is_any_of = ( std::same_as<T, Ts> || ... );

} // namespace ql
