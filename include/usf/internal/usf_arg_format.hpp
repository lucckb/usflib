// ----------------------------------------------------------------------------
// @file    usf_arg_format.hpp
// @brief   Argument format parser class.
// @date    07 January 2019
// ----------------------------------------------------------------------------

#pragma once

namespace usf::internal {

class ArgFormat
{
    public:

        // --------------------------------------------------------------------
        // PUBLIC TYPE ALIASES
        // --------------------------------------------------------------------

        using iterator       = CharType*;
        using const_iterator = const CharType*;

        // --------------------------------------------------------------------
        // PUBLIC DEFINITIONS
        // --------------------------------------------------------------------

        enum class Align : uint8_t
        {
            kNone    = (0U << 1U),
            kLeft    = (1U << 1U),
            kRight   = (2U << 1U),
            kCenter  = (3U << 1U),
            kNumeric = (4U << 1U)
        };

        enum class Sign : uint8_t
        {
            kNone    = (0U << 4U),
            kMinus   = (1U << 4U),
            kPlus    = (2U << 4U),
            kSpace   = (3U << 4U)
        };

        enum class Type : uint8_t
        {
            kNone,
            kChar,
            kIntegerDec,
            kIntegerHex,
            kIntegerOct,
            kIntegerBin,
            kPointer,
            kFloatFixed,
            kFloatScientific,
            kFloatGeneral,
            kString,
            kInvalid
        };

        // --------------------------------------------------------------------
        // PUBLIC MEMBER FUNCTIONS
        // --------------------------------------------------------------------
       ArgFormat(usf::StringView& fmt, const int arg_count);

        // Writes the alignment (sign, prefix and fill before) for any
        // argument type. Returns the fill counter to write after argument.
        int write_alignment(iterator& it, const_iterator end,
                                                int digits, const bool negative) const;

        inline constexpr CharType fill_char() const noexcept { return m_fill_char;                    }
        inline constexpr Type  type     () const noexcept { return m_type;                         }
        inline constexpr int   width    () const noexcept { return static_cast<int>(m_width    );  }
        inline constexpr int   precision() const noexcept { return static_cast<int>(m_precision);  }
        inline constexpr int   index    () const noexcept { return static_cast<int>(m_index    );  }

        inline constexpr Align align() const noexcept { return Align(m_flags & Flags::kAlignBitmask); }
        inline constexpr Sign  sign () const noexcept { return Sign (m_flags & Flags::kSignBitmask ); }

        inline constexpr bool  is_empty () const noexcept { return (m_flags & Flags::kEmpty    ) != 0; }
        inline constexpr bool  hash     () const noexcept { return (m_flags & Flags::kHash     ) != 0; }
        inline constexpr bool  uppercase() const noexcept { return (m_flags & Flags::kUppercase) != 0; }

        inline constexpr bool type_is_none   () const noexcept { return m_type == Type::kNone;    }
        inline constexpr bool type_is_char   () const noexcept { return m_type == Type::kChar;    }
        inline constexpr bool type_is_string () const noexcept { return m_type == Type::kString;  }
        inline constexpr bool type_is_pointer() const noexcept { return m_type == Type::kPointer; }

        inline constexpr bool type_is_integer() const noexcept { return m_type >= Type::kIntegerDec && m_type <= Type::kIntegerBin;   }
        inline constexpr bool type_is_float  () const noexcept { return m_type >= Type::kFloatFixed && m_type <= Type::kFloatGeneral; }
        inline constexpr bool type_is_numeric() const noexcept { return m_type >= Type::kIntegerDec && m_type <= Type::kFloatGeneral; }

        inline constexpr bool type_is_integer_dec() const noexcept { return m_type == Type::kIntegerDec; }
        inline constexpr bool type_is_integer_hex() const noexcept { return m_type == Type::kIntegerHex; }
        inline constexpr bool type_is_integer_oct() const noexcept { return m_type == Type::kIntegerOct; }
        inline constexpr bool type_is_integer_bin() const noexcept { return m_type == Type::kIntegerBin; }

        inline constexpr bool type_is_float_fixed     () const noexcept { return m_type == Type::kFloatFixed;      }
        inline constexpr bool type_is_float_scientific() const noexcept { return m_type == Type::kFloatScientific; }
        inline constexpr bool type_is_float_general   () const noexcept { return m_type == Type::kFloatGeneral;    }

        inline constexpr bool type_allow_hash() const noexcept
        {
            // Alternative format is valid for hexadecimal (including
            // pointers), octal, binary and all floating point types.
            return m_type >= Type::kIntegerHex && m_type <= Type::kFloatGeneral;
        }

        inline constexpr void default_align_left() noexcept
        {
            if((m_flags & Flags::kAlignBitmask) == Flags::kAlignNone)
            {
                m_flags |= Flags::kAlignLeft;
            }
        }

        inline constexpr auto ec_error() const {
            return m_error;
        }

    private:

        // --------------------------------------------------------------------
        // PRIVATE DEFINITIONS
        // --------------------------------------------------------------------

        enum Flags : uint8_t
        {
            kNone         = (0U << 0U),

            kEmpty        = (1U << 0U),

            kAlignNone    = (0U << 1U),
            kAlignLeft    = (1U << 1U),
            kAlignRight   = (2U << 1U),
            kAlignCenter  = (3U << 1U),
            kAlignNumeric = (4U << 1U),
            kAlignBitmask = (7U << 1U),

            kSignNone     = (0U << 4U),
            kSignMinus    = (1U << 4U),
            kSignPlus     = (2U << 4U),
            kSignSpace    = (3U << 4U),
            kSignBitmask  = (3U << 4U),

            kHash         = (1U << 6U),

            kUppercase    = (1U << 7U)
        };

        // --------------------------------------------------------------------
        // PRIVATE MEMBER FUNCTIONS
        // --------------------------------------------------------------------

        inline constexpr int sign_width(const bool negative) const noexcept
        {
            return (!negative && sign() <= Sign::kMinus) ? 0 : 1;
        }

        inline constexpr int prefix_width() const noexcept
        {
            // Alternative format is valid for hexadecimal (including
            // pointers), octal, binary and all floating point types.
            return (!hash() || type_is_float()) ? 0 : type_is_integer_oct() ? 1 : 2;
        }

        void write_sign(iterator& it, const bool negative) const noexcept;
       

        void write_prefix(iterator& it) const noexcept;

        // --------------------------------------------------------------------
        // PRIVATE STATIC FUNCTIONS
        // --------------------------------------------------------------------

        // Parses the input as a positive integer that fits into a `uint8_t` type. This
        // function assumes that the first character is a digit and terminates parsing
        // at the presence of the first non-digit character or when value overflows.
        static 
        int parse_positive_small_int(const_iterator& it, const int max_value);

        static uint8_t parse_align_flag(const CharType ch) noexcept;

        // --------------------------------------------------------------------
        // PRIVATE MEMBER VARIABLES
        // --------------------------------------------------------------------

        CharType   m_fill_char = ' ';
        Type    m_type     = Type::kNone;
        uint8_t m_flags    = Flags::kEmpty;
        uint8_t m_width    =  0;
        int8_t m_precision = -1;
        int8_t m_index     = -1;
        mutable error m_error      = error::success;
};

} // namespace usf

