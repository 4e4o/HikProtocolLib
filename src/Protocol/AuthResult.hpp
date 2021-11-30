#ifndef AUTH_RESULT_HPP
#define AUTH_RESULT_HPP

#include <string>

struct AuthResult {
    std::string key;
    uint32_t firstU32;          // at 0 offset
    uint32_t secondU32;         // at 12 offset
};

#endif /* AUTH_RESULT_HPP */
