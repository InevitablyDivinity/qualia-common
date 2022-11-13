#include <cstddef>
#include <gtest/gtest.h>
#include "common/tuple.hpp"
#include "common/memory.hpp"
#include "common/variant.hpp"

template<std::size_t I, typename... Ts>
static bool validate_offset( const ql::Tuple<Ts...>& t )
{
  if constexpr ( I == 0 )
  {
    return true;
  }
  else
  {
    std::intptr_t baseAddress = std::intptr_t( &t );
    std::ptrdiff_t offset = std::abs( std::intptr_t( std::get<I>( t ) ) - baseAddress );
    std::ptrdiff_t previousOffset = std::abs( std::intptr_t( std::get<I - 1>( t ) ) - baseAddress );
    return offset > previousOffset;
  }
};

TEST( Tuple, StandardLayout )
{
  ql::Tuple t = { 1337, 66.67, true };

  bool result = validate_offset<0>( t );
  EXPECT_TRUE( result );

  result = validate_offset<1>( t );
  EXPECT_TRUE( result );

  result = validate_offset<2>( t );
  EXPECT_TRUE( result );
}

TEST( Tuple, StdGet )
{
  ql::Tuple t = { 1337, 66.67, true };
  EXPECT_EQ( std::get<0>( t ) , 1337 );
  EXPECT_FLOAT_EQ( std::get<1>( t ), 66.67 );
  EXPECT_TRUE( std::get<2>( t ) );
}

TEST( Tuple, StructuredBindingDeclaration )
{
  ql::Tuple t = { 1337, 66.67, true };
  auto [i, f, b] = t;
  EXPECT_EQ( i , 1337 );
  EXPECT_FLOAT_EQ( f, 66.67 );
  EXPECT_TRUE( b );
}

TEST( Memory, WeakPtr )
{
  auto observe = [&]( ql::WeakPtr<int> weakPtr ) -> bool
  {
    auto sp = weakPtr.lock();
    if ( sp.valid() )
    {
      return true;
    }
    else
    {
      return false;
    }
  };

  ql::WeakPtr<int> weakPtr;
  {
    ql::SharedPtr<int> sharedPtr = ql::make_shared<int>( 99 );
    weakPtr = sharedPtr;

    EXPECT_TRUE( observe( weakPtr ) );
  }

  EXPECT_FALSE( observe( weakPtr ) );
}

TEST( Variant, TypeChecking )
{
  ql::Variant<int, float> variant;

  variant = 66.67f;
  EXPECT_TRUE( variant.holds_alternative<float>() );
  EXPECT_FALSE( variant.holds_alternative<int>() );

  variant = 1337;
  EXPECT_TRUE( variant.holds_alternative<int>() );
  EXPECT_FALSE( variant.holds_alternative<float>() );
}
