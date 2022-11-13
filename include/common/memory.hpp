#pragma once
#include "common/algorithm.hpp"
#include "common/iterator.hpp"
#include <cstring>
#include <cassert>
#include <type_traits>
#include <concepts>

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

  Ptr( Ptr&& other ) { ql::swap( m_object, other.m_object ); }
  Ptr( std::convertible_to<type*> auto ptr ) : m_object( ptr ) {}

  bool valid() const { return m_object != nullptr; }

  type*       get() { return m_object; }
  const type* get() const { return m_object; }

  operator type*() { return m_object; }
  operator const type*() const { return m_object; }

  operator bool() const { return m_object != nullptr; }

  Ptr& operator=( Ptr&& other )
  {
    ql::swap( m_object, other.m_object );
    return *this;
  }

  Ptr& operator=( std::convertible_to<type*> auto ptr )
  {
    m_object = ptr;
    return *this;
  }

  type*    operator->() { return m_object; }
  const T* operator->() const { return m_object; }

  type&    operator*() { return *m_object; }
  const T& operator*() const { return *m_object; }

  bool operator==( std::convertible_to<T> auto ptr ) const
  {
    return m_object == ptr;
  }

protected:

  type* m_object = nullptr;
};

template<typename T, typename Deleter = ql::default_delete<T>>
class UniquePtr : public Ptr<std::remove_all_extents_t<T>>
{
  using base = Ptr<std::remove_all_extents_t<T>>;
public:

  using type = typename base::type;

  UniquePtr( type&& object )
  {
    base::m_object = new type( std::move( object ) );
  }

  ~UniquePtr()
  {
    if ( valid() )
      destruct();
  }

  UniquePtr& operator=( const UniquePtr& other )
  {
    if ( valid() )
      destruct();

    assign( other );
    return *this;
  }

  UniquePtr& operator=( UniquePtr&& other )
  {
    if ( valid() )
      destruct();

    assign( other );
    return *this;
  }

  UniquePtr& operator=( type* ptr )
  {
    if ( valid() )
      destruct();

    assign( ptr );
    return *this;
  }

  using base::valid;

protected:

  void destruct()
  {
    m_delete( base::m_object );
    base::m_object = nullptr;
  }

  using base::m_ptr;
  inline static Deleter m_delete {};
};

template<typename T, typename Deleter>
class WeakPtr;

template<typename T, typename Deleter = ql::default_delete<T>>
class SharedPtr : public Ptr<std::remove_all_extents_t<T>>
{
  using base = Ptr<std::remove_all_extents_t<T>>;

  friend class WeakPtr<T, Deleter>;

public:

  using type = typename base::type;
  using weak_ptr_type = WeakPtr<T, Deleter>;

  SharedPtr() = default;

  SharedPtr( type&& object )
  : m_strongRefCount( new std::size_t( 1 ) )
  {
    m_object = new type( std::move( object ) );
  }
  SharedPtr( const weak_ptr_type& other ) { assign( other ); }
  SharedPtr( const SharedPtr& other ) { assign( other ); }
  SharedPtr( SharedPtr&& other ) { assign( other ); }

  ~SharedPtr() { destruct(); }

  SharedPtr& operator=( const weak_ptr_type& other )
  {
    if ( valid() )
      destruct();

    assign( other );
    return *this;
  }

  SharedPtr& operator=( const SharedPtr& other )
  {
    if ( valid() )
      destruct();

    assign( other );
    return *this;
  }

  SharedPtr& operator=( SharedPtr&& other )
  {
    if ( valid() )
      destruct();

    assign( other );
    return *this;
  }

  void assign( const weak_ptr_type& other )
  {
    if ( other.valid() )
    {
      m_strongRefCount = other.m_strongRefCount;
      m_weakRefCount = other.m_weakRefCount;
      m_object = other.m_object;
      increment_strong_ref_count();
    }
  }

  void assign( const SharedPtr& other )
  {
    if ( other.valid() )
    {
      m_strongRefCount = other.m_strongRefCount;
      m_weakRefCount = other.m_weakRefCount;
      m_object = other.m_object;
      increment_strong_ref_count();
    }
  }

  void assign( SharedPtr&& other )
  {
    ql::swap( m_object, other.m_object );
    ql::swap( m_strongRefCount, other.m_strongRefCount );
  }

  std::size_t use_count() const { return strong_ref_count(); }

  bool unique() const { return use_count() == 1; }

  bool valid() const
  {
    return use_count() > 0;
  }

protected:

  std::size_t strong_ref_count() const { return m_strongRefCount != nullptr ? *m_strongRefCount : 0; }
  std::size_t weak_ref_count() const { return m_strongRefCount != nullptr ? *m_strongRefCount : 0; }

  void decrement_strong_ref_count()
  {
    ( *m_strongRefCount )--;

    if ( m_weakRefCount != nullptr && weak_ref_count() == 0 )
    {
      delete m_strongRefCount;
      m_strongRefCount = nullptr;
    }
  }

  void increment_strong_ref_count()
  {
    ( *m_strongRefCount )++;
  }

  void destruct()
  {
    if ( use_count() > 0 )
      decrement_strong_ref_count();

    if ( m_object != nullptr && use_count() == 0 )
    {
      m_delete( m_object );
      m_object = nullptr;
    }
  }

  inline static Deleter m_delete {};

  using base::m_object;
  std::size_t* m_strongRefCount = nullptr;
  std::size_t* m_weakRefCount = nullptr;
};

// SharedPtr with no ownership, doesn't affect ref count
template<typename T, typename Deleter = ql::default_delete<T>>
class WeakPtr : public Ptr<std::decay_t<T>>
{
  using base = Ptr<std::decay_t<T>>;

  friend class SharedPtr<T, Deleter>;

public:

  using type = typename base::type;
  using shared_ptr_type = SharedPtr<T, Deleter>;

  WeakPtr() = default;

  WeakPtr( const shared_ptr_type& other ) { assign( other ); }
  WeakPtr( const WeakPtr& other ) { assign( other ); }
  WeakPtr( WeakPtr&& other ) { assign( other ); }

  WeakPtr& operator=( const shared_ptr_type& other )
  {
    if ( valid() )
      destruct();

    assign( other );
    return *this;
  }

  WeakPtr& operator=( const WeakPtr& other )
  {
    if ( valid() )
      destruct();

    assign( other );
    return *this;
  }

  WeakPtr& operator=( WeakPtr&& other )
  {
    assign( other );
    return *this;
  }

  void assign( const shared_ptr_type& other )
  {
    if ( other.valid() )
    {
      m_object = other.m_object;
      m_strongRefCount = other.m_strongRefCount;
      m_weakRefCount = new std::size_t( 1 );
    }
  }

  void assign( const WeakPtr& other )
  {
    if ( other.valid() )
    {
      m_object = other.m_object;
      m_strongRefCount = other.m_strongRefCount;
      m_weakRefCount = other.m_weakRefCount;
      increment_weak_ref_count();
    }
  }

  void assign( WeakPtr&& other )
  {
    ql::swap( m_object, other.m_object );
    ql::swap( m_strongRefCount, other.m_strongRefCount );
    ql::swap( m_weakRefCount, other.m_weakRefCount );
  }

  std::size_t use_count() const
  {
    if ( weak_ref_count() > 0 )
    {
      return m_strongRefCount != nullptr ? *m_strongRefCount : 0;
    }
    else
    {
      return 0;
    }
  }

  bool expired() const { return use_count() == 0; }

  bool valid() const
  {
    return !expired();
  }

  // Acquires the resource and guarantees an
  // extended lifetime for its usage
  shared_ptr_type lock() const
  {
    return shared_ptr_type( *this );
  }

protected:

  std::size_t weak_ref_count() const { return m_weakRefCount != nullptr ? *m_weakRefCount : 0; }

  void decrement_weak_ref_count()
  {
    ( *m_weakRefCount )--;

    if ( m_weakRefCount != nullptr && weak_ref_count() == 0 )
    {
      m_object = nullptr;
      m_strongRefCount = nullptr;

      delete m_strongRefCount;
      m_strongRefCount = nullptr;
    }
  }

  void increment_weak_ref_count()
  {
    ( *m_weakRefCount )++;
  }

  void destruct()
  {
    if ( weak_ref_count() > 0 )
      decrement_weak_ref_count();
  }

  using base::m_object;
  std::size_t* m_strongRefCount = nullptr;
  std::size_t* m_weakRefCount = nullptr;
};

template<typename T, typename Deleter = ql::default_delete<T>, typename... Args>
ql::UniquePtr<T, Deleter> make_unique( Args&&... args )
{
  return ql::UniquePtr<T, Deleter>( std::move( T( args... ) ) );
}

template<typename T, typename Deleter = ql::default_delete<T>, typename... Args>
ql::SharedPtr<T, Deleter> make_shared( Args&&... args )
{
  return ql::SharedPtr<T, Deleter>( std::move( T( args... ) ) );
}

} // namespace ql
