#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdio.h>

#ifdef NDEBUG
#define debug_print(...) { }
#else
#define debug_print(...) { printf(__VA_ARGS__); }
#endif // NDEBUG

#endif /* DEBUG_HPP */
