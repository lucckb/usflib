// ----------------------------------------------------------------------------
// @file    usf_config.hpp
// @brief   usflib configuration header file.
// @date    14 January 2019
// ----------------------------------------------------------------------------

#ifndef USF_CONFIG_HPP
#define USF_CONFIG_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <limits>
#include <string>
#include <type_traits>
#if defined(USF_THROW_ON_CONTRACT_VIOLATION)
#include <stdexcept>
#endif

// ----------------------------------------------------------------------------
// usflib configuration options
// ----------------------------------------------------------------------------

// Configuration of floating point support.
// USF_DISABLE_FLOAT_SUPPORT           : disables the support of floating point types (it will save considerable code size)

// Configuration of format output string termination option.
// USF_DISABLE_STRING_TERMINATION      : disables the null termination of the format output string

// Configuration of possible behavior when a condition is violated.
// USF_TERMINATE_ON_CONTRACT_VIOLATION : std::terminate() will be called (default)
// USF_ABORT_ON_CONTRACT_VIOLATION     : std::abort() will be called (more suitable for embedded platforms, maybe?)
// USF_THROW_ON_CONTRACT_VIOLATION     : an exception will be thrown


// ----------------------------------------------------------------------------
// Compiler version detection
// ----------------------------------------------------------------------------

#define USF_COMPILER_VERSION(major, minor, patch) (10 * (10 * (major) + (minor)) + (patch))

#if defined(__clang__)
#  define USF_COMPILER_CLANG
#  define USF_CLANG_VERSION  USF_COMPILER_VERSION(__clang_major__, __clang_minor__, __clang_patchlevel__)
#  if (USF_CLANG_VERSION < 340)
#    error usflib requires Clang 3.4.0 or greater
#  endif
#endif

#if defined(__GNUC__) && !defined(__clang__)
#  define USF_COMPILER_GCC
#  define USF_GCC_VERSION  USF_COMPILER_VERSION(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#  if (USF_GCC_VERSION < 481)
#    error usflib requires GCC 4.8.1 or greater
#  endif
#endif


// ----------------------------------------------------------------------------
// Error handling
// ----------------------------------------------------------------------------
// Two macros ensures any macro passed will
// be expanded before being stringified.
#define USF_STRINGIFY_DETAIL(x)  #x
#define USF_STRINGIFY(x)         USF_STRINGIFY_DETAIL(x)

#if defined(USF_THROW_ON_CONTRACT_VIOLATION)

namespace usf
{
namespace internal
{
template <typename Except>
[[noreturn]] USF_ALWAYS_INLINE constexpr
void throw_exception(const char* const msg)
{
    static_assert(std::is_convertible<Except, std::exception>::value,
                  "usf::throw_exception(): exception type should inherit from std::exception.");

    throw Except(msg);
}
} // namespace internal
} // namespace usf

#  define USF_CONTRACT_VIOLATION(except)  usf::internal::throw_exception<except>("Failure at " __FILE__ ", Line " USF_STRINGIFY(__LINE__))
#elif defined(USF_ABORT_ON_CONTRACT_VIOLATION)
#  define USF_CONTRACT_VIOLATION(except)  std::abort()
#elif defined(USF_TERMINATE_ON_CONTRACT_VIOLATION)
#  define USF_CONTRACT_VIOLATION(except)  std::terminate()
#else
namespace usf::internal {
    void contract_violation(const char* str);
}
#  define USF_CONTRACT_VIOLATION(except)  usf::internal::contract_violation("Failure at " __FILE__ ", Line " USF_STRINGIFY(__LINE__))

#endif

#define USF_ENFORCE(cond, except)  ((!!(cond)) ? static_cast<void>(0) : USF_CONTRACT_VIOLATION(except))


// Char type definiton

namespace usf
{
    using CharType = char;
}

#endif // USF_CONFIG_HPP
