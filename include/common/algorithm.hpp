#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <type_traits>

namespace ql
{

auto min( auto lhs, auto rhs )
{
  return lhs > rhs ? rhs : lhs;
}

auto max( auto lhs, auto rhs )
{
  return lhs < rhs ? rhs : lhs;
}

auto clamp( auto value, auto min, auto max )
{
  if ( value < min )
    return min;
  else if ( value > max )
    return max;
  else
    return value;
}

void swap( auto& lhs, auto& rhs )
{
  auto tmp = lhs;
  lhs      = rhs;
  rhs      = tmp;
}

template<typename Input, typename Output>
Output copy( Input first, Input last, Output output )
{
  using type = decltype( output );
  if constexpr ( std::is_trivially_copyable_v<type> )
  {
    std::memcpy( reinterpret_cast<void*>( output ), reinterpret_cast<const void*>( first ),
                 std::uintptr_t( last ) - std::uintptr_t( first ) );

    return output + ( last - first );
  }
  else
  {
    Output head = output;
    for ( Input i = first; i != last; i++, head++ )
      *head = *i;

    return head;
  }
}

template<typename Input, typename Output>
Output copy_n( Input input, std::size_t count, Output output )
{
  return ql::copy( input, input + count, output );
}

std::uint64_t fnv1a_hash( const void* data, std::size_t length )
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
