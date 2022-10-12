// ----------------------------------------------------------------------------
// @file    usf_integer.hpp
// @brief   Integer conversion and helper functions.
// @date    14 January 2019
// ----------------------------------------------------------------------------

#pragma once

namespace usf::internal::Integer
{
    int count_digits_dec(const uint32_t n) noexcept;
    int count_digits_dec(const uint64_t n) noexcept;
    int count_digits_bin(const uint32_t n) noexcept;
    int count_digits_bin(const uint64_t n) noexcept;

    template <typename T,
                  typename std::enable_if<std::numeric_limits<T>::is_integer && std::is_unsigned<T>::value, bool>::type = true>
        static constexpr int count_digits_oct(T n) noexcept
        {
            int digits = 1;
            while((n >>= 3U) != 0) { ++digits; }
            return digits;
        }

    template <typename T,
                  typename std::enable_if<std::numeric_limits<T>::is_integer && std::is_unsigned<T>::value, bool>::type = true>
        static constexpr int count_digits_hex(T n) noexcept
        {
            int digits = 1;
            while((n >>= 4U) != 0) { ++digits; }
            return digits;
        }

    
    uint32_t pow10_uint32(const int index) noexcept;
    uint64_t pow10_uint64(const int index) noexcept;

    void convert_dec(CharType* dst, uint32_t value) noexcept;
    void convert_dec(CharType* dst, uint64_t value) noexcept;
    void convert_bin(CharType* dst, uint32_t value) noexcept;
    void convert_bin(CharType* dst, uint64_t value) noexcept;
    void convert_oct(CharType* dst, uint32_t value) noexcept;
    void convert_oct(CharType* dst, uint64_t value) noexcept;
    void convert_hex(CharType* dst, uint32_t value, const bool uppercase) noexcept;
    void convert_hex(CharType* dst, uint64_t value, const bool uppercase) noexcept;

} // namespace usf

