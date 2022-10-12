#pragma once

namespace usf {
    enum class error : signed char {
        success             =  0,
        buf_overflow        = -1,
        arg_type            = -2,
        arg_precision       = -3,
        type_mismatch       = -4,
        no_custom_format    = -5,
        no_format_arg       = -6,
        arg_index_too_large = -7,
        arg_enclose         = -8,
        inv_prec_specifier  = -9,
        inv_type            = -10,
        inv_sign            = -11,
        inv_format          = -12,
        value_overflow      = -14,
    };

    [[nodiscard]] const char* error_to_string(error ec);
}