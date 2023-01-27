#pragma once
#include "common/functional.hpp"
#include <concepts>
#include <pthread.h>

namespace ql
{

class Thread
{
public:

  static constexpr int invalid_thread = -1;

  Thread() = default;

  Thread( std::invocable auto callable ) { assign( callable ); }

  ~Thread()
  {
    if ( m_thread != invalid_thread )
      join();
  }

  void join()
  {
    pthread_join( m_thread, nullptr );
  }

  void detach()
  {
    pthread_detach( m_thread );
    m_thread = invalid_thread;
  }

  Thread& operator=( std::invocable auto callable )
  {
    join();
    assign( callable );
    return *this;
  }

private:

  void assign( std::invocable auto callable )
  {
    m_function = callable;
    pthread_create( &m_thread, nullptr, &execute_thread, this );
  }

  static void* execute_thread( void* arguments )
  {
    Thread* thread = reinterpret_cast<Thread*>( arguments );
    thread->m_function();
    pthread_exit( nullptr );
  }

  Function<void()> m_function;
  pthread_t        m_thread = invalid_thread;
};

} // namespace ql
