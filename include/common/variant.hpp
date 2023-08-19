#pragma once
#include "common/common.hpp"
#include "common/types.hpp"
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
    FORCEINLINE constexpr VariadicUnion( in_place_index_t<0>, Args&&... args )
    : m_value( forward<Args>( args )... )
    {
    }

    // Recursive in-place constructor
    template<std::size_t I, typename... Args>
    FORCEINLINE constexpr VariadicUnion( in_place_index_t<I>, Args&&... args )
    : m_rest( in_place_index<I - 1>, forward<Args>( args )... )
    {
    }

    // The default active member is VariadicUnion<>,
    // so this recursively constructs `rest` until it
    // constructs the head.
    FORCEINLINE constexpr VariadicUnion() : m_rest {}
    {
    }

    FORCEINLINE constexpr ~VariadicUnion()
    {
    }

    template<std::size_t I, std::size_t N = 0>
    FORCEINLINE constexpr void assign( const auto& value )
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
    FORCEINLINE constexpr auto& get()
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
    FORCEINLINE constexpr const auto& get() const
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

    template<typename T>
    FORCEINLINE constexpr Variant( T&& value )
    {
      constexpr std::size_t index = type_index_for_overload_selection_v<T, Ts...>;
      construct<index>( forward<T>( value ) );
    }

    FORCEINLINE constexpr ~Variant()
    {
      destruct();
    }

    template<typename T>
    FORCEINLINE constexpr Variant& operator=( T&& value )
    {
      destruct();

      assign( forward<T>( value ) );
      return *this;
    }

    template<is_any_of<Ts...> T>
    FORCEINLINE constexpr auto& get()
    {
      return std::get<variant_index_v<T, Variant>>( m_union );
    }

    template<is_any_of<Ts...> T>
    FORCEINLINE constexpr const auto& get() const
    {
      return std::get<variant_index_v<T, Variant>>( m_union );
    }

    template<std::size_t I>
    FORCEINLINE constexpr auto& get()
    {
      return get<variant_alternative_t<I, Variant>>();
    }

    template<std::size_t I>
    FORCEINLINE constexpr const auto& get() const
    {
      return get<variant_alternative_t<I, Variant>>();
    }

    FORCEINLINE constexpr std::size_t index() const { return m_typeIndex; }

  private:

    static constexpr std::size_t invalid_variant = sizeof...( Ts );

    template<std::size_t I, typename T>
    inline constexpr void construct( T&& value )
    {
      // Construct in-place
      m_typeIndex = I;
      construct_at( &m_union, in_place_index<I>, forward<T>( value ) );
    }

    template<std::size_t I, typename T>
    inline constexpr void assign( T&& value )
    {
      m_typeIndex = I;
      m_union.template assign<I>( forward<T>( value ) );
    }

    template<typename T>
    inline constexpr void assign( T&& value )
    {
      constexpr std::size_t index = type_index_for_overload_selection_v<T, Ts...>;

      if ( m_typeIndex == index )
      {
        // The union is already initialised with this type
        assign<index>( forward<T>( value ) );
      }
      else
      {
        // Construct in-place
        construct<index>( forward<T>( value ) );
      }
    }

    inline constexpr void destruct()
    {
      if ( m_typeIndex != invalid_variant )
      {
        using variant_destructor = void (*)( Variant& );
        variant_destructor destructor = destructors[m_typeIndex];
        destructor( *this );
      }
    }

    template<std::size_t... Is>
    static consteval auto make_destructors( std::index_sequence<Is...> )
    {
      return ql::Array {
        +[]( Variant& variant )
        {
          if constexpr ( not std::is_trivially_destructible_v<variant_alternative_t<Is, Variant>>
                         and std::destructible<variant_alternative_t<Is, Variant>> )
          {
            destroy_at( addressof( std::get<Is>( variant ) ) );
          }
        }...
      };
    };

    static constexpr ql::Array destructors = make_destructors( std::index_sequence_for<Ts...> {} );

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
      return ql::Array {
        create_dispatch_matrix<VisitorType, VariantTypes...>(
          std::index_sequence<Is..., Js>(),
          ks...
        )...
      };
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
  FORCEINLINE constexpr bool holds_alternative( const ql::Variant<Ts...>& variant )
  {
    return variant_index_v<T, ql::Variant<Ts...>> == variant.index();
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
  FORCEINLINE constexpr auto& get( ql::Variant<Ts...>& variant )
  {
    return variant.template get<T>();
  }

  template<typename T, typename... Ts>
  FORCEINLINE constexpr const auto& get( const ql::Variant<Ts...>& variant )
  {
    return variant.template get<T>();
  }

  template<std::size_t I, typename... Ts>
  FORCEINLINE constexpr auto& get( ql::Variant<Ts...>& variant )
  {
    return variant.template get<I>();
  }

  template<std::size_t I, typename... Ts>
  FORCEINLINE constexpr const auto& get( const ql::Variant<Ts...>& variant )
  {
    return variant.template get<I>();
  }

  template<std::size_t I, typename... Ts>
  FORCEINLINE constexpr auto& get( ql::VariadicUnion<Ts...>& variant )
  {
    return variant.template get<I>();
  }

  template<std::size_t I, typename... Ts>
  FORCEINLINE constexpr const auto& get( const ql::VariadicUnion<Ts...>& variant )
  {
    return variant.template get<I>();
  }

  template<typename T, typename... Ts>
  FORCEINLINE constexpr bool holds_alternative( const ql::Variant<Ts...>& variant )
  {
    return ql::holds_alternative<T>( variant );
  }

}
