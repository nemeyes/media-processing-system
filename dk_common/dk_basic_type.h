#ifndef _DK_BASIC_TYPE_H_
#define _DK_BASIC_TYPE_H_

#include <limits.h>         // So we can set the bounds of our types
#include <stddef.h>         // For size_t
#include <string.h>         // for memcpy

#include <stdint.h>

#ifdef COMPILER_MSVC
#define GG_LONGLONG(x) x##I64
#define GG_ULONGLONG(x) x##UI64
#else
#define GG_LONGLONG(x) x##LL
#define GG_ULONGLONG(x) x##ULL
#endif

const uint8_t  uint8_max = ((uint8_t)0xFF);
const uint16_t uint16_max = ((uint16_t)0xFFFF);
const uint32_t uint32_max = ((uint32_t)0xFFFFFFFF);
const uint64_t uint64_max = ((uint64_t)GG_LONGLONG(0xFFFFFFFFFFFFFFFF));
const int8_t  int8_min = ((int8_t)0x80);
const int8_t  int8_max = ((int8_t)0x7F);
const int16_t int16_min = ((int16_t)0x8000);
const int16_t int16_max = ((int16_t)0x7FFF);
const int32_t int32_min = ((int32_t)0x80000000);
const int32_t int32_max = ((int32_t)0x7FFFFFFF);
const int64_t int64_min = ((int64_t)GG_LONGLONG(0x8000000000000000));
const int64_t int64_max = ((int64_t)GG_LONGLONG(0x7FFFFFFFFFFFFFFF));

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName();                                    \
  DISALLOW_COPY_AND_ASSIGN(TypeName)

// Use implicit_cast as a safe version of static_cast or const_cast
// for upcasting in the type hierarchy (i.e. casting a pointer to Foo
// to a pointer to SuperclassOfFoo or casting a pointer to Foo to
// a const pointer to Foo).
// When you use implicit_cast, the compiler checks that the cast is safe.
// Such explicit implicit_casts are necessary in surprisingly many
// situations where C++ demands an exact type match instead of an
// argument type convertable to a target type.
//
// The From type can be inferred, so the preferred syntax for using
// implicit_cast is the same as for static_cast etc.:
//
//   implicit_cast<ToType>(expr)
//
// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.
template<typename To, typename From>
inline To implicit_cast(From const &f) 
{
	return f;
}

#endif