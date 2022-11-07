#pragma once
#include <windows.h>

namespace ql::detail::win32
{

char* get_error()
{
  char* message = nullptr;
  DWORD error   = GetLastError();

  if ( error != NULL )
  {
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, error, 0, (LPTSTR)&message, 0, nullptr );
  }

  return message;
}

} // namespace ql::detail::win32
