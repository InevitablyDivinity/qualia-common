#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <type_traits>
#include <source_location>

#ifdef assert
  #undef assert
#endif

#if __GNUC__ || __clang__
  #define FORCEINLINE gnu::always_inline
#elif _MSC_VER
  #define FORCEINLINE msvc::forceinline
#else
  #define FORCEINLINE
#endif

namespace ql
{

// Various utility types
using std::size_t;
using std::ptrdiff_t;
using std::nullptr_t;

using index_t = ptrdiff_t;
using byte_t = uint8_t;

// Various fixed-width types
using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

/* std::float16_t will arrive in C++23,
 * under <cstdfloat>. */
// using std::float16_t;
using float32_t = float;
using float64_t = double;
/* This type should exist, but isn't portable because
 * MSVC doesn't adhere to the standard and implements
 * long double as a binary64 IEEE-754 float. */
// using float128_t = long double;

using bool8_t = bool;
using bool16_t = uint16_t;
using bool32_t = uint32_t;
using bool64_t = uint64_t;

#ifdef __cpp_if_consteval
  #define IF_CONSTEVAL if consteval
  #define IF_NOT_CONSTEVAL if !consteval
#else
  #define IF_CONSTEVAL	 if ( std::is_constant_evaluated() ) [[unlikely]]
  #define IF_NOT_CONSTEVAL if ( !std::is_constant_evaluated() ) [[likely]]
#endif


// A replacement for C assertions
inline void assert( bool expression, const char* msg,
                    const std::source_location l = std::source_location::current() ) noexcept
{
  if ( !std::is_constant_evaluated() )
  {
    if ( !expression )
    {
      std::fprintf( stderr, "%s(%u:%u) %s\n", l.file_name(), l.line(), l.column(), msg );
      std::abort();
    }
  }
}

// Temporary stand-in for C++23's std::unreachable()
[[FORCEINLINE, noreturn]] inline void unreachable() noexcept
{
#if __GNUC__ || __clang__
  __builtin_unreachable();
#elif _MSC_VER
  __assume( false );
#else
  #error ql::unreachable() is unimplemented for your compiler.
#endif
}

// Temporary stand-in for C++23's [[assume]]
[[FORCEINLINE]] inline void assume( bool expr ) noexcept
{
#if __clang__
  __builtin_assume( expr );
#elif __GNUC__
  if ( !expr )
    unreachable();
#elif _MSC_VER
  __assume( expr );
#else
  #error ql::assume() is unimplemented for your compiler.
#endif
}

// An equivalent of std::breakpoint()
[[FORCEINLINE]] inline void breakpoint() noexcept
{
#if __GNUC__ || __clang__
  __builtin_trap();
#elif _MSC_VER
  __debugbreak();
#elif __unix__
  raise( SIGTRAP );
#else
  #error ql::breakpoint() is unimplemented for your compiler.
#endif
}

}
