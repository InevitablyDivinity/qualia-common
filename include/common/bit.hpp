#pragma once
#include "common/utility.hpp"
#include <type_traits>

namespace ql
{

template<typename T>
class BitFlags
{
public:

  using type = T;
  using underlying_type = std::underlying_type_t<T>;

  BitFlags& operator=( const underlying_type& flags )
  {
    m_flags = flags;
    return *this;
  }

  bool operator==( const underlying_type& flags ) const
  {
    return m_flags == flags;
  }

  bool has( underlying_type flags ) const
  {
    return m_flags & flags;
  }

  bool has( type flags ) const
  {
    return has( to_underlying( flags ) );
  }

  void set( underlying_type flags )
  {
    m_flags |= flags;
  }

  void set( type flags )
  {
    set( to_underlying( flags ) );
  }

  operator underlying_type() const { return m_flags; }

private:

  underlying_type m_flags;

};

}
