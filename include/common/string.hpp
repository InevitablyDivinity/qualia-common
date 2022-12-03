#pragma once
#include "common/algorithm.hpp"
#include "common/memory.hpp"
#include <cstddef>
#include <string>

namespace ql
{

class String
{
public:

  using iterator       = char*;
  using const_iterator = const char*;

  constexpr String() = default;

  constexpr String( const char* src )
  {
    assign( src, std::char_traits<char>::length( src ) );
  }

  constexpr String( const char* src, std::size_t size ) { assign( src, size ); }

  constexpr String( const String& src ) { assign( src ); }

  constexpr String( String&& src ) { assign( src ); }

  constexpr ~String()
  {
    destruct();
  }

  constexpr String& operator=( const char* rhs )
  {
    destruct();

    assign( rhs );
    return *this;
  }

  constexpr String& operator=( const String& rhs )
  {
    destruct();

    assign( rhs );
    return *this;
  }

  constexpr String& operator=( String&& rhs )
  {
    destruct();

    assign( rhs );
    return *this;
  }

  constexpr bool operator==( const String& rhs ) const
  {
    if ( rhs.m_size != m_size )
      return false;

    return std::equal( begin(), end(), rhs.begin() );
  }

  constexpr bool operator==( const char* rhs ) const
  {
    return std::equal( begin(), end(), rhs );
  }

  constexpr operator const char*() const { return m_data; }

  constexpr char*       data() { return m_data; }
  constexpr const char* data() const { return m_data; }
  constexpr const char* c_str() const { return m_data; }

  constexpr std::size_t size() const { return m_size; }
  constexpr bool        empty() const { return m_size > 0; }

  constexpr iterator       begin() { return m_data; }
  constexpr iterator       end() { return nullptr; }
  constexpr const_iterator begin() const { return m_data; }
  constexpr const_iterator cbegin() const { return m_data; }
  constexpr const_iterator end() const { return nullptr; }
  constexpr const_iterator cend() const { return nullptr; }

  constexpr void clear()
  {
    destruct();
  }

private:

  constexpr bool using_ssbo() const
  {
    return m_size + 1 <= alignof( std::max_align_t );
  }

  constexpr void assign( const char* src, std::size_t size )
  {
    if ( src == nullptr )
      return;

    if ( std::is_constant_evaluated() )
    {
      m_data = const_cast<char*>( src );
      m_size = size;
    }
    else
    {
      if ( size + 1 > alignof( std::max_align_t ) )
        m_data = new char[ size + 1 ];
      else
        m_data = m_stackBuffer;

      copy_n( src, size, m_data );
      m_data[ size ] = '\0';
      m_size         = size;
    }
  }

  constexpr void assign( const String& src )
  {
    assign( src.m_data, src.m_size );
  }

  constexpr void assign( String&& src )
  {
    swap( m_data, src.m_data );
    swap( m_size, src.m_size );
  }

  constexpr void destruct()
  {
    if ( empty() || std::is_constant_evaluated() )
      return;

    if ( using_ssbo() )
    {
      uninitialized_fill_n( m_data, sizeof( std::max_align_t ), 0 );
      //fill_memory( m_data, 0, sizeof( std::max_align_t ) );
    }
    else
    {
      delete[] m_data;
    }

    m_size = 0;
    m_data = nullptr;
  }

  char m_stackBuffer[alignof( std::max_align_t )] = {};
  char*       m_data = nullptr;
  std::size_t m_size = 0;
};

/*template<typename Type>
std::uint64_t hash(const Type &value);

template <>
std::uint64_t hash<String>(const String &value)
{
  return fnv1a_hash(value.data(), value.size());
}*/

template<typename Type>
struct hash;

template<>
struct hash<String>
{
  std::size_t operator()( const String& value )
  {
    return fnv1a_hash( value.data(), value.size() );
  }
};

} // namespace ql
