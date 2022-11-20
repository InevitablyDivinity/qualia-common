#pragma once
#include "common/types.hpp"
#include "common/functional.hpp"
#include "common/memory.hpp"
#include "common/array.hpp"
#include <functional>

namespace ql
{

  template<typename... Ts>
  class Variant;


  template<typename T, typename V>
  struct variant_index;

  template<typename T, typename... Ts>
  struct variant_index<T, Variant<Ts...>>
  {
    static constexpr std::size_t value = index_for_type_v<T, Ts...>;
  };

  template<typename T, typename V>
  static constexpr std::size_t variant_index_v = variant_index<T, V>::value;


  template<std::size_t I, typename V>
  struct variant_alternative;

  template<std::size_t I, typename... Ts>
  struct variant_alternative<I, Variant<Ts...>>
  {
    using type = type_for_index_t<I, Ts...>;
  };

  template<std::size_t I, typename V>
  using variant_alternative_t = typename variant_alternative<I, V>::type;


  template<typename V>
  struct variant_size;

  template<typename... Ts>
  struct variant_size<Variant<Ts...>>
  {
    static constexpr std::size_t value = sizeof...( Ts );
  };

  template<typename V>
  static constexpr std::size_t variant_size_v = variant_size<V>::value;


  template<typename... Ts>
  class Variant
  {
  public:

    using variant_type = Variant<Ts...>;

    constexpr Variant( const is_any_of<Ts...> auto& value )
    {
      assign( value );
    }

    constexpr Variant& operator=( const is_any_of<Ts...> auto& value )
    {
      destruct();

      assign( value );
      return *this;
    }

    constexpr void assign( const is_any_of<Ts...> auto& value )
    {
      using type = std::remove_cvref_t<decltype( value )>;
      new ( m_object ) type( value );
      m_typeIndex = variant_index_v<type, variant_type>;

      if constexpr ( std::is_class_v<type> && std::is_destructible_v<type> )
      {
        m_destructor = [this]() {
          type* ptr = reinterpret_cast<type*>( m_object );
          destroy_at( ptr );
        };
      }
      else
      {
        m_destructor = nullptr;
      }
    }

    template<typename T>
    constexpr auto& get()
    {
      return *reinterpret_cast<T*>( &m_object );
    }

    template<typename T>
    constexpr const auto& get() const
    {
      return *reinterpret_cast<const T*>( &m_object );
    }

    template<std::size_t I>
    constexpr auto& get()
    {
      return get<variant_alternative_t<I, variant_type>>();
    }

    template<std::size_t I>
    constexpr const auto& get() const
    {
      return get<variant_alternative_t<I, variant_type>>();
    }

    constexpr std::size_t index() const { return m_typeIndex; }

    template<typename T>
    constexpr bool holds_alternative()
    {
      return variant_index_v<T, variant_type> == index();
    }

  private:

    constexpr void destruct()
    {
      if ( m_destructor )
      {
        m_destructor();
        m_destructor = nullptr;
      }
    }

    std::byte m_object[std::max( { alignof( Ts )... } )];
    ql::Function<void()> m_destructor;
    std::size_t m_typeIndex;

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

  template<typename T>
  struct variant_size;

  template<typename... Ts>
  struct variant_size<ql::Variant<Ts...>> : ql::variant_size<ql::Variant<Ts...>>
  {
  };

  template<std::size_t I, typename V>
  struct variant_alternative;

  template<std::size_t I, typename... Ts>
  struct variant_alternative<I, ql::Variant<Ts...>> : ql::variant_alternative<I, ql::Variant<Ts...>>
  {
  };

}


namespace ql
{

  // Base case
  template<typename VisitorType, typename... VariantTypes, std::size_t... Indices>
  consteval auto* create_dispatch_matrix( std::index_sequence<Indices...> )
  {
    struct Dispatcher
    {
      static constexpr auto dispatch( VisitorType visitor, VariantTypes... variants )
      {
        return visitor( std::get<Indices>( variants )... );
      }
    };

    return &Dispatcher::dispatch;
  };

  template<typename VisitorType, typename... VariantTypes, std::size_t... Is, std::size_t... Js, typename... Ks>
  consteval auto create_dispatch_matrix( std::index_sequence<Is...>, std::index_sequence<Js...>, Ks... ks
  )
  {
    return ql::make_array(
      create_dispatch_matrix<VisitorType, VariantTypes...>(
        std::index_sequence<Is..., Js>(),
        ks...
      )...
    );
  };

  template<typename VisitorType, typename... VariantTypes>
  consteval auto get_dispatch_matrix()
  {
    return create_dispatch_matrix<VisitorType, VariantTypes...>(
      std::index_sequence<>(),
      std::make_index_sequence<variant_size_v<std::decay_t<VariantTypes>>>()...
    );
  }


  template<typename T>
  constexpr T&& at_impl( T&& element ) { return std::forward<T>( element ); }

  template<typename T, typename... Indices>
  constexpr auto&& at_impl( T&& elements, std::size_t i, Indices... indices )
  {
    return at_impl( std::forward<T>( elements )[i], indices... );
  }

  template<typename T, typename... Indices>
  constexpr auto&& at( T&& elements, Indices... indices )
  {
    return at_impl( std::forward<T>( elements ), indices... );
  }

  template<typename F, typename... Vs>
  constexpr auto visit( F&& f, Vs&&... variants )
  {
    constexpr auto dispatchMatrix = get_dispatch_matrix<F&&, Vs&&...>();
    auto dispatch = at( dispatchMatrix, variants.index()... );
    return dispatch( std::forward<F>( f ), std::forward<Vs>( variants )... );
  }


  template<typename... Ts>
  struct Overload : Ts...
  {
    using Ts::operator()...;
  };

  template<typename... Ts>
  Overload( Ts... ) -> Overload<Ts...>;

  template<typename V, typename... Fs>
  constexpr auto match( V&& v, Fs&&... fs )
  {
    return ql::visit( ql::Overload { fs... }, v );
  }

}

