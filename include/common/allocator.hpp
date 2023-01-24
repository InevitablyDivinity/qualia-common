#pragma once
#include <cstddef>
#include <cmath>

namespace ql
{

template<typename Pointer>
struct AllocationResult
{
  Pointer ptr;
  std::size_t size;
};

template<typename T>
class Allocator
{
public:

  using value_type = T;

  constexpr value_type* allocate( std::size_t size )
  {
    return reinterpret_cast<value_type*>( ::operator new( size * sizeof( T ) ) );
  }

  constexpr AllocationResult<value_type*> allocate_at_least( std::size_t size )
  {
    constexpr std::size_t minimum_allocation = 4;
    size += minimum_allocation - ( minimum_allocation % size );
    return AllocationResult<value_type*> { allocate( size ), size };
  }

  constexpr void deallocate( value_type* memory, std::size_t size )
  {
    ::operator delete( memory );
  }

};

template<typename T>
class MemoryResource
{
public:

  void* allocate( std::size_t bytes, std::size_t alignment )
  {
    return do_allocate( bytes, alignment );
  }

  void deallocate( void* memory, std::size_t bytes, std::size_t alignment )
  {
    do_deallocate( memory, bytes, alignment );
  }

  bool is_equal( const MemoryResource& other ) const
  {
    return do_is_equal( other );
  }

private:

  virtual void* do_allocate( std::size_t bytes, std::size_t alignment ) = 0;
  virtual void do_deallocate( void* memory, std::size_t bytes, std::size_t alignment ) = 0;
  virtual bool do_is_equal( const MemoryResource& other ) const = 0;

};

template<typename T>
class PolymorphicAllocator
{
public:

  using value_type = T*;

  PolymorphicAllocator( MemoryResource<T>* resource )
  : m_resource( resource )
  {
  }

  [[nodiscard]] T* allocate( std::size_t size )
  {
    return m_resource->allocate( size * sizeof( T ), alignof( T ) );
  }

  void deallocate( T* memory, std::size_t size )
  {
    return m_resource->deallocate( memory, size * sizeof( T ), alignof( T ) );
  }

  // construct

  // allocate_bytes
  // deallocate_bytes

  // allocate_object
  // deallocate_object

  // new_object
  // delete_object

  PolymorphicAllocator select_on_container_copy_construction() const
  {
    return PolymorphicAllocator();
  }

  MemoryResource<T>* resource() const { return m_resource; }

private:

  MemoryResource<T>* m_resource = nullptr;

};

}
