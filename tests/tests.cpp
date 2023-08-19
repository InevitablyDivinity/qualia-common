#include <cstddef>
#include <gtest/gtest.h>
#include "common/tuple.hpp"
#include "common/memory.hpp"
#include "common/variant.hpp"
#include "common/vector.hpp"
#include <variant>

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
  EXPECT_EQ( std::get<int>( t ), 1337 );
  EXPECT_FLOAT_EQ( std::get<1>( t ), 66.67 );
  EXPECT_FLOAT_EQ( std::get<double>( t ), 66.67 );
  EXPECT_TRUE( std::get<2>( t ) );
  EXPECT_TRUE( std::get<bool>( t ) );
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
    if ( sp )
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
  ql::Variant<int, float> variant = 66.67f;

  EXPECT_TRUE( std::holds_alternative<float>( variant ) );
  EXPECT_FALSE( std::holds_alternative<int>( variant ) );

  variant = 1337;
  EXPECT_TRUE( std::holds_alternative<int>( variant ) );
  EXPECT_FALSE( std::holds_alternative<float>( variant )  );
}

TEST( Vector, Reallocation )
{
  struct MoveableObject
  {
      MoveableObject() = default;
      MoveableObject( MoveableObject&& ) noexcept {}
      MoveableObject* ptr = this;
  };

  ql::Vector<MoveableObject> v;
  for ( int i = 0; i < 10; i++ )
  {
    v.emplace_back();
  }

  for ( MoveableObject& s: v )
  {
    EXPECT_EQ( s.ptr, &s );
  }
}

TEST( Variant, Visit )
{
  ql::Variant<int, float> variant = 66.67f;

  variant = 3;

  ql::visit( ql::Overload {
    []( int i )   { std::cout << "variant<int> = " << i << std::endl; },
    []( float f ) { std::cout << "variant<float> = " << f << std::endl; }
  }, variant );
}

TEST( Variant, Std )
{
  using variant_type = ql::Variant<int, float>;

  bool b = std::variant_size_v<variant_type> == ql::variant_size_v<variant_type>;
  EXPECT_TRUE( b );

  b = std::is_same_v<std::variant_alternative_t<0, variant_type>, ql::variant_alternative_t<0, variant_type>>;
  EXPECT_TRUE( b );
}

TEST( Variant, Match )
{
  ql::Variant<int, float> variant = 66.67f;

  variant = 3;

  ql::match( variant,
    []( int i )   { std::cout << "variant<int> = " << i << std::endl; },
    []( float f ) { std::cout << "variant<float> = " << f << std::endl; }
  );
}
