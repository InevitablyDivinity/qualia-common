#pragma once
#include "common/algorithm.hpp"
#include "common/utility.hpp"
#include <cstring>
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

  using element_type = std::remove_extent_t<T>;

  Ptr() = default;

  Ptr( Ptr&& other ) { swap( m_object, other.m_object ); }
  Ptr( std::convertible_to<element_type*> auto ptr ) : m_object( ptr ) {}

  element_type*       get() { return m_object; }
  const element_type* get() const { return m_object; }

  operator element_type*() { return m_object; }
  operator const element_type*() const { return m_object; }

  operator bool() const { return m_object != nullptr; }

  Ptr& operator=( Ptr&& other )
  {
    swap( m_object, other.m_object );
    return *this;
  }

  Ptr& operator=( std::convertible_to<element_type*> auto ptr )
  {
    m_object = ptr;
    return *this;
  }

  element_type*    operator->() { return m_object; }
  const T* operator->() const { return m_object; }

  element_type&    operator*() { return *m_object; }
  const T& operator*() const { return *m_object; }

  bool operator==( std::convertible_to<element_type*> auto ptr ) const
  {
    return m_object == ptr;
  }

protected:

  element_type* m_object = nullptr;
};

template<typename T, typename Deleter = default_delete<T>>
class UniquePtr : public Ptr<std::remove_extent_t<T>>
{
public:

  using element_type = std::remove_extent_t<T>;

  UniquePtr( element_type&& object )
  {
    m_object = new element_type( move( object ) );
  }

  UniquePtr( const UniquePtr& other ) { assign( other ); }
  UniquePtr( UniquePtr&& other ) { assign( move( other ) ); }

  ~UniquePtr()
  {
    destruct();
  }

  UniquePtr& operator=( const UniquePtr& other )
  {
    destruct();

    assign( other );
    return *this;
  }

  UniquePtr& operator=( UniquePtr&& other )
  {
    destruct();

    assign( move( other ) );
    return *this;
  }

  UniquePtr& operator=( element_type* ptr )
  {
    destruct();

    assign( ptr );
    return *this;
  }

  void assign( const UniquePtr& other )
  {
    m_object = new element_type( *other.m_object );
  }

  void assign( UniquePtr&& other )
  {
    swap( m_object, other.m_object );
  }

protected:

  void destruct()
  {
    if ( m_object != nullptr )
    {
      deleter( m_object );
      m_object = nullptr;
    }
  }

  using base = Ptr<element_type>;
  using base::m_object;

  inline static Deleter deleter;
};

class RefCount
{
public:

  RefCount() = default;

  RefCount( std::size_t n )
  {
    m_refCount = n;
  }

  RefCount& operator=( std::size_t n )
  {
    m_refCount = n;
    return *this;
  }

  operator std::size_t() const { return m_refCount; }

  bool operator==( std::size_t n ) const
  {
    return m_refCount == n;
  }

  RefCount& operator--(int)
  {
    m_refCount--;
    return *this;
  }

  RefCount& operator++(int)
  {
    m_refCount++;
    return *this;
  }

private:

  std::size_t m_refCount = 0;

};

struct WeakStrongRefCount
{
  RefCount weak = 0;
  RefCount strong = 0;
};

template<typename T, typename Deleter>
class WeakPtr;

template<typename T, typename Deleter = default_delete<T>>
class SharedPtr : public Ptr<std::remove_extent_t<T>>
{
public:

  using element_type = std::remove_extent_t<T>;
  using weak_type = WeakPtr<element_type, Deleter>;

  SharedPtr() = default;

  SharedPtr( element_type&& object )
  {
    m_object = new element_type( move( object ) );
    m_refCount = new WeakStrongRefCount { .weak = 0, .strong = 1 };
  }
  SharedPtr( const weak_type& other ) { assign( other ); }
  SharedPtr( const SharedPtr& other ) { assign( other ); }
  SharedPtr( SharedPtr&& other ) { assign( move( other ) ); }

  ~SharedPtr()
  {
    destruct();
  }

  SharedPtr& operator=( const weak_type& other )
  {
    destruct();

    assign( other );
    return *this;
  }

  SharedPtr& operator=( const SharedPtr& other )
  {
    destruct();

    assign( other );
    return *this;
  }

  SharedPtr& operator=( SharedPtr&& other )
  {
    destruct();

    assign( move( other ) );
    return *this;
  }

  std::size_t use_count() const { return strong_ref_count(); }

  bool unique() const { return use_count() == 1; }

protected:

  std::size_t strong_ref_count() const { return m_refCount != nullptr ? std::size_t( m_refCount->strong ) : 0; }
  std::size_t weak_ref_count() const { return m_refCount != nullptr ? std::size_t( m_refCount->weak ) : 0; }

  void assign( const weak_type& other )
  {
    m_object = other.m_object;
    m_refCount = other.m_refCount;

    if ( m_refCount != nullptr )
    {
      m_refCount->strong++;
    }
  }

  void assign( const SharedPtr& other )
  {
    m_object = other.m_object;
    m_refCount = other.m_refCount;

    if ( m_refCount != nullptr )
    {
      m_refCount->strong++;
    }
  }

  void assign( SharedPtr&& other )
  {
    swap( m_object, other.m_object );
    swap( m_refCount, other.m_refCount );
  }

  void destruct()
  {
    if ( strong_ref_count() > 0 )
    {
      m_refCount->strong--;
    }

    if ( weak_ref_count() == 0 && strong_ref_count() == 0 )
    {
      if ( m_refCount != nullptr )
      {
        delete m_refCount;
      }
    }

    // Delete the object
    if ( use_count() == 0 && m_object != nullptr )
    {
      deleter( m_object );
    }

    m_refCount = nullptr;
    m_object = nullptr;
  }

  friend weak_type;

  using base = Ptr<element_type>;
  using base::m_object;

  WeakStrongRefCount* m_refCount = nullptr;
  inline static Deleter deleter;
};

// SharedPtr with no ownership, doesn't affect ref count
template<typename T, typename Deleter = default_delete<T>>
class WeakPtr : public Ptr<std::remove_extent_t<T>>
{
public:

  using element_type = std::remove_extent_t<T>;
  using shared_type = SharedPtr<element_type, Deleter>;

  WeakPtr() = default;

  WeakPtr( const shared_type& other ) { assign( other ); }
  WeakPtr( const WeakPtr& other ) { assign( other ); }
  WeakPtr( WeakPtr&& other ) { assign( move( other ) ); }

  ~WeakPtr()
  {
    if ( m_refCount != nullptr )
      destruct();
  }

  WeakPtr& operator=( const shared_type& other )
  {
    destruct();

    assign( other );
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

    assign( move( other ) );
    return *this;
  }

  std::size_t use_count() const { return strong_ref_count(); }

  bool expired() const { return use_count() == 0; }

  // Acquires the resource and guarantees an
  // extended lifetime for its usage
  shared_type lock() const
  {
    return expired() ? shared_type() : shared_type( *this );
  }

protected:

  std::size_t strong_ref_count() const { return m_refCount != nullptr ? std::size_t( m_refCount->strong ) : 0; }
  std::size_t weak_ref_count() const { return m_refCount != nullptr ? std::size_t( m_refCount->weak ) : 0; }

  void assign( const shared_type& other )
  {
    m_object = other.m_object;
    m_refCount = other.m_refCount;

    if ( m_refCount != nullptr )
    {
      m_refCount->weak++;
    }
  }

  void assign( const WeakPtr& other )
  {
    m_object = other.m_object;
    m_refCount = other.m_refCount;

    if ( m_refCount != nullptr )
    {
      m_refCount->weak++;
    }
  }

  void assign( WeakPtr&& other )
  {
    swap( m_object, other.m_object );
    swap( m_refCount, other.m_refCount );
  }

  void destruct()
  {
    if ( weak_ref_count() > 0 )
    {
      m_refCount->weak--;
    }

    if ( weak_ref_count() == 0 && strong_ref_count() == 0 )
    {
      if ( m_refCount != nullptr )
      {
        delete m_refCount;
      }
    }

    m_object = nullptr;
    m_refCount = nullptr;
  }

  friend shared_type;

  using base = Ptr<element_type>;
  using base::m_object;

  WeakStrongRefCount* m_refCount = nullptr;
};

template<typename T, typename Deleter = default_delete<T>, typename... Args>
UniquePtr<T, Deleter> make_unique( Args&&... args )
{
  return UniquePtr<T, Deleter>( move( T( args... ) ) );
}

template<typename T, typename Deleter = default_delete<T>, typename... Args>
SharedPtr<T, Deleter> make_shared( Args&&... args )
{
  return SharedPtr<T, Deleter>( move( T( args... ) ) );
}

} // namespace ql
