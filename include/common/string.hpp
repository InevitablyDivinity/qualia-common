#pragma once
#include "common/algorithm.hpp"
#include "common/allocator.hpp"
#include "common/tuple.hpp"
#include <cstddef>
#include <string>
#include <type_traits>

namespace ql
{

template<typename T>
struct char_traits
{
  using char_type = T;
  using size_type = std::uint16_t;

  static constexpr size_type length( const char_type* ptr )
  {
    size_type size = 0;

    while ( *ptr++ != static_cast<char_type>( '\0' ) )
      size++;

    return size;
  }

  static constexpr int compare( const char_type* s1, const char_type* s2, size_type count )
  {
    return std::char_traits<T>::compare( s1, s2, count );
  }
};

class String
{
public:

  using value_type      = char;
  using size_type       = std::size_t;
  using traits_type     = std::char_traits<value_type>;
  using allocator_type  = std::allocator<value_type>;

  using reference       = value_type&;
  using const_reference = const value_type&;

  using pointer         = value_type*;
  using const_pointer   = const value_type*;

  using iterator        = value_type*;
  using const_iterator  = const value_type*;

  class SharedData
  {
    std::size_t m_refCount = 1;
    value_type m_data[1];
  public:

    static ql::Tuple<value_type*, std::size_t> create( std::size_t size )
    {
      const std::size_t offset = data_offset();
      const std::size_t allocSize = data_offset() + ( size + 1 ) * char_size;
      byte_t* bytes = new byte_t[allocSize];

      SharedData* data = ql::construct_at<SharedData>( reinterpret_cast<SharedData*>( bytes ) );

      return {
        data->m_data,
        ( allocSize - data_offset() ) / char_size - 1
      };
    }

    static constexpr std::size_t data_offset()
    {
      return offsetof( SharedData, m_data );
    }

    static constexpr SharedData* from_data( pointer ptr )
    {
      return static_cast<SharedData*>(
          static_cast<void*>( static_cast<byte_t*>(
          static_cast<void*>( ptr ) ) - data_offset() ) );
    }

    static constexpr std::size_t increment_ref_count( pointer ptr )
    {
      return from_data( ptr )->increment_ref_count();
    }

    static constexpr std::size_t decrement_ref_count( pointer ptr )
    {
      SharedData* prologue = from_data( ptr );
      std::size_t count = prologue->decrement_ref_count();
      if ( count == 0 )
      {
        byte_t* bytes = reinterpret_cast<byte_t*>( prologue );
        delete[] prologue;
      }
      return count;
    }

    constexpr SharedData() = default;

    constexpr std::size_t increment_ref_count()
    {
      return m_refCount++;
    }

    constexpr std::size_t decrement_ref_count()
    {
      return m_refCount--;
    }

    constexpr pointer data() { return m_data; }
    constexpr const_pointer data() const { return m_data; }

    constexpr operator pointer() { return m_data; }
    constexpr operator const_pointer() const { return m_data; }

  };

  constexpr String() = default;

  constexpr String( const_pointer src )
  {
    if ( not src )
    {
      assign( nullptr );
    }

    assign( src );
  }

  constexpr String( const_pointer src, size_type size )
  {
    if ( not src or size == 0 )
    {
      assign( nullptr );
    }

    assign( src, size );
  }

  constexpr String( const String& src )
  {
    assign( src );
  }

  constexpr String( String&& src )
  {
    assign( move( src ) );
  }

  constexpr String( std::nullptr_t )
  {
    assign( nullptr );
  }

  constexpr String& operator=( const_pointer rhs )
  {
    destruct();
    assign( rhs );
    return *this;
  }

  constexpr String& operator=( const String& rhs )
  {
    destruct();
    assign( rhs );
    return *this;
  }

  constexpr String& operator=( String&& rhs )
  {
    destruct();
    assign( std::move( rhs ) );
    return *this;
  }

  constexpr bool operator==( const String& rhs ) const
  {
    if ( size() != rhs.size() )
      return false;

    return char_traits<value_type>::compare( data(), rhs.data(), size() );
  }

  static consteval size_type max_size() { return std::numeric_limits<size_type>::max(); }

  constexpr size_type size() const
  {
    switch ( category() )
    {
      using enum Category;
      case Small: // stack
      {
        return small.size();
      }
      case Medium: // normal
      case Large: // ref-counted/COW
      {
        return typical.size();
      }
    }
  }

  constexpr pointer data()
  {
    switch ( category() )
    {
      using enum Category;
      case Small: // stack
      {
        return small.m_data;
      }
      case Medium: // normal
      case Large: // ref-counted/COW
      {
        return typical.m_data;
      }
    }
  }

  constexpr const_pointer data() const
  {
    switch ( category() )
    {
      using enum Category;
      case Small: // stack
      {
        return small.m_data;
      }
      case Medium: // normal
      case Large: // ref-counted/COW
      {
        return typical.m_data;
      }
    }
  }

  constexpr operator const_pointer() const { return data(); }

private:

  constexpr void assign( const_pointer src )
  {
    assign( src, char_traits<value_type>::length( src ) );
  }

  constexpr void assign( const String& src )
  {
    switch ( src.category() )
    {
      using enum Category;
      case Small: // stack
      case Medium: // normal
      {
        assign( src.data(), src.size() );
        break;
      }
      case Large: // ref-counted/COW
      {
        typical.m_data = src.typical.m_data;
        typical.m_size = src.typical.m_size;
        typical.m_capacity = src.typical.m_capacity;
        break;
      }
    }
  }

  constexpr void assign( String&& src )
  {
    switch ( src.category() )
    {
      case Small:
        ql::swap( small, src.small );
        break;
      case Medium:
      case Large:
        ql::swap( typical, src.typical );
        break;
    }
  }

  constexpr void assign( std::nullptr_t )
  {
  }

  constexpr void assign( const_pointer src, std::size_t size )
  {
    switch ( categorize_size( size ) )
    {
      using enum Category;
      case Small: // stack
      {
        ql::uninitialized_copy_n( src, size, small.m_data );
        small.set_size( size );
        break;
      }
      case Medium: // normal
      {
        const std::size_t allocSize = ( size + 1 ) * char_size;
        typical.m_data = new value_type[allocSize];
        ql::uninitialized_copy_n( src, size, typical.m_data );
        typical.m_size = size;
        typical.set_capacity( allocSize / char_size - 1, Medium );
        typical.m_data[size] = '\0';
        break;
      }
      case Large: // ref-counted/COW
      {
        auto [data, allocated] = SharedData::create( size );
        ql::uninitialized_copy_n( src, size, data );

        typical.m_size = size;
        typical.set_capacity( allocated, Large );
        typical.m_data = data;
        typical.m_data[size] = '\0';

        break;
      }
    }
  }

  constexpr void destruct()
  {
    switch ( category() )
    {
      using enum Category;
      case Small: // stack
        break;
      case Medium: // normal
        delete[] typical.m_data;
        break;
      case Large: // ref-counted/COW
        SharedData::decrement_ref_count( typical.m_data );
        break;
    }
  }

  // find() uses Boyer-Moore algorithm

  /*
   * Normal string layout ("Hello world!!")
   *  data            size            capacity
   * ┌───────────────┬───────────────┬───────────────┐
   * │               │       14      │       15      │
   * └───────────────┴───────────────┴───────────────┘
   *   🠋
   * ┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
   * │H│e│l│l│o│,│ │w│o│r│l│d│!│!│0│
   * └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
   *
   * Small string layout: ("Hello world!!")
   * ┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
   * │H│e│l│l│o│,│ │w│o│r│l│d│!│!│0│.│.│.│.│.│.│.│.│9│
   * └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
   *                                                🠋
   *                                       remaining capacity
   */


  enum Category : std::uint8_t
  {
    Small  = 0,        // in-situ
    Medium = (1 << 7), // heap memory and copied eagerly
    Large  = (1 << 6), // ref-counted heap memory and copied lazily (copy-on-write)
  };

  struct TypicalString
  {
    value_type* m_data;
    size_type   m_size;
    size_type   m_capacity;

    constexpr size_type size() const
    {
      return m_size;
    }

    constexpr size_type capacity() const
    {
      return m_capacity & capacity_extract_mask;
    }

    constexpr void set_capacity( std::size_t capacity, Category category )
    {
      m_capacity = capacity | ( static_cast<size_type>( category ) << category_shift );
    }
  };

  static constexpr size_type char_size = sizeof( value_type );

  static constexpr size_type typical_string_size = sizeof( TypicalString );
  static constexpr size_type last_byte = typical_string_size - 1;
  static constexpr size_type small_string_capacity = last_byte / char_size;

  struct SmallString
  {
    value_type m_data[typical_string_size / char_size];

    inline size_type remaining_capacity() const
    {
      return static_cast<size_type>( m_data[small_string_capacity] );
    }

    void set_size( size_type size )
    {
      m_data[small_string_capacity] = value_type( small_string_capacity - size );
      m_data[size] = '\0';
    }

    size_type size() const
    {
      return capacity() - remaining_capacity();
    }

    size_type capacity() const
    {
      return small_string_capacity;
    }
  };

  union
  {
    byte_t        bytes[typical_string_size] = {};
    TypicalString typical;
    SmallString   small;
  };

  using category_type = std::underlying_type_t<Category>;
  using capacity_type = size_type;

  // Used to extract the category from the last byte of a size_type
  static constexpr category_type category_extract_mask =
    Category::Small | Category::Medium | Category::Large;

  // Shift to the category section of the capacity type (last byte)
  static constexpr capacity_type category_shift = ( sizeof( capacity_type ) - 1 ) * 8;

  // Used to extract capacity from a size_type
  static constexpr capacity_type capacity_extract_mask =
    ~( static_cast<capacity_type>( category_extract_mask ) << category_shift );

  constexpr Category category() const
  {
    return static_cast<Category>( bytes[last_byte] & category_extract_mask );
  }

  static constexpr Category categorize_size( size_type size )
  {
    if ( std::is_constant_evaluated() )
    {
      // Use the simplest code path in constant evaluation
      return Category::Medium;
    }
    else
    {
      if ( size <= 23 )
        return Category::Small;
      else if ( size <= 255 )
        return Category::Medium;
      else
        return Category::Large;
    }
  }

};

}
