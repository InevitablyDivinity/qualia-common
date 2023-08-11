#pragma once
#include "common/algorithm.hpp"
#include "common/memory.hpp"
#include "common/utility.hpp"
#include "common/allocator.hpp"
#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <algorithm>

namespace ql
{

template<typename T, typename Allocator = ql::Allocator<T>>
class Vector
{
public:

  using type            = T;
  using iterator        = type*;
  using const_iterator  = const type*;
  using reference       = type&;
  using const_reference = const type&;

  constexpr Vector( std::initializer_list<T> items );

  constexpr Vector( const Vector& other );
  constexpr Vector( Vector&& other );

  constexpr Vector( const T* items, std::size_t size );

  constexpr Vector( std::size_t size );

  template<std::size_t N>
  constexpr Vector( const T ( &items )[ N ] );

  constexpr Vector() = default;
  constexpr ~Vector();

  constexpr Vector& operator=( std::initializer_list<type> items );
  constexpr Vector& operator=( const Vector& rhs );
  constexpr Vector& operator=( Vector&& rhs );
  template<std::size_t N>
  constexpr Vector& operator=( const type ( *items )[ N ] );

  // Capacity
  constexpr bool        empty() const { return m_items == nullptr; }
  constexpr std::size_t size() const { return m_size; }
  constexpr std::size_t max_size() const { return SIZE_MAX; }
  constexpr void        reserve( std::size_t capacity );
  constexpr std::size_t capacity() const { return m_capacity; }
  constexpr void        shrink_to_fit();

  // Modifiers
  constexpr void clear() { resize( 0 ); }

  constexpr iterator insert( const_iterator pos, const T& value );
  constexpr iterator insert( const_iterator pos, T&& value );
  constexpr iterator insert( const_iterator pos, std::size_t count, const T& value );
  constexpr iterator insert( const_iterator pos, iterator first, iterator last );
  constexpr iterator insert( const_iterator pos, std::initializer_list<type> list );

  template<typename... Args>
  constexpr iterator emplace( const_iterator pos, Args&&... args );

  constexpr iterator erase( const_iterator pos );
  constexpr iterator erase( const_iterator first, const_iterator last );

  constexpr void push_back( const T& item );
  constexpr void push_back( T&& item );

  template<typename... Args>
  constexpr reference emplace_back( Args&&... args );

  constexpr void pop_back();
  constexpr void resize( std::size_t size );
  constexpr void swap( Vector& other );

  // Iterators
  constexpr iterator       begin() { return m_items; }
  constexpr const_iterator begin() const { return m_items; }
  constexpr const_iterator cbegin() const { return begin(); }

  constexpr iterator       end() { return m_items + m_size; }
  constexpr const_iterator end() const { return m_items + m_size; }
  constexpr const_iterator cend() const { return end(); }

  // Element access
  constexpr reference       at( std::size_t i ) { return m_items[ i ]; }
  constexpr const_reference at( std::size_t i ) const { return m_items[ i ]; }

  constexpr reference       operator[]( std::size_t i ) { return m_items[ i ]; }
  constexpr const_reference operator[]( std::size_t i ) const { return m_items[ i ]; }

  constexpr reference       front() { return m_items[ 0 ]; }
  constexpr const_reference front() const { return m_items[ 0 ]; }

  constexpr reference       back() { return m_items[ m_size - 1 ]; }
  constexpr const_reference back() const { return m_items[ m_size - 1 ]; }

  constexpr T*       data() { return m_items; }
  constexpr const T* data() const { return m_items; }

private:

  constexpr void assign( std::initializer_list<T> items );

  constexpr void assign( const Vector& other );
  constexpr void assign( Vector&& other );

  constexpr void assign( const T* items, std::size_t size );

  template<std::size_t N>
  constexpr void assign( const T ( &items )[ N ] );

  constexpr void destruct();

  Allocator m_allocator;

  T*          m_items    = nullptr;
  std::size_t m_size     = 0;
  std::size_t m_capacity = 0;
};

template<typename T, typename Allocator>
constexpr Vector<T, Allocator>::Vector( std::size_t size )
{
  resize( size );
}

template<typename T, typename Allocator>
template<std::size_t N>
constexpr Vector<T, Allocator>::Vector( const T ( &items )[ N ] )
{
  assign( items );
}

template<typename T, typename Allocator>
constexpr Vector<T, Allocator>::Vector( std::initializer_list<T> items )
{
  assign( items );
}

template<typename T, typename Allocator>
constexpr Vector<T, Allocator>::Vector( const Vector& other )
{
  assign( other );
}

template<typename T, typename Allocator>
constexpr Vector<T, Allocator>::Vector( Vector&& other )
{
  assign( move( other ) );
}

template<typename T, typename Allocator>
constexpr Vector<T, Allocator>::Vector( const T* items, std::size_t size )
{
  assign( items, size );
}

template<typename T, typename Allocator>
constexpr Vector<T, Allocator>::~Vector()
{
  destruct();
}

template<typename T, typename Allocator>
template<std::size_t N>
constexpr void Vector<T, Allocator>::assign( const type ( &items )[ N ] )
{
  assign( items, N );
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::assign( std::initializer_list<type> items )
{
  assign( items.begin(), items.size() );
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::assign( const Vector& other )
{
  assign( other.begin(), other.size() );
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::assign( Vector&& other )
{
  ql::swap( m_size, other.m_size );
  ql::swap( m_items, other.m_items );
  ql::swap( m_capacity, other.m_capacity );
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::assign( const type* items, std::size_t size )
{
  m_size = size;
  reserve( m_size );
  copy_n( items, m_size, m_items );
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::reserve( std::size_t capacity )
{
  if ( capacity > m_capacity )
  {
    auto result = m_allocator.allocate_at_least( capacity );

    if ( !empty() )
    {
      uninitialized_move( begin(), end(), result.ptr );
      m_allocator.deallocate( m_items, m_capacity );
    }

    m_capacity = result.size;
    m_items = result.ptr;
  }
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::shrink_to_fit()
{
  if ( m_capacity > m_size )
  {
    type* ptr = m_allocator.allocate( m_size );

    if ( !empty() )
    {
      uninitialized_move( begin(), end(), ptr );
      m_allocator.deallocate( m_items, m_capacity );
    }

    m_items = ptr;
    m_capacity = m_size;
  }
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::resize( std::size_t count )
{
  if ( count < m_size )
  {
    destroy_n( m_items, m_size - count );
    m_size = count;
    shrink_to_fit();
  }
  else if ( count > m_size )
  {
    reserve( count );
    uninitialized_default_construct_n( m_items + m_size, count - m_size );
    m_size = count;
  }
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::push_back( const type& item )
{
  m_size++;

  if ( size() > capacity() )
    reserve( capacity() + 1 );

  back() = item;
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::push_back( type&& item )
{
  m_size++;

  if ( size() > capacity() )
    reserve( m_capacity + 1 );

  back() = move( item );
}

template<typename T, typename Allocator>
template<std::size_t N>
constexpr Vector<T, Allocator>& Vector<T, Allocator>::operator=( const type ( *items )[ N ] )
{
  destruct();

  assign( items );
  return *this;
}

template<typename T, typename Allocator>
constexpr Vector<T, Allocator>& Vector<T, Allocator>::operator=( std::initializer_list<type> items )
{
  destruct();

  assign( items );
  return *this;
}

template<typename T, typename Allocator>
constexpr Vector<T, Allocator>& Vector<T, Allocator>::operator=( const Vector& rhs )
{
  if ( this == ql::addressof( rhs ) )
    return *this;

  destruct();

  assign( rhs );
  return *this;
}

template<typename T, typename Allocator>
constexpr Vector<T, Allocator>& Vector<T, Allocator>::operator=( Vector&& rhs )
{
  if ( this == ql::addressof( rhs ) )
    return *this;

  destruct();

  assign( rhs );
  return *this;
}

// inserts value before pos.
template<typename T, typename Allocator>
constexpr typename Vector<T, Allocator>::iterator Vector<T, Allocator>::insert( const_iterator pos, const T& value )
{
  if ( pos <= begin() || pos > end() )
    return end();

  m_size++;

  if ( size() > capacity() )
    reserve( capacity() + 1 );

  copy( pos, const_iterator( end() - pos ), iterator( pos + 1 ) );

  m_items[ m_size - 1 ] = type();
  return &m_items[ m_size - 1 ];
}

// inserts value before pos.
template<typename T, typename Allocator>
constexpr typename Vector<T, Allocator>::iterator Vector<T, Allocator>::insert( const_iterator pos, T&& value )
{
  if ( pos <= begin() || pos > end() )
    return end();

  m_size++;

  if ( size() > capacity() )
    reserve( capacity() + 1 );

  copy( pos, end() - pos, iterator( pos ) );

  m_items[ m_size - 1 ] = move( type() );
  return &m_items[ m_size - 1 ];
}

// inserts count copies of the value before pos.
template<typename T, typename Allocator>
constexpr typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::insert( const_iterator pos, std::size_t count, const T& value )
{
  for ( std::size_t i = count; i > 0; i-- )
    insert( std::prev( pos, i ), value );

  return pos - count;
}

template<typename T, typename Allocator>
constexpr typename Vector<T, Allocator>::iterator Vector<T, Allocator>::insert( const_iterator pos,
                                                iterator first, iterator last )
{
  for ( iterator i = first; i != last; i++ )
    insert( std::prev( pos, std::distance( i, last ) ), *i );

  return const_cast<iterator>( std::prev( pos, std::distance( first, last ) ) );
}

template<typename T, typename Allocator>
constexpr typename Vector<T, Allocator>::iterator
Vector<T, Allocator>::insert( const_iterator pos, std::initializer_list<type> list )
{
  for ( std::size_t i = 0; i < list.size(); i++ )
    insert( pos + i, list[ i ] );
}

template<typename T, typename Allocator>
template<typename... Args>
constexpr typename Vector<T, Allocator>::iterator Vector<T, Allocator>::emplace( const_iterator pos,
                                                 Args&&... args )
{
  if ( pos <= begin() || pos > end() )
    return end();

  m_size++;

  if ( size() > capacity() )
    reserve( capacity() + 1 );

  copy( pos, end() - pos, pos + 1 );

  return construct_object( pos - 1, args... );
}

template<typename T, typename Allocator>
constexpr typename Vector<T, Allocator>::iterator Vector<T, Allocator>::erase( const_iterator pos )
{
  m_size--;

  destroy_at( pos );
  copy( pos, end() - pos, pos + 1 );

  return end();
}

template<typename T, typename Allocator>
constexpr typename Vector<T, Allocator>::iterator Vector<T, Allocator>::erase( const_iterator first,
                                               const_iterator last )
{
  if ( first == last )
    return end();

  m_size -= last - first;

  destroy( first, last );
  copy( last, end() - last, first );

  return end();
}

template<typename T, typename Allocator>
template<typename... Args>
constexpr typename Vector<T, Allocator>::reference Vector<T, Allocator>::emplace_back( Args&&... args )
{
  m_size++;

  if ( size() > capacity() )
    reserve( m_capacity + 1 );

  return *construct_at( m_items + m_size - 1, args... );
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::pop_back()
{
  m_size--;
  destroy_at( m_items + m_size );
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::swap( Vector& other )
{
  ql::swap( m_items, other.m_items );
  ql::swap( m_size, other.m_size );
  ql::swap( m_capacity, other.m_capacity );
}

template<typename T, typename Allocator>
constexpr void Vector<T, Allocator>::destruct()
{
  if ( empty() )
    return;

  destroy( begin(), end() );
  m_allocator.deallocate( m_items, m_size );

  m_items    = nullptr;
  m_size     = 0;
  m_capacity = 0;
}

} // namespace ql
