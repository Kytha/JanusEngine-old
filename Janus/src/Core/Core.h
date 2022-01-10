#pragma once

#include <stdio.h>
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
  #define JN_PLATFORM_WINDOWS
  #if defined(__MINGW32__)  // Defined for both 32 bit/64 bit MinGW
    #define JN_PLATFORM_MINGW
  #elif defined(_MSC_VER)
    #define JN_PLATFORM_MSVC
  #endif
#elif defined(__linux__)
  #define JN_PLATFORM_LINUX
#elif defined(__APPLE__)
  #define JN_PLATFORM_OSX
#elif defined(__OpenBSD__)
  #define JN_PLATFORM_OPENBSD
#elif defined(__FreeBSD__)
  #define JN_PLATFORM_FREEBSD
#endif

#if defined(JN_PLATFORM_WINDOWS)
    #define JN_ABORT __debugbreak();
#else
    #define JN_ABORT abort();
#endif

#define BIT(x) (1 << x)


static const float Pi      = 3.14159265358979323846f;
static const float InvPi   = 0.31830988618379067154f;
static const float Inv2Pi  = 0.15915494309189533577f;
static const float Inv4Pi  = 0.07957747154594766788f;
static const float PiOver2 = 1.57079632679489661923f;
static const float PiOver4 = 0.78539816339744830961f;
static const float Sqrt2   = 1.41421356237309504880f;


inline float Radians(float deg) { 
    return (Pi / 180) * deg; 
}
inline float Degrees(float rad) { 
    return (180 / Pi) * rad; 
}

#include "Core/Ref.h"
namespace Janus {

  template<typename T>
  using Scope = std::unique_ptr<T>;
  template<typename T, typename ... Args>
  constexpr Scope<T> CreateScope(Args&& ... args)
  {
      return std::make_unique<T>(std::forward<Args>(args)...);
  }
}

using byte = uint8_t;
