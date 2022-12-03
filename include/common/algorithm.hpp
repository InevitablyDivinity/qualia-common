#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include "utility.hpp"

namespace ql
{

constexpr auto min( const auto& lhs, const auto& rhs )
{
  return lhs > rhs ? rhs : lhs;
}

constexpr auto max( const auto& lhs, const auto& rhs )
{
  return lhs < rhs ? rhs : lhs;
}

constexpr auto clamp( const auto& value, const auto& min, const auto& max )
{
  if ( value < min )
    return min;
  else if ( value > max )
    return max;
  else
    return value;
}

constexpr void swap( auto& lhs, auto& rhs )
{
  auto tmp = move( lhs );
  lhs      = move( rhs );
  rhs      = move( tmp );
}

template<typename InputIterator, typename OutputIterator>
constexpr OutputIterator copy( InputIterator first, InputIterator last, OutputIterator out )
{
  while ( first != last )
  {
    *out++ = *first++;
  }

  return out;
}

template<typename InputIterator, typename OutputIterator>
constexpr OutputIterator copy_n( InputIterator input, std::size_t count, OutputIterator out )
{
  return copy( input, input + count, out );
}

template<typename InputIterator, typename OutputIterator>
constexpr OutputIterator move( InputIterator first, InputIterator last, OutputIterator out )
{
  while ( first != last )
  {
    *out++ = move( *first++ );
  }

  return out;
}

template<typename InputIterator, typename OutputIterator>
constexpr OutputIterator move_n( InputIterator input, std::size_t count, OutputIterator out )
{
  return move( input, input + count, out );
}

template<typename ForwardIterator, typename T>
constexpr void fill( ForwardIterator first, ForwardIterator last, const T& value )
{
  while ( first != last )
  {
    *first = value;
    first++;
  }
}

template<typename ForwardIterator, typename T>
constexpr void fill_n( ForwardIterator input, std::size_t count, const T& value )
{
  fill( input, input + count, value );
}

inline std::uint64_t fnv1a_hash( const void* data, std::size_t length )
{
  const std::uint64_t FNV_prime        = 0x100000001B3;
  const std::uint64_t FNV_offset_basis = 0xCBF29CE484222325;
  std::uint64_t       hash             = FNV_offset_basis;

  const auto* bytes = reinterpret_cast<const uint8_t*>( data );
  for ( auto i = 0; i < length; i++ )
  {
    hash *= FNV_prime;
    hash ^= bytes[ i ];
  }

  return hash;
}

template<typename T>
struct hash;

template<typename T>
struct hash : std::hash<T>
{
};

} // namespace ql
