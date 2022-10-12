// ----------------------------------------------------------------------------
// @file    usf_main.hpp
// @brief   Main process functions and public interface.
// @date    14 January 2019
// ----------------------------------------------------------------------------

#pragma once

namespace usf
{
    namespace internal
    {
        [[nodiscard]] error parse_format_string(usf::StringSpan& str, usf::StringView& fmt);

        [[nodiscard]] error process(usf::StringSpan& str, usf::StringView& fmt,
                 const Argument* const args, const int arg_count);
    } // namespace internal


    template <typename... Args> constexpr
    result_t basic_format_to(StringSpan str, StringView fmt)
    {
        auto str_begin = str.begin();

        if(auto err=internal::parse_format_string(str, fmt); err!=error::success) {
            return {err, {}};
        }

        //USF_ENFORCE(fmt.empty(), "std::runtime_error");
        if(!fmt.empty()) {
            return { error::no_format_arg, {} };
        }

    #if !defined(USF_DISABLE_STRING_TERMINATION)
        // If not disabled in configuration, null terminate the resulting string.
        str[0] = CharType{};
    #endif

        // Return a string span to the resulting string
        return { error::success, StringSpan(str_begin, str.begin()) };
    }

    template <typename... Args> constexpr
    result_t basic_format_to(StringSpan str, StringView fmt, Args&&... args)
    {
        // Nobody should be that crazy, still... it costs nothing to be sure!
        static_assert(sizeof...(Args) < 32, "usf::basic_format_to(): crazy number of arguments supplied!");

        auto str_begin = str.begin();

        const internal::Argument arguments[sizeof...(Args)]{internal::make_argument(args)...};

        if(auto err=internal::process(str, fmt, arguments, static_cast<int>(sizeof...(Args))); err!=error::success) {
            return {err, {}};
        }

    #if !defined(USF_DISABLE_STRING_TERMINATION)
        // If not disabled in configuration, null terminate the resulting string.
        str[0] = CharType{};
    #endif

        // Return a string span to the resulting string
        return {error::success, StringSpan(str_begin, str.begin())};
    }

    template <typename... Args> constexpr
    ssize_t basic_format_to(CharType* str, const std::ptrdiff_t str_count, StringView fmt, Args&&... args)
    {
        auto [err, span] = basic_format_to(StringSpan(str, str_count), fmt, args...);
        return err==error::success ? ssize_t(span.size()) : ssize_t(err);
    }

    template <typename... Args> constexpr
    ssize_t format_to(char* str, const std::ptrdiff_t str_count, StringView fmt, Args&&... args)
    {
        return basic_format_to(str, str_count, fmt, args...);
    }
    
    static inline error check_error(ssize_t err_code) {
        return (err_code>0)?(error::success):error(err_code);
    }

} // namespace usf

