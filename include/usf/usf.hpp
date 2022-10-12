#pragma once

#ifndef __cplusplus
#error This library is for C++ only
#endif


#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("Os")
#endif

#include "usf/internal/usf_config.hpp"
#include "usf/internal/usf_error.hpp"
#include "usf/internal/usf_traits.hpp"
#include "usf/internal/usf_string_span.hpp"
#include "usf/internal/usf_result.hpp"
#include "usf/internal/usf_string_view.hpp"
#include "usf/internal/usf_integer.hpp"
#include "usf/internal/usf_float.hpp"
#include "usf/internal/usf_arg_format.hpp"
#include "usf/internal/usf_arg_custom_type.hpp"
#include "usf/internal/usf_argument.hpp"
#include "usf/internal/usf_main.hpp"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif



