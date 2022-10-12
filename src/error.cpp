#include <usf/usf.hpp>


namespace usf {
    const char* error_to_string(error ec)
    {
        switch(ec) {
        case error::success:
            return "USF success (error::success)";
        case error::buf_overflow:
            return "USF buffer overflow (error::buff_overflow)";
        case error::arg_type:
            return "USF invalid argument type (error::arg_type)";
        case error::arg_precision:
            return "USF argument precision error (error::arg_precision)";
        case error::type_mismatch:
            return "USF type mismatch (error::type_mismatch)";
        case error::no_custom_format:
            return "USF Missing custom formatter (error::no_custom_format)";
        case error::no_format_arg:
            return "USF Missing format argument (error::no_format_arg)";
        case error::arg_index_too_large:
            return "USF Argument index too large (error::arg_index_too_large)";
        case error::arg_enclose:
            return "USF Argument enclose error (error::arg_enclose_err)";
        case error::inv_prec_specifier:
            return "USF invalid precision specifier (error::inv_prec_specifier)";
        case error::inv_type:
            return "USF invalid type (error::inv_type)";
        case error::inv_sign:
            return "USF invalid sign (error::inv_sign)";
        case error::inv_format:
            return "USF invalid format (error::inv_format)";
        case error::value_overflow:
            return "USF value overflow (error::value_overflow)";
        }
        return "USF unknown error";
    }
}