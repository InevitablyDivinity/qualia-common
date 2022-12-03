#pragma once
#include "common/variant.hpp"

namespace ql
{

  template<typename T, typename E>
  class Result : public Variant<T, E>
  {
    using base = Variant<T, E>;
  public:

    using value_type = T;
    using error_type = E;

    constexpr Result( const is_any_of<T, E> auto& value )
    : base( value )
    {
    }

    using base::operator=;

    constexpr operator base()
    {
      return base( *this );
    }

    constexpr bool has_value() const
    {
      return holds_alternative<value_type>( *this );
    }

    constexpr bool has_error() const
    {
      return holds_alternative<error_type>( *this );
    }

    constexpr value_type& value()
    {
      return base::template get<value_type>();
    }

    constexpr const value_type& value() const
    {
      return base::template get<value_type>();
    }

    constexpr error_type& error()
    {
      return base::template get<error_type>();
    }

    constexpr const error_type& error() const
    {
      return base::template get<error_type>();
    }

  };

  template<typename T, typename... Ts>
  struct variant_index<T, Result<Ts...>>
  {
    static constexpr std::size_t value = index_for_type_v<T, Ts...>;
  };

  template<std::size_t I, typename... Ts>
  struct variant_alternative<I, Result<Ts...>>
  {
    using type = type_for_index_t<I, Ts...>;
  };


  template<typename... Ts>
  struct variant_size<Result<Ts...>>
  {
    static constexpr std::size_t value = sizeof...( Ts );
  };

};
