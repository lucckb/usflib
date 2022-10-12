#include <usf/usf.hpp>

namespace usf::internal {

        error Argument::format(usf::StringSpan& dst, ArgFormat& format) const
        {
            if(format.ec_error()!=error::success) {
                return format.ec_error();
            }
            iterator it = dst.begin();
            error ret { error::success };
            switch(m_type_id)
            {
                case TypeId::kBool:    ret = format_bool   (it, dst.end(), format, m_bool   ); break;
                case TypeId::kChar:    ret = format_char   (it, dst.end(), format, m_char   ); break;
                case TypeId::kInt32:   ret = format_integer(it, dst.end(), format, m_int32  ); break;
                case TypeId::kUint32:  ret = format_integer(it, dst.end(), format, m_uint32 ); break;
                case TypeId::kInt64:   ret = format_integer(it, dst.end(), format, m_int64  ); break;
                case TypeId::kUint64:  ret = format_integer(it, dst.end(), format, m_uint64 ); break;
                case TypeId::kPointer: ret = format_pointer(it, dst.end(), format, m_pointer); break;
#if !defined(USF_DISABLE_FLOAT_SUPPORT)
                case TypeId::kFloat:   ret = format_float  (it, dst.end(), format, m_float  ); break;
#endif
                case TypeId::kString:  ret = format_string (it, dst.end(), format, m_string ); break;
                /*
                case TypeId::kCustom:  USF_ENFORCE(format.is_empty(), std::runtime_error);
                                       it = m_custom(dst).end();                         break;
                */
                case TypeId::kCustom:  if(format.is_empty()) {
                                        auto [err,span] = m_custom(dst);
                                        ret = err;
                                        if(ret==error::success) 
                                            it = span.end();
                                       } else {
                                         ret = error::no_custom_format;
                                       }
                                       break;
            }

            if(ret==error::success) {
                dst.remove_prefix(it - dst.begin());
            }
            return ret;
        }

        error Argument::format_bool(iterator& it, const_iterator end,
                                                    const ArgFormat& format, const bool value)
        {
            if(format.ec_error()!=error::success) {
                return format.ec_error();
            }
            if(format.type_is_none())
            {
                return format_string(it, end, format, value ? "true" : "false", value ? 4 : 5);
            }
            else if(format.type_is_integer())
            {
                return format_integer(it, end, format, static_cast<uint32_t>(value));
            }
            else
            {
                // Argument type / format mismatch
                //USF_CONTRACT_VIOLATION(std::runtime_error);
                return error::type_mismatch;
            }
        }

        error Argument::format_char(iterator& it, const_iterator end,
                                                    ArgFormat& format, const CharType value)
        {
            if(format.ec_error()!=error::success) {
                return format.ec_error();
            }
            if(format.type_is_none() || format.type_is_char())
            {
                // Characters and strings align to left by default.
                format.default_align_left();

                const int fill_after = format.write_alignment(it, end, 1, false);
                if(fill_after<0) {
                    return error::buf_overflow;
                }
                *it++ = value;
                CharTraits::assign(it, format.fill_char(), fill_after);
                return error::success;
            }
            else if(format.type_is_integer())
            {
                return format_integer(it, end, format, static_cast<int32_t>(value));
            }
            else
            {
                // Argument type / format mismatch
                //USF_CONTRACT_VIOLATION(std::runtime_error);
                return error::type_mismatch;
            }
        }
        error Argument::format_pointer(iterator& it, const_iterator end,
                                                       const ArgFormat& format, const std::uintptr_t value) const
        {
            if(format.ec_error()!=error::success) {
                return format.ec_error();
            }
            if(format.type_is_none() || format.type_is_pointer())
            {
#if defined(USF_TARGET_64_BITS)
                const auto ivalue = static_cast<uint64_t>(value);
#else
                const auto ivalue = static_cast<uint32_t>(value);
#endif
                const auto digits = Integer::count_digits_hex(ivalue);
                const auto fill_after = format.write_alignment(it, end, digits, false);
                if(fill_after<0) {
                    return error::buf_overflow;
                }
                it += digits;
                Integer::convert_hex(it, ivalue, format.uppercase());
                CharTraits::assign(it, format.fill_char(), fill_after);
                return error::success;
            }
            else
            {
                // Argument type / format mismatch
                //USF_CONTRACT_VIOLATION(std::runtime_error);
                return error::type_mismatch;
            }
        }
#if !defined(USF_DISABLE_FLOAT_SUPPORT)
        
        error Argument::format_float(iterator& it, const_iterator end, const ArgFormat& format, double value) 
        {
            if(format.ec_error()!=error::success) {
                return format.ec_error();
            }
            // Test for argument type / format match
            //USF_ENFORCE(format.type_is_none() || format.type_is_float(), std::runtime_error);
            if(!(format.type_is_none() || format.type_is_float())) {
                return error::type_mismatch;
            }

            if(std::isnan(value))
            {
                if( auto err = format_string(it, end, format, format.uppercase() ? "NAN" : "nan", 3); err!=error::success ) {
                    return err;
                }
            }
            else
            {
                const bool negative = std::signbit(value);

                if(std::isinf(value))
                {
                    if( auto err = format_string(it, end, format, format.uppercase() ? "INF" : "inf", 3, negative); err!=error::success) {
                        return err;
                    }
                }
                else
                {
                    if(negative) { value = -value; }

                    struct fp_t
                    {
                        union
                        {
                            double   d;
                            uint64_t i;
                        };
                    };

                    const fp_t fp_value{{value}};

                    if(fp_value.i == 0)
                    {
                        format_float_zero(it, end, format, negative);
                    }
                    else if(value >= 1E-19 && value <= 1.8446744E19)
                    {
                        int precision = format.precision();

                        if(precision < 0) { precision = 6; }

                        bool format_fixed = format.type_is_float_fixed();
                        bool significant_figures = false;

                        if(format.type_is_none() || format.type_is_float_general())
                        {
                            // General format
                            significant_figures = true;

                            if(precision > 0) { --precision; }
                        }

                        CharType significand[36]{}; // 34 characters should be the maximum size needed
                        int exponent = 0;

                        const auto significand_size = Float::convert(significand, exponent, value, format_fixed, precision);

                        if(significant_figures)
                        {
                            if(exponent >= -4 && exponent <= precision)
                            {
                                format_fixed = true;
                            }

                            if(!format.hash()) { precision = significand_size - 1; }

                            if(format_fixed)
                            {
                                precision -= exponent;
                            }
                        }

                        int fill_after = 0;

                        if(format_fixed)
                        {
                            // Fixed point format
                            if(exponent < 0)
                            {
                                // 0.<0>SIGNIFICAND[0:N]<0>

                                const int full_digits = precision + 2;
                                fill_after = format.write_alignment(it, end, full_digits, negative);
                                if(fill_after<0) {
                                    return error::buf_overflow;
                                }

                                *it++ = '0';
                                *it++ = '.';

                                int zero_digits = -exponent - 1;
                                CharTraits::assign(it, '0', zero_digits);
                                CharTraits::copy(it, significand, significand_size);

                                // Padding is needed if conversion function removes trailing zeros.
                                zero_digits = precision - zero_digits - significand_size;
                                CharTraits::assign(it, '0', zero_digits);
                            }
                            else
                            {
                                const int full_digits = exponent + 1 + precision + static_cast<int>(precision > 0 || format.hash());
                                fill_after = format.write_alignment(it, end, full_digits, negative);
                                if(fill_after<0) {
                                    return error::buf_overflow;
                                }

                                const int ipart_digits = exponent + 1;

                                if(ipart_digits >= significand_size)
                                {
                                    // [SIGNIFICAND]<0><.><0>

                                    CharTraits::copy(it, significand, significand_size);
                                    CharTraits::assign(it, '0', ipart_digits - significand_size);

                                    if(precision > 0 || format.hash())
                                    {
                                        *it++ = '.';
                                    }

                                    if(precision > 0)
                                    {
                                        CharTraits::assign(it, '0', precision);
                                    }
                                }
                                else
                                {
                                    // SIGNIFICAND[0:x].SIGNIFICAND[x:N]<0>

                                    CharTraits::copy(it, significand, ipart_digits);
                                    *it++ = '.';

                                    const int copy_size = significand_size - ipart_digits;
                                    CharTraits::copy(it, significand + ipart_digits, copy_size);

                                    // Padding is needed if conversion function removes trailing zeros.
                                    CharTraits::assign(it, '0', precision - copy_size);
                                }
                            }
                        }
                        else
                        {
                            // Exponent format
                            // SIGNIFICAND[0:N]<.>eEXP
                            // OR
                            // SIGNIFICAND[0].SIGNIFICAND[1:N]<0>eEXP

                            const int full_digits = 5 + precision + static_cast<int>(precision > 0 || format.hash());
                            fill_after = format.write_alignment(it, end, full_digits, negative);
                            if(fill_after<0) {
                                return error::buf_overflow;
                            }

                            *it++ = *significand;

                            if(precision > 0 || format.hash())
                            {
                                *it++ = '.';

                                const int copy_size = significand_size - 1;
                                CharTraits::copy(it, significand + 1, copy_size);
                                CharTraits::assign(it, '0', precision - copy_size);
                            }

                            write_float_exponent(it, exponent, format.uppercase());
                        }

                        CharTraits::assign(it, format.fill_char(), fill_after);

                        //it += sprintf(it, "[%s] Size:%d Exponent:%d Precision:%d Fixed:%d->", significand, significand_size, exponent, precision, int(format_fixed));
                    }
                    else
                    {
                        if(auto err=format_string(it, end, format, format.uppercase() ? "OVF" : "ovf", 3, negative); err!=error::success) {
                            return err;
                        }
                    }
                    return error::success;
                }
            }
            return error::success;

        }

        
        void Argument::write_float_exponent(iterator& it, int exponent, const bool uppercase) noexcept 
        {
            *it++ = uppercase ? 'E' : 'e';

            if(exponent < 0)
            {
                exponent = -exponent;
                *it++ = '-';
            }
            else
            {
                *it++ = '+';
            }

            // No point in making a proper integer to string
            // conversion for exponent since we only support [e-19; e19].
            assert(exponent <= 19);

            if(exponent < 10)
            {
                *it++ = '0';
                *it++ = static_cast<CharType>('0' + exponent);
            }
            else
            {
                *it++ = '1';
                *it++ = static_cast<CharType>('0' + (exponent - 10));
            }
        }

        void Argument::format_float_zero(iterator& it, const_iterator end, const ArgFormat& format, const bool negative)
        {
            if(format.ec_error()!=error::success) {
                return;
            }
            int precision = 0;

            if(format.type_is_float_fixed() || format.type_is_float_scientific())
            {
                precision = format.precision();
            }

            int digits = 1;

            if(precision > 0) { digits += precision + 1; }

            if(format.type_is_float_scientific()) { digits += 4; }

            const int fill_after = format.write_alignment(it, end, digits, negative);

            *it++ = '0';

            if(precision > 0)
            {
                *it++ = '.';
                CharTraits::assign(it, '0', precision);
            }

            if(format.type_is_float_scientific())
            {
                *it++ = format.uppercase() ? 'E' : 'e';
                *it++ = '+';
                *it++ = '0';
                *it++ = '0';
            }

            CharTraits::assign(it, format.fill_char(), fill_after);
        }
#endif // !defined(USF_DISABLE_FLOAT_SUPPORT)

        error Argument::format_string(iterator& it, const_iterator end,
                                                      ArgFormat& format, const usf::StringView& str) const
        {
            if(format.ec_error()!=error::success) {
                return format.ec_error();
            }
            // Test for argument type / format match
            //USF_ENFORCE(format.type_is_none() || format.type_is_string(), std::runtime_error);
            if( !(format.type_is_none() || format.type_is_string()) ) {
                return error::arg_type;
            }

            // Characters and strings align to left by default.
            format.default_align_left();

            // If precision is specified use it up to string size.
            const int str_length = (format.precision() == -1)
                                 ? static_cast<int>(str.size())
                                 : std::min(static_cast<int>(format.precision()), static_cast<int>(str.size()));

            return format_string(it, end, format, str.data(), str_length);
        }
}