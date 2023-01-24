#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>
#include "common/memory.hpp"
#include "common/types.hpp"

namespace ql
{

template<typename F>
class Function;

template<typename R, typename... Args>
class Function<R( Args... )>
{
public:

  using return_type = R;
  using argument_types = parameter_pack<Args...>;
  using pointer = R (*)( Args... );

  constexpr Function() = default;

  constexpr Function( auto&& f )
  {
    assign( f );
  }

  constexpr Function( std::nullptr_t )
  {
    assign( std::nullptr_t() );
  }

  constexpr ~Function()
  {
    destruct();
  }

  constexpr Function& operator=( auto&& f )
  {
    destruct();
    assign( f );
    return *this;
  }

  constexpr Function operator=( std::nullptr_t )
  {
    destruct();
    assign( std::nullptr_t() );
    return *this;
  }

  constexpr R operator()( Args&&... args ) const
  {
    if ( m_isFunctionPtr )
    {
      return m_ptr( forward<Args>( args )... );
    }
    else
    {
      return m_callable->operator()( forward<Args>( args )... );
    }
  }

  constexpr pointer target() const { return m_callable->target(); }

private:

  constexpr void assign( auto&& f )
  {
    using type = std::remove_cvref_t<decltype( f )>;

    if constexpr ( std::is_pointer_v<type> || std::is_convertible_v<type, pointer> )
    {
      m_isFunctionPtr = true;
      m_ptr = f;
    }
    else
    {
      m_isFunctionPtr = false;

      if constexpr ( sizeof( type ) > sizeof( m_stackBuffer ) )
      {
        m_callable = new Callable<type>( f );
      }
      else
      {
        m_callable = new ( m_stackBuffer ) Callable<type>( f );
      }
    }
  }

  constexpr void assign( std::nullptr_t )
  {
    m_callable = nullptr;
  }

  constexpr void destruct()
  {
    if ( !m_isFunctionPtr )
    {
      if ( m_callable->size() > sizeof( std::max_align_t ) )
      {
        delete m_callable;
      }
      else
      {
        destroy_at( m_callable );
      }
    }
  }

  class ICallable
  {
  public:

    constexpr virtual ~ICallable() = default;
    constexpr virtual R operator()( Args&&... args ) = 0;
    constexpr virtual std::size_t size() const = 0;
    constexpr virtual pointer target() const = 0;

  };

  template<typename F>
  class Callable : public ICallable
  {
  public:

    constexpr Callable( F&& callable )
    : m_callable( callable )
    {
    }

    constexpr virtual R operator()( Args&&... args )
    {
      return m_callable( forward<Args>( args )... );
    }

    constexpr virtual std::size_t size() const
    {
      return sizeof( F );
    }

    constexpr virtual pointer target() const
    {
      if constexpr ( std::is_convertible_v<F, pointer> )
      {
        return &m_callable;
      }
      else
      {
        return nullptr;
      }
    }

  private:

    F m_callable;
  };

  byte_t m_stackBuffer[alignof( std::max_align_t )] = {};

  union
  {
    R ( *m_ptr )( Args... ) = nullptr;
    ICallable* m_callable;
  };

  bool m_isFunctionPtr = false;

};

template<typename R, typename... Args>
Function( R (*)( Args... ) ) -> Function<R( Args... )>;


template<typename T>
struct callable_type;

template<typename R, typename... Args>
struct callable_type<Function<R( Args... )>> : callable_type<R( Args... )>
{
};

} // namespace ql
