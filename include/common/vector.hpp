#pragma once
#include "common/algorithm.hpp"
#include "common/memory.hpp"
#include "common/utility.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <type_traits>

namespace ql
{

template<typename T>
class Vector
{
public:

  using type            = T;
  using iterator        = type*;
  using const_iterator  = const type*;
  using reference       = type&;
  using const_reference = const type&;

  Vector( std::initializer_list<T> items );

  Vector( const Vector& other );
  Vector( Vector&& other );

  Vector( const T* items, std::size_t size );

  Vector( std::size_t size );

  template<std::size_t N>
  Vector( const T ( &items )[ N ] );

  Vector() = default;
  ~Vector();

  Vector& operator=( std::initializer_list<type> items );
  Vector& operator=( const Vector& rhs );
  Vector& operator=( Vector&& rhs );
  template<std::size_t N>
  Vector& operator=( const type ( *items )[ N ] );

  void assign( std::initializer_list<T> items );

  void assign( const Vector& other );
  void assign( Vector&& other );

  void assign( const T* items, std::size_t size );

  template<std::size_t N>
  void assign( const T ( &items )[ N ] );

  // Capacity
  bool        empty() const { return m_size == 0; }
  std::size_t size() const { return m_size; }
  std::size_t max_size() const { return UINT64_MAX; }
  void        reserve( std::size_t capacity );
  std::size_t capacity() const { return m_capacity; }
  void        shrink_to_fit();

  // Modifiers
  void clear() { resize( 0 ); }

  iterator insert( const_iterator pos, const T& value );
  iterator insert( const_iterator pos, T&& value );
  iterator insert( const_iterator pos, std::size_t count, const T& value );
  iterator insert( const_iterator pos, iterator first, iterator last );
  iterator insert( const_iterator pos, std::initializer_list<type> list );

  template<typename... Args>
  iterator emplace( const_iterator pos, Args&&... args );

  iterator erase( const_iterator pos );
  iterator erase( const_iterator first, const_iterator last );

  void push_back( const T& item );
  void push_back( T&& item );

  template<typename... Args>
  void emplace_back( Args&&... args );

  template<typename... Args>
  reference emplace_back( Args&&... args );

  void pop_back();
  void resize( std::size_t size );
  void swap( Vector& other );

  // Iterators
  iterator       begin() { return m_items; }
  const_iterator begin() const { return m_items; }
  const_iterator cbegin() const { return begin(); }

  iterator       end() { return m_items + m_size; }
  const_iterator end() const { return m_items + m_size; }
  const_iterator cend() const { return end(); }

  // Element access
  reference       at( std::size_t i ) { return m_items[ i ]; }
  const_reference at( std::size_t i ) const { return m_items[ i ]; }

  reference       operator[]( std::size_t i ) { return m_items[ i ]; }
  const_reference operator[]( std::size_t i ) const { return m_items[ i ]; }

  reference       front() { return m_items[ 0 ]; }
  const_reference front() const { return m_items[ 0 ]; }

  reference       back() { return m_items[ m_size - 1 ]; }
  const_reference back() const { return m_items[ m_size - 1 ]; }

  T*       data() { return m_items; }
  const T* data() const { return m_items; }

private:

  void destruct();

  const static std::size_t m_minimumAllocation = 4;

  T*          m_items    = nullptr;
  std::size_t m_size     = 0;
  std::size_t m_capacity = 0;
};

template<typename T>
Vector<T>::Vector( std::size_t size )
{
  resize( size );
}

template<typename T>
template<std::size_t N>
Vector<T>::Vector( const T ( &items )[ N ] )
{
  assign( items );
}

template<typename T>
Vector<T>::Vector( std::initializer_list<T> items )
{
  assign( items );
}

template<typename T>
Vector<T>::Vector( const Vector& other )
{
  assign( other );
}

template<typename T>
Vector<T>::Vector( Vector&& other )
{
  assign( other );
}

template<typename T>
Vector<T>::Vector( const T* items, std::size_t size )
{
  assign( items, size );
}

template<typename T>
Vector<T>::~Vector()
{
  if ( m_items != nullptr )
    destruct();
}

template<typename T>
template<std::size_t N>
void Vector<T>::assign( const type ( &items )[ N ] )
{
  assign( items, N );
}

template<typename T>
void Vector<T>::assign( std::initializer_list<type> items )
{
  assign( items.begin(), items.size() );
}

template<typename T>
void Vector<T>::assign( const Vector& other )
{
  assign( other.begin(), other.size() );
}

template<typename T>
void Vector<T>::assign( Vector&& other )
{
  ql::swap( m_size, other.m_size );
  ql::swap( m_items, other.m_items );
  ql::swap( m_capacity, other.m_capacity );
}

template<typename T>
void Vector<T>::assign( const type* items, std::size_t size )
{
  m_size = size;
  reserve( m_size );
  ql::copy_n( items, m_size, m_items );
}

template<typename T>
void Vector<T>::reserve( std::size_t capacity )
{
  if ( capacity > m_capacity )
  {
    m_capacity =
      capacity + m_minimumAllocation - ( capacity % m_minimumAllocation );
    std::size_t bytes = m_capacity * sizeof( T );

    m_items = reinterpret_cast<T*>( std::realloc( m_items, bytes ) );
  }
}

template<typename T>
void Vector<T>::shrink_to_fit()
{
  if ( m_capacity > m_size )
  {
    m_capacity        = m_size;
    std::size_t bytes = m_capacity * sizeof( T );

    m_items = reinterpret_cast<T*>( std::realloc( m_items, bytes ) );
  }
}

template<typename T>
void Vector<T>::resize( std::size_t count )
{
  if ( count < m_size )
  {
    ql::destroy_n( m_items, m_size - count );
    shrink_to_fit();
  }
  else if ( count > m_size )
  {
    reserve( count );
    ql::default_construct_n( m_items + m_size, count - m_size );
  }

  m_size = count;
}

template<typename T>
void Vector<T>::push_back( const type& item )
{
  m_size++;

  if ( size() > capacity() )
    reserve( capacity() + 1 );

  back() = item;
}

template<typename T>
void Vector<T>::push_back( type&& item )
{
  m_size++;

  if ( size() > capacity() )
    reserve( m_capacity + 1 );

  back() = ql::move( item );
}

template<typename T>
template<std::size_t N>
Vector<T>& Vector<T>::operator=( const type ( *items )[ N ] )
{
  if ( m_items != nullptr )
    destruct();

  assign( items );
  return *this;
}

template<typename T>
Vector<T>& Vector<T>::operator=( std::initializer_list<type> items )
{
  if ( m_items != nullptr )
    destruct();

  assign( items );
  return *this;
}

template<typename T>
Vector<T>& Vector<T>::operator=( const Vector& rhs )
{
  if ( *this == rhs )
    return *this;

  if ( m_items != nullptr )
    destruct();

  assign( rhs );
  return *this;
}

template<typename T>
Vector<T>& Vector<T>::operator=( Vector&& rhs )
{
  if ( *this == rhs )
    return *this;

  if ( m_items != nullptr )
    destruct();

  assign( rhs );
  return *this;
}

template<typename T>
typename Vector<T>::iterator Vector<T>::insert( const_iterator pos,
                                                const T&       value )
{
  if ( pos <= begin() || pos > end() )
    return end();

  m_size++;

  if ( size() > capacity() )
    reserve( capacity() + 1 );

  ql::copy( pos, const_iterator( end() - pos ), iterator( pos + 1 ) );

  m_items[ m_size - 1 ] = type();
  return &m_items[ m_size - 1 ];
}

template<typename T>
typename Vector<T>::iterator Vector<T>::insert( const_iterator pos, T&& value )
{
  if ( pos <= begin() || pos > end() )
    return end();

  m_size++;

  if ( size() > capacity() )
    reserve( capacity() + 1 );

  ql::copy( pos, end() - pos, pos + 1 );

  m_items[ m_size - 1 ] = ql::move( type() );
  return &m_items[ m_size - 1 ];
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::insert( const_iterator pos, std::size_t count, const T& value )
{
  for ( std::size_t i = 0; i < count; i++ )
    insert( pos + i, value );
}

template<typename T>
typename Vector<T>::iterator Vector<T>::insert( const_iterator pos,
                                                iterator first, iterator last )
{
  for ( iterator i = first; i != last; i++ )
    insert( pos + i - first, *i );
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::insert( const_iterator pos, std::initializer_list<type> list )
{
  for ( std::size_t i = 0; i < list.size(); i++ )
    insert( pos + i, list[ i ] );
}

template<typename T>
template<typename... Args>
typename Vector<T>::iterator Vector<T>::emplace( const_iterator pos,
                                                 Args&&... args )
{
  if ( pos <= begin() || pos > end() )
    return end();

  m_size++;

  if ( size() > capacity() )
    reserve( capacity() + 1 );

  ql::copy( pos, end() - pos, pos + 1 );

  return ql::construct_at( pos - 1, args... );
}

template<typename T>
typename Vector<T>::iterator Vector<T>::erase( const_iterator pos )
{
  m_size--;

  ql::destroy_at( pos );
  ql::copy( pos, end() - pos, pos + 1 );

  return end();
}

template<typename T>
typename Vector<T>::iterator Vector<T>::erase( const_iterator first,
                                               const_iterator last )
{
  if ( first == last )
    return end();

  m_size -= last - first;

  ql::destroy( first, last );
  std::copy( last, end() - last, first );

  return end();
}

template<typename T>
template<typename... Args>
void Vector<T>::emplace_back( Args&&... args )
{
  m_size++;

  if ( size() > capacity() )
    reserve( m_capacity + 1 );

  ql::construct_at( m_items + m_size - 1, args... );
}

template<typename T>
template<typename... Args>
typename Vector<T>::reference Vector<T>::emplace_back( Args&&... args )
{
  m_size++;

  if ( size() > capacity() )
    reserve( m_capacity + 1 );

  return *ql::construct_at( m_items + m_size - 1, args... );
}

template<typename T>
void Vector<T>::pop_back()
{
  m_size--;
  ql::destroy_at( m_items + m_size );
}

template<typename T>
void Vector<T>::swap( Vector& other )
{
  ql::swap( m_items, other.m_items );
  ql::swap( m_size, other.m_size );
  ql::swap( m_capacity, other.m_capacity );
}

template<typename T>
void Vector<T>::destruct()
{
  ql::destroy( begin(), end() );
  std::free( m_items );

  m_items    = nullptr;
  m_size     = 0;
  m_capacity = 0;
}

} // namespace ql
