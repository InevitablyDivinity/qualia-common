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
    *out = *first;
    ++first, ++out;
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

/*template<typename Type>
using HashFunction = std::uint64_t (const Type &);

template<typename Type>
struct hash;

template<typename Type>
  requires std::has_unique_object_representations_v<Type>
std::uint64_t hash(const Type &value)
{
  return fnv1a_hash(&value, sizeof(Type));
}*/

template<typename Type>
struct hash : std::hash<Type>
{
};

} // namespace ql
