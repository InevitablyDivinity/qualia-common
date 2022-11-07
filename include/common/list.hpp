#pragma once
#include "common/utility.hpp"
#include <concepts>
#include <cstddef>
#include <initializer_list>

namespace ql
{

template<typename Type>
class List
{
  struct Node
  {
    Node* previous = nullptr;
    Node* next     = nullptr;
    Type  value;
  };

  class NodeIterator
  {
    friend class List;

  public:

    NodeIterator( Node* node ) : m_node( node ) {}
    NodeIterator() = delete;

    bool operator==( const NodeIterator& rhs ) const
    {
      return m_node == rhs.m_node;
    }
    bool operator==( const Node* node ) const { return m_node == node; }

    bool operator!=( const NodeIterator& rhs ) const
    {
      return m_node != rhs.m_node;
    }

    Type&       operator*() { return m_node->value; }
    const Type& operator*() const { return m_node->value; }

    Type*       operator->() { return &m_node->value; }
    const Type* operator->() const { return &m_node->value; }

    NodeIterator& operator--()
    {
      m_node = m_node->previous;
      return *this;
    }

    NodeIterator& operator++()
    {
      m_node = m_node->next;
      return *this;
    }

    NodeIterator operator--( int ) { return NodeIterator( m_node->previous ); }

    NodeIterator operator++( int ) { return NodeIterator( m_node->next ); }

  private:

    Node* m_node = nullptr;
  };

public:

  using type     = Type;
  using iterator = NodeIterator;

  List() = default;

  template<std::size_t Size>
  List( const Type ( &items )[ Size ] )
  {
    for ( Type& item : items )
      insert( item );
  }

  List( std::initializer_list<Type> items )
  {
    for ( Type& item : items )
      insert( item );
  }

  List( const List& src )
  {
    for ( Type& item : src )
      insert( item );
  }

  List( List&& src )
  {
    m_begin     = src.m_begin;
    m_end       = src.m_end;
    m_size      = src.m_end;
    src.m_begin = nullptr;
    src.m_end   = nullptr;
    src.m_size  = 0;
  }

  ~List()
  {
    for ( Node* node = m_begin; node != nullptr; node = node->next )
    {
      delete node->previous;
    }
  }

  void push_back( Type&& value )
  {
    Node* node  = new Node();
    node->value = ql::move( value );

    if ( !m_begin )
    {
      m_begin = node;
      m_end   = m_begin;
    }
    else
    {
      m_end->next = node;
      m_end       = node;
    }

    m_size++;
  }

  void push_back( const Type& value )
  {
    Node* node  = new Node();
    node->value = value;

    if ( !m_begin )
    {
      m_begin = node;
      m_end   = m_begin;
    }
    else
    {
      m_end->next = node;
      m_end       = node;
    }

    m_size++;
  }

  iterator find( const Type& value )
    requires std::equality_comparable<Type>
  {
    for ( Node* node = m_begin; node != m_end; node = node->next )
    {
      if ( node->value == value )
        return iterator( node );
    }

    return iterator( nullptr );
  }

  void remove( iterator item )
  {
    Node* node = item.m_node;

    if ( node == m_begin )
    {
      m_begin = node->next;
    }
    else if ( node == m_end )
    {
      m_end = node->previous;
    }
    else
    {
      Node* previous = node->previous;
      Node* next     = node->next;

      previous->next = next;
      next->previous = previous;
    }

    delete node;
  }

  void resize( std::size_t size )
  {
    Node* node = m_end;
    while ( size > m_size )
    {
      node->next = new Node();
      node       = node->next;

      m_size++;
    }

    while ( size < m_size )
    {
      node = node->previous;
      delete node->next;

      m_size--;
    }
  }

  iterator       begin() { return iterator( m_begin ); }
  const iterator begin() const { return iterator( m_begin ); }

  iterator       end() { return iterator( nullptr ); }
  const iterator end() const { return iterator( nullptr ); }

  bool        empty() const { return m_size == 0; }
  std::size_t size() const { return m_size; }

private:

  std::size_t m_size  = 0;
  Node*       m_begin = nullptr;
  Node*       m_end   = nullptr;
};

} // namespace ql
