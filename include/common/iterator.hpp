#pragma once
#include <iterator>

namespace ql
{

template<typename T>
using IteratorTraits = std::iterator_traits<T>;

template<typename Input>
typename IteratorTraits<Input>::difference_type distance( Input first,
                                                          Input last )
{
}

template<typename T, typename PtrType = T*, typename RefType = T&>
class Iterator
{
public:

  using type      = T;
  using pointer   = PtrType;
  using reference = RefType;

  Iterator( pointer ptr ) { m_ptr = ptr; }

  bool operator==( Iterator& rhs ) { return m_ptr == rhs.m_ptr; }
  bool operator!=( Iterator& rhs ) { return m_ptr != rhs.m_ptr; }

  reference operator*() { return *m_ptr; }

  Iterator& operator++()
  {
    ++m_ptr;
    return *this;
  }
  Iterator& operator--()
  {
    --m_ptr;
    return *this;
  }

  Iterator operator++( int ) { return Iterator( m_ptr + 1 ); }
  Iterator operator--( int ) { return Iterator( m_ptr + 1 ); }

  operator bool() const { return m_ptr != nullptr; }

protected:

  pointer m_ptr = nullptr;
};

template<typename Type, typename PtrType = Type*, typename RefType = Type&>
class ReverseIterator : public Iterator<Type, PtrType, RefType>
{
public:

  using base      = Iterator<Type, PtrType, RefType>;
  using pointer   = PtrType;
  using reference = RefType;

  ReverseIterator& operator++()
  {
    base::m_ptr--;
    return *this;
  }
  ReverseIterator& operator--()
  {
    base::m_ptr++;
    return *this;
  }

  ReverseIterator operator++( int )
  {
    return ReverseIterator( base::m_ptr - 1 );
  }
  ReverseIterator operator--( int )
  {
    return ReverseIterator( base::m_ptr + 1 );
  }
};

} // namespace ql
