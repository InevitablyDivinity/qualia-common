#pragma once
#include "common/types.hpp"
#include "common/memory.hpp"
#include "common/array.hpp"

namespace ql
{

  template<typename... Ts>
  class Variant;

  template<typename... Ts>
  union VariadicUnion;

  template<typename... Ts>
  struct Overload;

  template<typename T, typename V>
  struct variant_index;

  template<typename T, typename V>
  static constexpr std::size_t variant_index_v = variant_index<T, V>::value;

  template<std::size_t I, typename V>
  struct variant_alternative;

  template<std::size_t I, typename V>
  using variant_alternative_t = typename variant_alternative<I, V>::type;

  template<typename V>
  struct variant_size;

  template<typename V>
  static constexpr std::size_t variant_size_v = variant_size<V>::value;

  template<std::size_t N>
  struct in_place_index_t {};

  template<std::size_t N>
  static constexpr in_place_index_t<N> in_place_index;

  template<typename VisitorType, typename... VariantTypes>
  constexpr auto visit( VisitorType&& f, VariantTypes&&... variants );

  template<typename Variant, typename... VisitorTypes>
  constexpr auto match( Variant&& variant, VisitorTypes&&... visitors );

}

namespace std
{

  template<typename T>
  struct variant_size;

  template<std::size_t I, typename V>
  struct variant_alternative;

  template<typename T, typename... Ts>
  constexpr auto& get( ql::Variant<Ts...>& variant );

  template<typename T, typename... Ts>
  constexpr const auto& get( const ql::Variant<Ts...>& variant );

  template<std::size_t I, typename... Ts>
  constexpr auto& get( ql::Variant<Ts...>& variant );

  template<std::size_t I, typename... Ts>
  constexpr const auto& get( const ql::Variant<Ts...>& variant );

  template<std::size_t I, typename... Ts>
  constexpr auto& get( ql::VariadicUnion<Ts...>& variant );

  template<std::size_t I, typename... Ts>
  constexpr const auto& get( const ql::VariadicUnion<Ts...>& variant );

}

namespace ql
{

  template<typename T, typename... Ts>
  struct variant_index<T, Variant<Ts...>>
  {
    static constexpr std::size_t value = index_for_type_v<T, Ts...>;
  };

  template<std::size_t I, typename... Ts>
  struct variant_alternative<I, Variant<Ts...>>
  {
    using type = type_for_index_t<I, Ts...>;
  };

  template<typename... Ts>
  struct variant_size<Variant<Ts...>>
  {
    static constexpr std::size_t value = sizeof...( Ts );
  };

  template<typename... VisitorTypes>
  struct Overload : VisitorTypes...
  {
    using VisitorTypes::operator()...;
  };

  template<typename... VisitorTypes>
  Overload( VisitorTypes... ) -> Overload<VisitorTypes...>;

  // Tail, default active member
  template<typename... Ts>
  union VariadicUnion
  {
    constexpr VariadicUnion() = default;
  };

  template<typename T, typename... Ts>
  union VariadicUnion<T, Ts...>
  {
    T m_value;
    VariadicUnion<Ts...> m_rest;

    // In-place constructor
    template<typename... Args>
    constexpr VariadicUnion( in_place_index_t<0>, Args&&... args )
    : m_value( forward<Args>( args )... )
    {
    }

    // Recursive in-place constructor
    template<std::size_t I, typename... Args>
    constexpr VariadicUnion( in_place_index_t<I>, Args&&... args )
    : m_rest( in_place_index<I - 1>, forward<Args>( args )... )
    {
    }

    // The default active member is VariadicUnion<>,
    // so this recursively constructs `rest` until it
    // constructs the head.
    constexpr VariadicUnion() : m_rest()
    {
    }

    constexpr ~VariadicUnion()
    {
    }

    template<std::size_t I, std::size_t N = 0>
    constexpr void assign( const auto& value )
    {
      if constexpr ( I == N )
      {
        m_value = value;
      }
      else
      {
        m_rest.template assign<I, N + 1>( value );
      }
    }

    template<std::size_t I, std::size_t N = 0>
    constexpr auto& get()
    {
      if constexpr ( I == N )
      {
        return m_value;
      }
      else
      {
        return m_rest.template get<I, N + 1>();
      }
    }

    template<std::size_t I, std::size_t N = 0>
    constexpr const auto& get() const
    {
      if constexpr ( I == N )
      {
        return m_value;
      }
      else
      {
        return m_rest.template get<I, N + 1>();
      }
    }

  };

  template<typename... Ts>
  class Variant
  {
  public:

    constexpr Variant( const is_convertible_to_any_of<Ts...> auto& value )
    {
      assign( value );
    }

    constexpr ~Variant()
    {
      destruct();
    }

    constexpr Variant& operator=( const is_convertible_to_any_of<Ts...> auto& value )
    {
      destruct();

      assign( value );
      return *this;
    }

    template<is_any_of<Ts...> T>
    constexpr auto& get()
    {
      constexpr std::size_t index = variant_index_v<T, Variant>;
      return std::get<index>( m_union );
    }

    template<is_any_of<Ts...> T>
    constexpr const auto& get() const
    {
      constexpr std::size_t index = variant_index_v<T, Variant>;
      return std::get<index>( m_union );
    }

    template<std::size_t I>
    constexpr auto& get()
    {
      return get<variant_alternative_t<I, Variant>>();
    }

    template<std::size_t I>
    constexpr const auto& get() const
    {
      return get<variant_alternative_t<I, Variant>>();
    }

    constexpr std::size_t index() const { return m_typeIndex; }

  private:

    static constexpr std::size_t invalid_variant = sizeof...( Ts );

    constexpr void assign( const is_convertible_to_any_of<Ts...> auto& value )
    {
      constexpr std::size_t index = value_to_index<decltype( value )>();

      if ( m_typeIndex == index )
      {
        // The union is already initialised with this type
        m_typeIndex = index;
        m_union.template assign<index>( value );
      }
      else
      {
        // Construct in-place
        m_typeIndex = index;
        construct_at( &m_union, in_place_index<index>, value );
      }
    }

    constexpr void destruct()
    {
      if ( !std::is_constant_evaluated() )
      {
        destroy_alternative( m_typeIndex );
      }
    }

    template<std::size_t... Is>
    static consteval auto get_overload_index_table( std::index_sequence<Is...> )
    {
      return Overload {
        []( const variant_alternative_t<Is, Variant>& value )
        {
          using type = variant_alternative_t<Is, Variant>;
          constexpr std::size_t index = variant_index_v<type, Variant>;
          return std::integral_constant<std::size_t, index>();
        }...
      };
    };

    template<typename T>
    static consteval std::size_t value_to_index()
    {
      constexpr auto overload_index = get_overload_index_table( std::index_sequence_for<Ts...>() );
      using index = decltype( overload_index( std::declval<T>() ) );
      return index();
    }

    using variant_destructor = void (*)( Variant& );

    template<std::size_t I>
    static consteval variant_destructor make_destructor()
    {
      using type = variant_alternative_t<I, Variant>;

      return []( Variant& variant )
      {
        if constexpr ( !std::is_trivially_destructible_v<type> && std::is_destructible_v<type> )
        {
          auto& object = std::get<I>( variant );
          destroy_at( &object );
        }
      };
    }

    template<std::size_t... Is>
    static consteval auto make_destructors( std::index_sequence<Is...> )
    {
      return make_array( make_destructor<Is>()... );
    };

    static consteval auto make_destructors()
    {
      using indices = std::index_sequence_for<Ts...>;
      return make_destructors( indices() );
    };

    constexpr void destroy_alternative( std::size_t index )
    {
      constexpr auto destructors = make_destructors();
      variant_destructor destructor = destructors[m_typeIndex];
      destructor( *this );
    }

    VariadicUnion<Ts...> m_union = {};
    std::size_t m_typeIndex = invalid_variant;

  };

  namespace detail
  {

    // Base case
    template<typename VisitorType, typename... VariantTypes, std::size_t... Indices>
    consteval auto* create_dispatch_matrix( std::index_sequence<Indices...> )
    {
      return +[]( VisitorType visitor, VariantTypes... variants )
      {
        return visitor( std::get<Indices>( variants )... );
      };
    };

    template<typename VisitorType, typename... VariantTypes,
            std::size_t... Is, std::size_t... Js, typename... Ks>
    consteval auto create_dispatch_matrix( std::index_sequence<Is...>, std::index_sequence<Js...>, Ks... ks)
    {
      return make_array(
        create_dispatch_matrix<VisitorType, VariantTypes...>(
          std::index_sequence<Is..., Js>(),
          ks...
        )...
      );
    };

    // Creates an N-dimensional matrix of visitor functions
    template<typename VisitorType, typename... VariantTypes>
    consteval auto get_dispatch_matrix()
    {
      return create_dispatch_matrix<VisitorType, VariantTypes...>(
        std::index_sequence<>(),
        std::make_index_sequence< variant_size_v<std::decay_t<VariantTypes>> >()...
      );
    }

    // Accesses the N-dimensional visitor matrix through recursion
    template<typename T>
    constexpr T&& at_impl( T&& element )
    {
      return forward<T>( element );
    }

    // Recursively expands indices to access the element
    template<typename T, typename... Indices>
    constexpr auto&& at_impl( T&& elements, std::size_t i, Indices... indices )
    {
      return at_impl( forward<T>( elements )[i], indices... );
    }

    template<typename T, typename... Indices>
    constexpr auto&& at( T&& elements, Indices... indices )
    {
      return at_impl( forward<T>( elements ), indices... );
    }

  }

  template<typename T, typename... Ts>
  constexpr bool holds_alternative( const ql::Variant<Ts...>& variant )
  {
    using variant_type = std::remove_cvref_t<decltype( variant )>;
    return variant_index_v<T, variant_type> == variant.index();
  }

  template<typename VisitorType, typename... VariantTypes>
  constexpr auto visit( VisitorType&& f, VariantTypes&&... variants )
  {
    constexpr auto dispatchMatrix = detail::get_dispatch_matrix<VisitorType&&, VariantTypes&&...>();
    auto* dispatch = detail::at( dispatchMatrix, variants.index()... );
    return dispatch( forward<VisitorType>( f ), forward<VariantTypes>( variants )... );
  }

  template<typename Variant, typename... VisitorTypes>
  constexpr auto match( Variant&& variant, VisitorTypes&&... visitors )
  {
    return visit( Overload { visitors... }, variant );
  }

}

namespace std
{

  template<typename... Ts>
  struct variant_size<ql::Variant<Ts...>> : ql::variant_size<ql::Variant<Ts...>>
  {
  };

  template<std::size_t I, typename... Ts>
  struct variant_alternative<I, ql::Variant<Ts...>> : ql::variant_alternative<I, ql::Variant<Ts...>>
  {
  };

  template<typename T, typename... Ts>
  constexpr auto& get( ql::Variant<Ts...>& variant )
  {
    return variant.template get<T>();
  }

  template<typename T, typename... Ts>
  constexpr const auto& get( const ql::Variant<Ts...>& variant )
  {
    return variant.template get<T>();
  }

  template<std::size_t I, typename... Ts>
  constexpr auto& get( ql::Variant<Ts...>& variant )
  {
    return variant.template get<I>();
  }

  template<std::size_t I, typename... Ts>
  constexpr const auto& get( const ql::Variant<Ts...>& variant )
  {
    return variant.template get<I>();
  }

  template<std::size_t I, typename... Ts>
  constexpr auto& get( ql::VariadicUnion<Ts...>& variant )
  {
    return variant.template get<I>();
  }

  template<std::size_t I, typename... Ts>
  constexpr const auto& get( const ql::VariadicUnion<Ts...>& variant )
  {
    return variant.template get<I>();
  }

  template<typename T, typename... Ts>
  constexpr bool holds_alternative( const ql::Variant<Ts...>& variant )
  {
    return ql::holds_alternative<T>( variant );
  }

}
