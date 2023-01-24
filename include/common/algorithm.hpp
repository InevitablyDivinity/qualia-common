#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <memory>
#include "common/utility.hpp"
#include "common/memory.hpp"

namespace ql
{

using std::addressof;

template<typename T, typename... Args>
constexpr T* construct_at( T* ptr, Args&&... args ) noexcept
{
  return std::construct_at( ptr, forward<Args>( args )... );
}

template<typename T>
constexpr void destroy_at( T* object )
{
  if constexpr ( std::is_array_v<T> )
  {
    destroy( std::begin( *object ), std::end( *object ) );
  }
  else
  {
    object->~T();
  }
}

template<typename ForwardIterator>
constexpr void destroy( ForwardIterator first, ForwardIterator last )
{
  while ( first != last )
  {
    destroy_at( addressof( *first ) );
    first++;
  }
}

template<typename ForwardIterator>
constexpr void destroy_n( ForwardIterator first, std::size_t count )
{
  destroy( first, first + count );
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

template<typename InputIterator, typename OutputIterator>
constexpr OutputIterator uninitialized_copy( InputIterator first, InputIterator last, OutputIterator out )
{
  while ( first != last )
  {
    construct_at( out, *first );
    first++, out++;
  }

  return out;
}

template<typename InputIterator, typename OutputIterator>
constexpr OutputIterator uninitialized_copy_n( InputIterator first, std::size_t count, OutputIterator out )
{
  return uninitialized_copy( first, first + count, out );
}

template<typename InputIterator, typename OutputIterator>
constexpr OutputIterator uninitialized_move( InputIterator first, InputIterator last, OutputIterator out )
{
  while ( first != last )
  {
    construct_at( out, move( *first ) );
    first++, out++;
  }

  return out;
}

template<typename InputIterator, typename OutputIterator>
constexpr OutputIterator uninitialized_move_n( InputIterator input, std::size_t count, OutputIterator out )
{
  return uninitialized_move( input, input + count, out );
}

template<typename ForwardIterator, typename T>
constexpr void uninitialized_fill( ForwardIterator first, ForwardIterator last, const T& value )
{
  while ( first != last )
  {
    construct_at( first, value );
    first++;
  }
}

template<typename ForwardIterator, typename T>
constexpr void uninitialized_fill_n( ForwardIterator input, std::size_t count, const T& value )
{
  uninitialized_fill( input, input + count, value );
}

template<typename ForwardIterator>
constexpr void uninitialized_default_construct( ForwardIterator first, ForwardIterator last )
{
  while ( first != last )
  {
    construct_at( first );
    first++;
  }
}

template<typename ForwardIterator>
constexpr void uninitialized_default_construct_n( ForwardIterator input, std::size_t count )
{
  uninitialized_default_construct( input, input + count );
}

template<typename ForwardIterator, typename Compare>
constexpr ForwardIterator min_element( ForwardIterator first, ForwardIterator last, Compare compare )
{
  if ( first == last )
    return last;

  auto lhs = first, rhs = ++lhs;
  while ( rhs != last )
  {
    lhs = compare( *lhs, *rhs ) ? lhs : rhs;
    lhs++, rhs++;
  }

  return lhs;
}

template<typename ForwardIterator>
constexpr ForwardIterator min_element( ForwardIterator first, ForwardIterator last )
{
  if ( first == last )
    return last;

  auto lhs = first, rhs = ++lhs;
  while ( rhs != last )
  {
    lhs = *lhs < *rhs ? lhs : rhs;
    lhs++, rhs++;
  }

  return lhs;
}

constexpr auto min( const auto& lhs, const auto& rhs )
{
  return lhs < rhs ? lhs : rhs;
}

template<typename Compare>
constexpr auto min( const auto& lhs, const auto& rhs, Compare compare )
{
  return compare( lhs, rhs ) ? lhs : rhs;
}

template<typename T>
constexpr auto min( std::initializer_list<T> list )
{
  return *min_element( list.begin(), list.end() );
}

template<typename T, typename Compare>
constexpr auto min( std::initializer_list<T> list, Compare compare )
{
  return *min_element( list.begin(), list.end(), compare );
}


template<typename ForwardIterator, typename Compare>
constexpr ForwardIterator max_element( ForwardIterator first, ForwardIterator last, Compare compare )
{
  if ( first == last )
    return last;

  auto lhs = first, rhs = ++lhs;
  while ( rhs != last )
  {
    lhs = compare( *lhs, *rhs ) ? lhs : rhs;
    lhs++, rhs++;
  }

  return lhs;
}

template<typename ForwardIterator>
constexpr ForwardIterator max_element( ForwardIterator first, ForwardIterator last )
{
  if ( first == last )
    return last;

  auto lhs = first, rhs = ++lhs;
  while ( rhs != last )
  {
    lhs = *lhs > *rhs ? lhs : rhs;
    lhs++, rhs++;
  }

  return lhs;
}

constexpr auto max( const auto& lhs, const auto& rhs )
{
  return lhs > rhs ? lhs : rhs;
}

template<typename Compare>
constexpr auto max( const auto& lhs, const auto& rhs, Compare compare )
{
  return compare( lhs, rhs ) ? lhs : rhs;
}

template<typename T>
constexpr auto max( std::initializer_list<T> list )
{
  return *max_element( list.begin(), list.end() );
}

template<typename T, typename Compare>
constexpr auto max( std::initializer_list<T> list, Compare compare )
{
  return *max_element( list.begin(), list.end(), compare );
}

constexpr auto& clamp( const auto& value, const auto& min, const auto& max )
{
  if ( value < min )
    return min;
  else if ( value > max )
    return max;
  else
    return value;
}

template<typename Compare>
constexpr auto& clamp( const auto& value, const auto& min, const auto& max, Compare compare )
{
  if ( compare( value, min ) )
    return min;
  else if ( compare( max, value ) )
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

template<std::size_t N>
constexpr void swap( auto ( &lhs )[N], auto ( &rhs )[N] )
{
  for ( std::size_t i = 0; i < N; i++ )
    swap( lhs[i], rhs[i] );
}

template<typename ForwardIterator>
constexpr bool equal( ForwardIterator first1, ForwardIterator last1, ForwardIterator first2 )
{
  while ( first1 != last1 )
  {
    if ( *first1++ != *first2++ )
      return false;
  }

  return true;
}

inline std::uint64_t fnv1a_hash( const void* data, std::size_t length )
{
  const std::uint64_t FNV_prime        = 0x100000001B3;
  const std::uint64_t FNV_offset_basis = 0xCBF29CE484222325;
  std::uint64_t       hash             = FNV_offset_basis;

  const auto* bytes = reinterpret_cast<const uint8_t*>( data );
  for ( std::size_t i = 0; i < length; i++ )
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
