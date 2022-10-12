// ----------------------------------------------------------------------------
// @file    usf_argument.hpp
// @brief   Argument format processor class.
// @date    14 January 2019
// ----------------------------------------------------------------------------

#pragma once

namespace usf
{
namespace internal
{

class Argument
{
    public:

        // --------------------------------------------------------------------
        // PUBLIC TYPE ALIASES
        // --------------------------------------------------------------------

        using       iterator =       CharType*;
        using const_iterator = const CharType*;

        // --------------------------------------------------------------------
        // PUBLIC MEMBER FUNCTIONS
        // --------------------------------------------------------------------

        constexpr Argument() = delete;

        constexpr Argument(const bool value) noexcept
            : m_bool(value), m_type_id(TypeId::kBool) {}

        constexpr Argument(const CharType value) noexcept
            : m_char(value), m_type_id(TypeId::kChar) {}

        constexpr Argument(const int32_t value) noexcept
            : m_int32(value), m_type_id(TypeId::kInt32) {}

        constexpr Argument(const uint32_t value) noexcept
            : m_uint32(value), m_type_id(TypeId::kUint32) {}

        constexpr Argument(const int64_t value) noexcept
            : m_int64(value), m_type_id(TypeId::kInt64) {}

        constexpr Argument(const uint64_t value) noexcept
            : m_uint64(value), m_type_id(TypeId::kUint64) {}
    
        Argument(const void* value) noexcept
            : m_pointer(reinterpret_cast<std::uintptr_t>(value)), m_type_id(TypeId::kPointer) {}

#if !defined(USF_DISABLE_FLOAT_SUPPORT)
        constexpr Argument(const double value) noexcept
            : m_float(value), m_type_id(TypeId::kFloat) {}
#endif
        constexpr Argument(const usf::StringView value) noexcept
            : m_string(value), m_type_id(TypeId::kString) {}

        constexpr Argument(const ArgCustomType value) noexcept
            : m_custom(value), m_type_id(TypeId::kCustom) {}

        [[nodiscard]] error format(usf::StringSpan& dst, ArgFormat& format) const;

    private:

        // --------------------------------------------------------------------
        // PRIVATE STATIC FUNCTIONS
        // --------------------------------------------------------------------

        static error format_bool(iterator& it, const_iterator end,
                                                    const ArgFormat& format, const bool value);
      
        static error format_char(iterator& it, const_iterator end,
                                                    ArgFormat& format, const CharType value);
     

        template <typename T, typename std::enable_if<std::is_signed<T>::value, bool>::type = true>
        static error format_integer(iterator& it, const_iterator end,
                                                       const ArgFormat& format, const T value)
        {
            using unsigned_type = typename std::make_unsigned<T>::type;

            const bool negative = (value < 0);
            const auto uvalue = static_cast<unsigned_type>(negative ? -value : value);

            return format_integer(it, end, format, uvalue, negative);
        }

        template <typename T, typename std::enable_if<std::is_unsigned<T>::value, bool>::type = true>
        static error format_integer(iterator& it, const_iterator end, const ArgFormat& format,
                                                       const T value, const bool negative = false)
        {
            int fill_after = 0;

            if(format.type_is_none() || format.type_is_integer_dec())
            {
                const auto digits = Integer::count_digits_dec(value);
                fill_after = format.write_alignment(it, end, digits, negative);
                if(fill_after<0)
                    return error::buf_overflow;
                it += digits;
                Integer::convert_dec(it, value);
            }
            else if(format.type_is_integer_hex())
            {
                const auto digits = Integer::count_digits_hex(value);
                fill_after = format.write_alignment(it, end, digits, negative);
                if(fill_after<0)
                    return error::buf_overflow;
                it += digits;
                Integer::convert_hex(it, value, format.uppercase());
            }
            else if(format.type_is_integer_oct())
            {
                const auto digits = Integer::count_digits_oct(value);
                fill_after = format.write_alignment(it, end, digits, negative);
                if(fill_after<0)
                    return error::buf_overflow;
                it += digits;
                Integer::convert_oct(it, value);
            }
            else if(format.type_is_integer_bin())
            {
                const auto digits = Integer::count_digits_bin(value);
                fill_after = format.write_alignment(it, end, digits, negative);
                if(fill_after<0)
                    return error::buf_overflow;
                it += digits;
                Integer::convert_bin(it, value);
            }
            else
            {
                // Argument type / format mismatch
                //USF_CONTRACT_VIOLATION(std::runtime_error);
                return error::arg_type;
            }

            CharTraits::assign(it, format.fill_char(), fill_after);
            return error::success;
        }


        error format_pointer(iterator& it, const_iterator end,
                                                       const ArgFormat& format, const std::uintptr_t value) const;

#if !defined(USF_DISABLE_FLOAT_SUPPORT)
        static 
        error format_float(iterator& it, const_iterator end, const ArgFormat& format, double value) ;

        static 
        void write_float_exponent(iterator& it, int exponent, const bool uppercase) noexcept ;

        static 
        void format_float_zero(iterator& it, const_iterator end, const ArgFormat& format, const bool negative);

#endif // !defined(USF_DISABLE_FLOAT_SUPPORT)

        error format_string(iterator& it, const_iterator end,
                                                      ArgFormat& format, const usf::StringView& str) const;
       
        
        template <typename CharSrc,
                  typename std::enable_if<std::is_convertible<CharSrc, CharType>::value, bool>::type = true>
        static error format_string(iterator& it, const_iterator end,
                                                      const ArgFormat& format, const CharSrc* str,
                                                      const int str_length, const bool negative = false)
        {
            const int fill_after = format.write_alignment(it, end, str_length, negative);
            if(fill_after<0)
                return error::buf_overflow;

            CharTraits::copy(it, str, str_length);
            CharTraits::assign(it, format.fill_char(), fill_after);
            return error::success;
        }

        // --------------------------------------------------------------------
        // PRIVATE MEMBER VARIABLES
        // --------------------------------------------------------------------

        enum class TypeId : unsigned char
        {
            kBool = 0,
            kChar,
            kInt32,
            kUint32,
            kInt64,
            kUint64,
            kPointer,
#if !defined(USF_DISABLE_FLOAT_SUPPORT)
            kFloat,
#endif
            kString,
            kCustom
        };

        union
        {
            bool                            m_bool;
            CharType                        m_char;
            int32_t                         m_int32;
            uint32_t                        m_uint32;
            int64_t                         m_int64;
            uint64_t                        m_uint64;
            std::uintptr_t                  m_pointer;
#if !defined(USF_DISABLE_FLOAT_SUPPORT)
            double                          m_float;
#endif
            usf::StringView                 m_string;
            ArgCustomType                   m_custom;
        };

        TypeId                              m_type_id;
};




// Boolean
inline
Argument make_argument(const bool arg)
{
    return arg;
}

// Character (char)
inline 
Argument make_argument(const char arg)
{
    return static_cast<CharType>(arg);
}

#if 0
// Character (CharT != char)
template <typename CharT, typename std::enable_if<!std::is_same<CharT, char>::value, bool>::type = true>
inline constexpr
Argument<CharT> make_argument(const CharT arg)
{
    return arg;
}
#endif

// 8 bit signed integer
inline constexpr
Argument make_argument(const int8_t arg)
{
    return static_cast<int32_t>(arg);
}

// 8 bit unsigned integer
inline constexpr
Argument make_argument(const uint8_t arg)
{
    return static_cast<uint32_t>(arg);
}

// 16 bit signed integer
inline constexpr
Argument make_argument(const int16_t arg)
{
    return static_cast<int32_t>(arg);
}

// 16 bit unsigned integer
inline constexpr
Argument make_argument(const uint16_t arg)
{
    return static_cast<uint32_t>(arg);
}

// 32 bit signed integer
inline constexpr
Argument make_argument(const int arg)
{
    return static_cast<int32_t>(arg);
}

// 32 bit unsigned integer
inline constexpr
Argument make_argument(const unsigned int arg)
{
    return static_cast<uint32_t>(arg);
}

#if (__LONG_MAX__ != __LONG_LONG_MAX__)

// 32 bit signed integer
inline constexpr
Argument make_argument(const long int arg)
{
    return static_cast<int32_t>(arg);
}

// 32 bit unsigned integer
inline constexpr
Argument make_argument(const unsigned long int arg)
{
    return static_cast<uint32_t>(arg);
}
#else // (__LONG_MAX__ != __LONG_LONG_MAX__)

// 64 bit signed integer
inline constexpr
Argument make_argument(const long int arg)
{
    return static_cast<int64_t>(arg);
}

// 64 bit unsigned integer
inline constexpr
Argument make_argument(const unsigned long int arg)
{
    return static_cast<uint64_t>(arg);
}
#endif // (__LONG_MAX__ != __LONG_LONG_MAX__)

// 64 bit signed integer
inline constexpr
Argument make_argument(const int64_t arg)
{
    if(arg >= std::numeric_limits<int32_t>::min()
    && arg <= std::numeric_limits<int32_t>::max())
    {
        return static_cast<int32_t>(arg);
    }

    return arg;
}

// 64 bit unsigned integer
inline constexpr
Argument make_argument(const uint64_t arg)
{
    if(arg <= std::numeric_limits<uint32_t>::max())
    {
        return static_cast<uint32_t>(arg);
    }

    return arg;
}

// Pointer (void*)
inline
Argument make_argument(void* arg)
{
    return arg;
}

// Pointer (const void*)
inline
Argument make_argument(const void* arg)
{
    return arg;
}

#if !defined(USF_DISABLE_FLOAT_SUPPORT)
// Floating point (float)
inline
Argument make_argument(float arg)
{
    return static_cast<double>(arg);
}

// Floating point (double)
inline
Argument make_argument(double arg)
{
    return arg;
}
#endif // !defined(USF_DISABLE_FLOAT_SUPPORT)

// String (convertible to string view)
template <typename T,
          typename std::enable_if<std::is_convertible<T, usf::StringView>::value, bool>::type = true>
inline Argument make_argument(const T& arg)
{
    return usf::StringView(arg);
}


} // namespace internal

// User-defined custom type formatter forward declaration
template <typename T>
struct Formatter
{
    static result_t format_to(StringSpan, const T&);
};

namespace internal
{

// User-defined custom type
template <typename T,
          typename std::enable_if<!std::is_convertible<T, usf::StringView>::value, bool>::type = true>
inline Argument make_argument(const T& arg)
{
    using _T = typename std::decay<decltype(arg)>::type;

    return ArgCustomType::template create<_T, &usf::Formatter<_T>::format_to>(&arg);
}

} // namespace internal
} // namespace usf

