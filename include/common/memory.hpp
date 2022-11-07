#pragma once
#include "common/algorithm.hpp"
#include "common/iterator.hpp"
#include <cstring>
#include <type_traits>

namespace ql
{

template<typename T, typename... Args>
T* construct_at( T* dest, Args&&... args )
{
  return new ( dest ) T( args... );
}

template<typename ForwardIterator>
void default_construct( ForwardIterator first, ForwardIterator last )
{
  using traits = ql::IteratorTraits<ForwardIterator>;
  using type   = typename traits::value_type;

  if constexpr ( std::is_trivially_default_constructible_v<type> )
  {
    std::memset( first, 0, last - first );
  }
  else
  {
    for ( ForwardIterator i = first; i != last; i++ )
      ql::construct_at<type>( i );
  }
}

template<typename ForwardIterator>
void default_construct_n( ForwardIterator first, std::size_t count )
{
  default_construct( first, first + count );
}

template<typename T>
void destroy_at( T* object )
{
  if constexpr ( std::is_destructible_v<T> )
    object->~T();
}

template<typename ForwardIterator>
void destroy( ForwardIterator first, ForwardIterator last )
{
  for ( auto object = first; object != last; object++ )
    destroy_at( &( *object ) );
}

template<typename ForwardIterator>
void destroy_n( ForwardIterator first, std::size_t count )
{
  destroy( first, first + count );
}

template<typename T>
class default_delete
{
public:

  void operator()( T* object ) const { delete object; }
};

template<typename T>
class default_delete<T[]>
{
public:

  void operator()( T* array ) const { delete[] array; }
};

template<typename T>
class Ptr
{
public:

  using type = T;

  Ptr() = default;

  Ptr( type* ptr ) { assign( ptr ); }

  void assign( type* ptr ) { m_ptr = ptr; }

  type*    operator->() { return m_ptr; }
  const T* operator->() const { return m_ptr; }

  type&    operator*() { return *m_ptr; }
  const T& operator*() const { return *m_ptr; }

  template<typename U>
  bool operator==( const Ptr<U>& rhs ) const
  {
    return m_ptr == rhs.m_ptr;
  }

  bool valid() const { return m_ptr != nullptr; }
       operator bool() const { return m_ptr != nullptr; }

  operator type*() { return m_ptr; }
  operator const type*() const { return m_ptr; }

  type*       get() { return m_ptr; }
  const type* get() const { return m_ptr; }

protected:

  type* m_ptr = nullptr;
};

template<typename T, typename Deleter = ql::default_delete<T>>
class UniquePtr : public Ptr<std::decay_t<T>>
{
  using base = Ptr<std::remove_all_extents_t<T>>;

public:

  using type = typename base::type;

  UniquePtr() = default;

  UniquePtr( type* ptr ) { assign( ptr ); }
  UniquePtr( UniquePtr&& other ) { assign( other ); }

  ~UniquePtr()
  {
    if ( base::m_ptr != nullptr )
      destruct();
  }

  UniquePtr& operator=( UniquePtr&& other )
  {
    if ( base::m_ptr != nullptr )
      destruct();

    assign( other );
    return *this;
  }

  UniquePtr& operator=( type* ptr )
  {
    if ( base::m_ptr != nullptr )
      destruct();

    assign( ptr );
    return *this;
  }

  void assign( UniquePtr&& other ) { ql::swap( base::m_ptr, other.m_ptr ); }

  void assign( type* ptr ) { base::m_ptr = ptr; }

  void release() { destruct(); }

private:

  void destruct()
  {
    m_delete( base::m_ptr );
    base::m_ptr = nullptr;
  }

  inline static Deleter m_delete {};
};

template<typename T, typename Deleter = ql::default_delete<T>>
class SharedPtr : public UniquePtr<std::decay_t<T>, Deleter>
{
  using base = UniquePtr<std::decay_t<T>, Deleter>;

public:

  using type = typename base::type;

  SharedPtr() = default;

  SharedPtr( type* ptr ) { assign( ptr ); }
  SharedPtr( const SharedPtr& other ) { assign( other ); }
  SharedPtr( SharedPtr&& other ) { assign( other ); }

  ~SharedPtr() { destruct(); }

  SharedPtr& operator=( const SharedPtr& other )
  {
    if ( base::m_ptr != nullptr )
      destruct();

    assign( other );
    return *this;
  }

  SharedPtr& operator=( SharedPtr&& other )
  {
    if ( base::m_ptr != nullptr )
      destruct();

    assign( other );
    return *this;
  }

  SharedPtr& operator=( type* ptr )
  {
    if ( base::m_ptr != nullptr )
      destruct();

    assign( ptr );
    return *this;
  }

  void assign( const SharedPtr& other )
  {
    base::assign( other.m_ptr );
    increment_ref_count();
  }

  void assign( SharedPtr&& other )
  {
    ql::swap( base::m_ptr, other.m_ptr );
    ql::swap( m_refCount, other.m_refCount );
  }

  void assign( type* ptr )
  {
    increment_ref_count();
    base::assign( ptr );
  }

  void release() { destruct(); }

  std::size_t use_count() const
  {
    return m_refCount != nullptr ? *m_refCount : 0;
  }
  bool unique() const { return use_count() > 0; }

protected:

  void destruct()
  {
    decrement_ref_count();

    if ( *m_refCount == 0 )
    {
      base::destruct();

      delete m_refCount;
      m_refCount = nullptr;
    }
  }

  void increment_ref_count()
  {
    if ( m_refCount == nullptr )
      m_refCount = new std::size_t( 0 );

    ( *m_refCount )++;
  }

  void decrement_ref_count() { ( *m_refCount )--; }

  // TODO: make m_refCount atomic
  std::size_t* m_refCount = nullptr;
};

// SharedPtr with no ownership, doesn't affect ref count
template<typename T, typename Deleter = ql::default_delete<T>>
class WeakPtr : public SharedPtr<std::decay_t<T>, Deleter>
{
  using base = SharedPtr<std::decay_t<T>, Deleter>;

public:

  using type = typename base::type;

  WeakPtr() = default;

  WeakPtr( const SharedPtr<type, Deleter>& other ) { assign( other.m_ptr ); }

  WeakPtr( type* ptr ) { assign( ptr ); }
  WeakPtr( const WeakPtr& other ) { assign( other ); }
  WeakPtr( WeakPtr&& other ) { assign( other ); }

  ~WeakPtr() { destruct(); }

  WeakPtr& operator=( SharedPtr<type, Deleter> other )
  {
    destruct();
    assign( other.m_ptr );
    return *this;
  }

  WeakPtr& operator=( const WeakPtr& other )
  {
    destruct();
    assign( other );
    return *this;
  }

  WeakPtr& operator=( WeakPtr&& other )
  {
    destruct();
    assign( other );
    return *this;
  }

  WeakPtr& operator=( type* ptr )
  {
    destruct();
    assign( ptr );
    return *this;
  }

  void assign( const SharedPtr<type, Deleter>& other ) {}

  void assign( const WeakPtr& other ) {}

  void assign( WeakPtr&& other )
  {
    ql::swap( base::m_ptr, other.m_ptr );
    ql::swap( base::m_refCount, other.m_refCount );
  }

  void assign( type* ptr ) { base::assign( ptr ); }

  void release() { destruct(); }

  bool expired() const {}

  // Acquires the resource and guarantees an
  // extended lifetime for its usage
  SharedPtr<type, Deleter> lock() const { return SharedPtr( base::m_ptr ); }

private:

  void destruct() { base::destruct(); }
};

} // namespace ql
