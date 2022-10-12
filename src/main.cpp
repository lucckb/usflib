#include <usf/usf.hpp>

namespace usf::internal {

error parse_format_string(usf::StringSpan& str, usf::StringView& fmt)
{
          CharType* str_it = str.begin();
    const CharType* fmt_it = fmt.cbegin();

    while(fmt_it < fmt.cend() && str_it < str.end())
    {
        if(*fmt_it == '{' )
        {
            if(*(fmt_it + 1) == '{')
            {
                // Found '{{' escape character, skip the first and copy the second '{'.
                ++fmt_it;
                *str_it++ = *fmt_it++;
            }
            else
            {
                // A type format should follow...
                break;
            }
        }
        else if(*fmt_it == '}')
        {
            //USF_ENFORCE(*(fmt_it + 1) == '}', std::runtime_error);
            if(*(fmt_it + 1) != '}') {
                return error::arg_enclose;
            }

            // Found '}}' escape character, skip the first and copy the second '}'.
            ++fmt_it;
            *str_it++ = *fmt_it++;
        }
        else
        {
            // Copy literal text
            *str_it++ = *fmt_it++;
        }
    }

    //USF_ENFORCE(str_it < str.end(), std::runtime_error);
    if(str_it < str.end()) {
        str.remove_prefix(str_it - str.begin());
        fmt.remove_prefix(fmt_it - fmt.cbegin());
        return error::success;
    } else {
        return error::buf_overflow;
    }
}

error process(usf::StringSpan& str, usf::StringView& fmt,
             const Argument* const args, const int arg_count)
{
    // Argument's sequential index
    int arg_seq_index = 0;

    if(auto err=parse_format_string(str, fmt); err!=error::success) {
        return err;
    }

    while(!fmt.empty())
    {
        ArgFormat format(fmt, arg_count);

        if(format.ec_error()!=error::success) {
            return format.ec_error();
        }

        // Determine which argument index to use, sequential or positional.
        int arg_index = format.index();

        if(arg_index < 0)
        {
            //USF_ENFORCE(arg_seq_index < arg_count, std::runtime_error);
            if(arg_seq_index >= arg_count) {
                return error::arg_index_too_large;
            }
            arg_index = arg_seq_index++;
        }

        if(auto err=args[arg_index].format(str, format); err!=error::success) {
            return err;
        }

        if(auto err=parse_format_string(str, fmt); err!=error::success) {
            return err;
        }
    }
    return error::success;
}

}