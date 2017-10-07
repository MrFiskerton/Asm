#ifndef TRAMPOLINE_ARGUMENT_TYPES_H
#define TRAMPOLINE_ARGUMENT_TYPES_H

#include <xmmintrin.h>

/**
 * Structures that saves the number of integer and SSE arguments given as parameters.
 */
template<typename ... Args>
struct arg_type;

template<>
struct arg_type<> {
    static const int reg = 0;
    static const int sse = 0;
};

template<typename Return, typename ... Args>
struct arg_type<Return, Args ...> {
    // Here must be more accurate check for the return statement
    static const int reg = arg_type<Args ...>::reg + 1;
    static const int sse = arg_type<Args ...>::sse;
};

template<typename ... Args>
struct arg_type<double, Args ...> {
    static const int reg = arg_type<Args ...>::reg;
    static const int sse = arg_type<Args ...>::sse + 1;
};

template<typename ... Args>
struct arg_type<float, Args ...> {
    static const int reg = arg_type<Args ...>::reg;
    static const int sse = arg_type<Args ...>::sse + 1;
};

#endif // TRAMPOLINE_ARGUMENT_TYPES_H