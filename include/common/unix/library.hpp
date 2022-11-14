#pragma once
#include "common/string.hpp"
#include <cstdio>
#include <dlfcn.h>

namespace ql
{

class Library
{
public:

  Library() = default;

  Library( const char* path ) { load( path ); }

  Library( Library&& library )
    : m_name( library.m_name ), m_library( library.m_library )
  {
    library.m_name    = nullptr;
    library.m_library = nullptr;
  }

  Library( const Library& ) = delete;

  ~Library()
  {
    if ( m_library != nullptr )
      destruct();
  }

  Library& operator=( Library&& other )
  {
    if ( m_library != nullptr )
      destruct();

    ql::swap( m_name, other.m_name );
    ql::swap( m_library, other.m_library );

    return *this;
  }

  String name() const { return m_name; }

  bool is_loaded() const { return m_library != nullptr; }

  bool load( const char* path )
  {
    if ( is_loaded() )
      destruct();

    m_library = dlopen( path, RTLD_NOW );

    char* error = dlerror();
    if ( error != nullptr )
    {
      std::fprintf( stderr, "%s\n", error );
    }

    if ( m_library )
    {
      m_name = path;
      return true;
    }
    else
    {
      return false;
    }
  }

  template<typename T>
  T* get( const char* symbol )
  {
    return reinterpret_cast<T*>( find_symbol( symbol ) );
  }

private:

  void destruct()
  {
    dlclose( m_library );
    m_library = nullptr;
    m_name    = nullptr;
  }

  inline void* find_symbol( const char* symbol )
  {
    return dlsym( m_library, symbol );
  }

  String m_name    = nullptr;
  void*  m_library = nullptr;
};

String decorate_library_name( const char* filename )
{
  char buf[ 50 ];
  std::snprintf( buf, sizeof( buf ), "lib%s.so", filename );
  return String( buf );
}

} // namespace ql
