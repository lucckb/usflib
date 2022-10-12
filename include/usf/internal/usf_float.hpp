// ----------------------------------------------------------------------------
// @file    usf_float.hpp
// @brief   Floating point conversion and helper functions. Naive and limited
//          implementation with the usual precision/rounding errors (good for now).
// @date    07 January 2019
// ----------------------------------------------------------------------------

#pragma once

#if !defined(USF_DISABLE_FLOAT_SUPPORT)


namespace usf::internal::Float
{

    int convert(CharType* const significand, int& exponent,
            double value, const bool format_fixed, const int precision) noexcept;
        
} // namespace usf

#endif // USF_FLOAT_HPP
