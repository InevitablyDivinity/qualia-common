#pragma once
#include "common/functional.hpp"
#include <concepts>
#include <pthread.h>
#include <sched.h>

namespace ql
{

class Thread
{
public:

  static constexpr int invalid_thread = -1;

  Thread() = default;

  Thread( std::invocable auto callable ) { assign( callable ); }

  static Thread current()
  {
    return Thread( pthread_self() );
  }

  ~Thread()
  {
    join();
  }

  void join()
  {
    if ( m_thread != invalid_thread )
      pthread_join( m_thread, nullptr );
  }

  void detach()
  {
    pthread_detach( m_thread );
    m_thread = invalid_thread;
  }

  void yield()
  {
    sched_yield();
  }

  Thread& operator=( std::invocable auto callable )
  {
    join();
    assign( callable );
    return *this;
  }

private:

  Thread( pthread_t thread )
  : m_thread( thread )
  {
  }

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
