#pragma once
#include "common/types.hpp"
#include "common/functional.hpp"

namespace ql
{

  template<typename... Ts>
  class Variant
  {
  public:

    Variant() = default;

    Variant( const is_any_of<Ts...> auto& value )
    {
      assign( value );
    }

    Variant& operator=( const is_any_of<Ts...> auto& value )
    {
      destruct();

      assign( value );
      return *this;
    }

    template<typename T>
    bool holds_alternative()
    {
      constexpr std::size_t typeIndex = get_index_from_type<T>( parameter_pack<Ts...>() );
      return typeIndex == m_typeIndex;
    }

    void assign( const is_any_of<Ts...> auto& value )
    {
      using type = std::remove_cvref_t<decltype( value )>;
      new ( m_data ) type( value );
      m_typeIndex = get_index_from_type<type>( parameter_pack<Ts...>() );

      if constexpr ( std::is_class_v<type> && std::is_destructible_v<type> )
      {
        m_destructor = [this]
        () -> void
        {
          type* ptr = reinterpret_cast<type*>( m_data );
          ptr->~type();
        };
      }
      else
      {
        m_destructor = nullptr;
      }
    }

    template<typename T>
    auto& get()
    {
      return *reinterpret_cast<T*>( &m_data );
    }

    template<typename T>
    const auto& get() const
    {
      return *reinterpret_cast<T*>( &m_data );
    }

    template<std::size_t I>
    auto& get()
    {
      using type = typename decltype( get_nth_type<I>( parameter_pack<Ts...>() ) )::type;
      return get<type>();
    }

    template<std::size_t I>
    const auto& get() const
    {
      using type = typename decltype( get_nth_type<I>( parameter_pack<Ts...>() ) )::type;
      return get<type>();
    }

  private:

    void destruct()
    {
      if ( m_destructor )
      {
        m_destructor();
        m_destructor = nullptr;
      }
    }

    static constexpr std::size_t m_maxSize = get_largest_type_size( parameter_pack<Ts...>() );
    std::byte m_data[m_maxSize];
    ql::Function<void()> m_destructor;
    std::size_t m_typeIndex = SIZE_MAX;

  };

}

namespace std
{

  template<typename T, typename... Ts>
  auto& get( ql::Variant<Ts...>& variant )
  {
    return variant.template get<T>();
  }

  template<typename T, typename... Ts>
  const auto& get( const ql::Variant<Ts...>& variant )
  {
    return variant.template get<T>();
  }

  template<std::size_t I, typename... Ts>
  auto& get( ql::Variant<Ts...>& variant )
  {
    return variant.template get<I>();
  }

  template<std::size_t I, typename... Ts>
  const auto& get( const ql::Variant<Ts...>& variant )
  {
    return variant.template get<I>();
  }

}
