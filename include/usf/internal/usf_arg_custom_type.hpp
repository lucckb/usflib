// ----------------------------------------------------------------------------
// @file    usf_arg_custom_type.hpp
// @brief   User-defined custom type class (using the delegate idiom).
// @date    07 January 2019
// ----------------------------------------------------------------------------

#pragma once

namespace usf::internal
{

class ArgCustomType
{
    public:

        // --------------------------------------------------------------------
        // PUBLIC MEMBER FUNCTIONS
        // --------------------------------------------------------------------

        constexpr ArgCustomType() = delete;

        template<typename T, result_t(*func)(StringSpan, const T&)>
        static constexpr ArgCustomType create(const T* obj)
        {
            return ArgCustomType(invoke_func<T, func>, obj);
        }

        constexpr result_t operator()(StringSpan dst) const
        {
            return m_function(dst, m_obj);
        }

    private:

        // --------------------------------------------------------------------
        // PRIVATE TYPE ALIASES
        // --------------------------------------------------------------------

        using FunctionType = result_t(*)(StringSpan, const void*);

        // --------------------------------------------------------------------
        // PRIVATE MEMBER FUNCTIONS
        // --------------------------------------------------------------------

        constexpr ArgCustomType(const FunctionType func, const void* obj)
            : m_function{func}, m_obj{obj} {}

        template<typename T, result_t(*func)(StringSpan, const T&)>
        static constexpr result_t invoke_func(StringSpan dst, const void* obj)
        {
            return func(dst, *static_cast<const T*>(obj));
        }

        // --------------------------------------------------------------------
        // PRIVATE VARIABLES
        // --------------------------------------------------------------------

        const FunctionType m_function{nullptr};
        const void*        m_obj     {nullptr};
};

} // namespace usf

