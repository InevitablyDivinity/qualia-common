#pragma once
#include "common/types.hpp"
#include <type_traits>
#include <utility>

namespace ql
{

template<std::size_t I, typename T>
class TupleElement
{
public:

  T value;
};

template<typename Firsts, typename Seconds>
class TupleBase;

template<std::size_t... Firsts, typename... Seconds>
class TupleBase<std::index_sequence<Firsts...>, parameter_pack<Seconds...>>
  : public TupleElement<Firsts, Seconds>...
{
};

template<typename... Ts>
class Tuple
  : public TupleBase<std::index_sequence_for<Ts...>, parameter_pack<Ts...>>
{
public:

  static constexpr std::size_t size = sizeof...( Ts );

  template<std::size_t I>
  constexpr auto& get()
  {
    auto value_type    = get_nth_type<I>( parameter_pack<Ts...>() );
    using element_type = TupleElement<I, typename decltype( value_type )::type>;
    return element_type::value;
  }

  template<std::size_t I>
  constexpr const auto& get() const
  {
    auto value_type    = get_nth_type<I>( parameter_pack<Ts...>() );
    using element_type = TupleElement<I, typename decltype( value_type )::type>;
    return element_type::value;
  }
};

} // namespace ql

template<typename... Ts>
struct std::tuple_size<ql::Tuple<Ts...>>
{
  static constexpr std::size_t value = ql::Tuple<Ts...>::size;
};

template<std::size_t I, typename... Ts>
struct std::tuple_element<I, ql::Tuple<Ts...>>
{
  using type =
    typename decltype( ql::get_nth_type<I>( ql::parameter_pack<Ts...>() ) )::type;
};

namespace std
{
  template<std::size_t I, typename... Ts>
  auto& get( ql::Tuple<Ts...>& t )
  {
    return t.template get<I>();
  }
}

