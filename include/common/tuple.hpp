#pragma once
#include "common/common.hpp"
#include "common/types.hpp"
#include <type_traits>
#include <utility>

namespace ql
{

template<std::size_t I, typename T>
class TupleElement
{
public:

  using type = std::decay_t<T>;
  type value;
};

template<typename Firsts, typename Seconds>
class TupleBase;

template<std::size_t... Firsts, typename... Seconds>
class TupleBase<std::index_sequence<Firsts...>, parameter_pack<Seconds...>>
  : public TupleElement<Firsts, Seconds>...
{
};

template<typename... Ts>
class Tuple : public TupleBase<std::index_sequence_for<Ts...>, parameter_pack<Ts...>>
{
public:

  template<std::size_t I>
  FORCEINLINE constexpr auto& get()
  {
    return TupleElement<I, type_for_index_t<I, Ts...>>::value;
  }

  template<std::size_t I>
  FORCEINLINE constexpr const auto& get() const
  {
    return TupleElement<I, type_for_index_t<I, Ts...>>::value;
  }
};

template<typename... Ts>
Tuple( Ts... ) -> Tuple<Ts...>;

} // namespace ql

namespace std
{
template<typename... Ts>
struct tuple_size<ql::Tuple<Ts...>>
{
  static constexpr std::size_t value = sizeof...( Ts );
};

template<std::size_t I, typename... Ts>
struct tuple_element<I, ql::Tuple<Ts...>>
{
  using type = std::decay_t<ql::type_for_index_t<I, Ts...>>;
};

template<std::size_t I, typename... Ts>
FORCEINLINE constexpr auto& get( ql::Tuple<Ts...>& t )
{
  return static_cast<ql::TupleElement<I, ql::type_for_index_t<I, Ts...>>&>( t ).value;
}

template<std::size_t I, typename... Ts>
FORCEINLINE constexpr const auto& get( const ql::Tuple<Ts...>& t )
{
  return static_cast<const ql::TupleElement<I, ql::type_for_index_t<I, Ts...>>&>( t ).value;
}

template<typename T, typename... Ts>
  requires ql::unique_types<Ts...>
FORCEINLINE constexpr auto& get( ql::Tuple<Ts...>& t )
{
  return std::get<ql::index_for_type_v<T, Ts...>>( t );
}

template<typename T, typename... Ts>
  requires ql::unique_types<Ts...>
FORCEINLINE constexpr const auto& get( const ql::Tuple<Ts...>& t )
{
  return std::get<ql::index_for_type_v<T, Ts...>>( t );
}
} // namespace std
