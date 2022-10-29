#pragma once
#include "common/functional.hpp"
#include "common/win32/win32.hpp"
#include <concepts>
#include <processthreadsapi.h>

namespace ql
{

class Thread
{
public:

  Thread() = default;

  Thread( std::invocable auto callable ) { assign( callable ); }

  ~Thread() { join(); }

  void join()
  {
    DWORD exitCode;
    do
    {
      if ( GetExitCodeThread( m_thread, &exitCode ) == false )
      {
        char* error = detail::win32::get_error();
        if ( error != nullptr )
        {
          std::fprintf( stderr, "%s\n", error );
        }

        break;
      }

      Sleep( 10 );
    } while ( exitCode != STILL_ACTIVE )
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
    m_thread = CreateThread( nullptr, 0, &execute_thread, this, 0, m_threadId );
  }

  static void* execute_thread( void* arguments )
  {
    Thread* thread = reinterpret_cast<Thread*>( arguments );
    thread->m_function();
    ExitThread( 0 );
  }

  Function<void()> m_function;
  LPDWORD          m_threadId;
  HANDLE           m_thread;
};

} // namespace ql
