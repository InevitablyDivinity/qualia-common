#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>
#include "common/types.hpp"

namespace ql
{

template<typename T>
class Function;

template<typename R, typename... Args>
class Function<R( Args... )>
{
public:

  using return_type = R;
  using argument_types = parameter_pack<Args...>;

  Function() = default;

  Function( std::invocable auto callable ) { assign( callable ); }

  ~Function() { cleanup(); }

  Function& operator=( const std::invocable auto& callable )
  {
    assign( callable );
    return *this;
  }

  R operator()( Args... args )
  {
    return m_callable->call( std::forward( args )... );
  }

  operator bool() const { return m_callable != nullptr; }

private:

  void cleanup()
  {
    if ( m_callable )
    {
      // If we're using SSBO, then destroy - don't delete.
      if ( std::uintptr_t( m_callable ) == std::uintptr_t( &m_stackBuffer ) )
        m_callable->~ICallable();
      else
        delete m_callable;
    }
  }

  void assign( std::invocable auto callable )
  {
    cleanup();

    using InternalType = decltype( callable );
    using Wrapper      = Callable<InternalType>;

    if ( alignof( InternalType ) <= alignof( std::max_align_t ) )
      m_callable = new ( &m_stackBuffer ) Wrapper( std::move( callable ) );
    else
      m_callable = new Wrapper( std::move( callable ) );
  }

  // Use an abstract callable interface
  // to copy lambda states (captures)
  class ICallable
  {
  public:

    virtual ~ICallable()           = default;
    virtual R call( Args... args ) = 0;
  };

  template<typename T>
  class Callable : public ICallable
  {
  public:

    Callable( T callable ) : m_callable( std::move( callable ) ) {}

    virtual R call( Args... args ) { return m_callable( args... ); }

  private:

    T m_callable;
  };

  ICallable*       m_callable = nullptr;
  std::max_align_t m_stackBuffer;
};

void for_each( auto range, auto predicate )
{
  for ( auto& item : range )
    predicate( item );
}

auto find_if( auto range, auto predicate ) -> typename decltype( range )::type*
{
  for ( auto& item : range )
  {
    if ( predicate( item ) )
      return &item;
  }

  return nullptr;
}

} // namespace ql
