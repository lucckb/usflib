      #include <usf/usf.hpp>

namespace usf::internal {

       ArgFormat::ArgFormat(usf::StringView& fmt, const int arg_count)
        {
            const_iterator it = fmt.cbegin();

            //USF_ENFORCE(*it == '{', std::runtime_error);
            if( *it != '{') {
                m_error = error::arg_enclose;
                return;
            }

            // Iterator is placed at "{" character, so advance it.
            ++it;

            // Parse argument index
            if(*it >= '0' && *it <= '9')
            {
                // Index limited to `arg_count` value.
                const auto ret = parse_positive_small_int(it, arg_count);
                if(ret<0) {
                    m_error = error(ret);
                    return;
                }
                m_index = static_cast<int8_t>(ret);
            }

            if(*it == ':' && *(it + 1) != '}')
            {
                // A format spec is expected next...

                //Remove the empty format flag
                m_flags = Flags::kNone;

                // Advance ':' character
                ++it;

                // Try to parse alignment flag at second character of format spec.
                m_flags = parse_align_flag(*(it + 1));

                if(m_flags != Flags::kNone)
                {
                    // Alignment flag present at second character of format spec.
                    // Should also have a fill character at the first character.

                    // The fill character can be any character except '{' or '}'.
                    //USF_ENFORCE(*it != '{' && *it != '}', std::runtime_error);
                    if(!(*it != '{' && *it != '}')) {
                        m_error = error::arg_enclose;
                        return;
                    }
                    m_fill_char = *it;
                    it += 2;
                }
                else
                {
                    // Alignment flag not present at the second character of format spec.
                    // Try to parse the alignment flag at the first character instead...
                    m_flags = parse_align_flag(*it);

                    if(m_flags != Flags::kNone)
                    {
                        ++it;
                    }
                }

                // Parse sign flag
                switch(*it)
                {
                    case '-': m_flags |= Flags::kSignMinus; ++it; break;
                    case '+': m_flags |= Flags::kSignPlus;  ++it; break;
                    case ' ': m_flags |= Flags::kSignSpace; ++it; break;
                    default :                                     break;
                }

                // Parse hash flag
                if(*it == '#')
                {
                    m_flags |= Flags::kHash;
                    ++it;
                }

                bool fill_zero = false;

                // Parse fill zero flag
                if(*it == '0')
                {
                    fill_zero = true;
                    ++it;
                }

                // Parse width
                if(*it >= '0' && *it <= '9')
                {
                    // Limit width to 255 characters
                    m_width = parse_positive_small_int(it, 255);
                }

                // Parse precision
                if(*it == '.')
                {
                    ++it;

                    // Check for a missing/invalid precision specifier.
                    //USF_ENFORCE(*it >= '0' && *it <= '9', std::runtime_error);
                    if(!(*it >= '0' && *it <= '9')) {
                        m_error = error::inv_prec_specifier;
                        return;
                    }

                    m_precision = static_cast<int8_t>(parse_positive_small_int(it, 127));
                }

                // Parse type
                if(*it != '}')
                {
                    switch(*it++)
                    {
                        case 'c': m_type = Type::kChar;            break;

                        case 'd': m_type = Type::kIntegerDec;      break;

                        case 'X': m_flags |= Flags::kUppercase;    [[fallthrough]];
                        case 'x': m_type = Type::kIntegerHex;      break;

                        case 'o': m_type = Type::kIntegerOct;      break;

                        case 'B': m_flags |= Flags::kUppercase;    [[fallthrough]];
                        case 'b': m_type = Type::kIntegerBin;      break;

                        case 'P': m_flags |= Flags::kUppercase;    [[fallthrough]];
                        case 'p': m_type = Type::kPointer;         break;

                        case 'F': m_flags |= Flags::kUppercase;    [[fallthrough]];
                        case 'f': m_type = Type::kFloatFixed;      break;

                        case 'E': m_flags |= Flags::kUppercase;    [[fallthrough]];
                        case 'e': m_type = Type::kFloatScientific; break;

                        case 'G': m_flags |= Flags::kUppercase;    [[fallthrough]];
                        case 'g': m_type = Type::kFloatGeneral;    break;

                        case 's': m_type = Type::kString;          break;

                        default : m_type = Type::kInvalid;         break;
                    }

                    //USF_ENFORCE(m_type != Type::kInvalid, std::runtime_error);
                    if(!(m_type != Type::kInvalid)) {
                        m_error = error::inv_type;
                        return;
                    }
                }

                // Validate the read format spec!

                if(fill_zero)
                {
                    // Fill zero flag has precedence over any other alignment and fill character.
                    m_flags = static_cast<uint8_t>((m_flags & (~Flags::kAlignBitmask)) | Flags::kAlignNumeric);
                    m_fill_char = '0';
                }

                if(align() == Align::kNumeric)
                {
                    // Numeric alignment are only valid for numeric and pointer types.
                    //USF_ENFORCE(type_is_numeric() || type_is_pointer(), std::runtime_error);
                    if(!(type_is_numeric() || type_is_pointer())) {
                        m_error = error::inv_format;
                        return;
                    }
                }

                if(sign() != Sign::kNone)
                {
                    // Sign is only valid for numeric types.
                    //USF_ENFORCE(type_is_numeric(), std::runtime_error);
                    if(!(type_is_numeric())) {
                        m_error = error::inv_sign;
                        return;
                    }
                }

                if(hash())
                {
                    // Alternative format is valid for hexadecimal (including
                    // pointers), octal, binary and all floating point types.
                    //USF_ENFORCE(type_allow_hash(), std::runtime_error);
                    if(!(type_allow_hash())) {
                        m_error = error::inv_format;
                        return;
                    }
                }

                if(m_precision != -1)
                {
                    // Precision is only valid for floating point and string types.
                    //USF_ENFORCE(type_is_float() || type_is_string(), std::runtime_error);
                    if(!(type_is_float() || type_is_string())) {
                        m_error = error::inv_format;
                        return;
                    }
                }
            }

            // Test for unterminated argument format spec.
            //USF_ENFORCE(it < fmt.cend() && *it++ == '}', std::runtime_error);
            if(!(it < fmt.cend() && *it++ == '}')) {
                m_error = error::arg_enclose;
                return;
            }
            fmt.remove_prefix(it - fmt.cbegin());
        }

     // Writes the alignment (sign, prefix and fill before) for any
        // argument type. Returns the fill counter to write after argument.
        int ArgFormat::write_alignment(iterator& it, const_iterator end,
                                                int digits, const bool negative) const
        {
            digits += sign_width(negative) + prefix_width();

            int fill_after = 0;

            if(width() <= digits)
            {
                //USF_ENFORCE(it + digits < end, std::runtime_error);
                if(it + digits >= end) {
                    m_error = error::buf_overflow;
                    return -1;
                }
                write_sign(it, negative);
                write_prefix(it);
            }
            else
            {
                //USF_ENFORCE(it + width() < end, std::runtime_error);
                if(it + width() >= end) {
                    m_error = error::buf_overflow;
                    return -1;
                }

                int fill_count = width() - digits;

                const Align al = align();

                if(al == Align::kLeft)
                {
                    fill_after = fill_count;
                }
                else if(al == Align::kCenter)
                {
                    fill_after = fill_count - (fill_count / 2);
                    fill_count /= 2;
                }

                if(al != Align::kLeft && al != Align::kNumeric)
                {
                    // None (default right), Right or Center alignment
                    CharTraits::assign(it, fill_char(), fill_count);
                }

                write_sign(it, negative);
                write_prefix(it);

                if(al == Align::kNumeric)
                {
                    CharTraits::assign(it, fill_char(), fill_count);
                }
            }

            return fill_after;
        }

      void ArgFormat::write_sign(iterator& it, const bool negative) const noexcept
        {
            if(negative)
            {
                *it++ = '-';
            }
            else
            {
                const Sign s = sign();

                if(s != Sign::kNone)
                {
                    if(s == Sign::kPlus )
                    {
                        *it++ = '+';
                    }
                    else if(s == Sign::kSpace)
                    {
                        *it++ = ' ';
                    }
                }
            }
        }

      void ArgFormat::write_prefix(iterator& it) const noexcept
      {
            // Alternative format is valid for hexadecimal (including
            // pointers), octal, binary and all floating point types.
            if(hash() && !type_is_float())
            {
                *it++ = '0';

                if(type_is_integer_bin())
                {
                    *it++ = uppercase() ? 'B' : 'b';
                }
                else if(type_is_integer_hex() || type_is_pointer())
                {
                    *it++ = uppercase() ? 'X' : 'x';
                }
            }
      }

      // Parses the input as a positive integer that fits into a `uint8_t` type. This
        // function assumes that the first character is a digit and terminates parsing
        // at the presence of the first non-digit character or when value overflows.
        int ArgFormat::parse_positive_small_int(const_iterator& it, const int max_value)
        {
            assert(max_value < 256);

            int value = 0;

            do
            {
                value = (value * 10) + static_cast<int>(*it++ - '0');

                // Check for overflow
                //USF_ENFORCE(value <= max_value, std::runtime_error);
                if(!(value <= max_value)) {
                    return int(error::value_overflow);
                }
            }while(*it >= '0' && *it <= '9');

            return static_cast<uint8_t>(value);
        }

        uint8_t ArgFormat::parse_align_flag(const CharType ch) noexcept
        {
            switch(ch)
            {
                case '<': return Flags::kAlignLeft;    break;
                case '>': return Flags::kAlignRight;   break;
                case '^': return Flags::kAlignCenter;  break;
                case '=': return Flags::kAlignNumeric; break;
                default : return Flags::kNone;         break;
            }
        }

}  