#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>
#include <sstream>
#include <cassert>

#ifdef _WIN32
#include <intrin.h>
#define DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#endif

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::ostringstream oss; \
            oss << "Assertion failed: (" << #condition << "), function " << __FUNCTION__ \
                << ", file " << __FILE__ << ", line " << __LINE__ << ".\n" << message; \
            DEBUG_BREAK(); \
            throw std::runtime_error(oss.str()); \
        } \
    } while (false)

#define THROW(message) \
    do { \
        std::ostringstream oss; \
        oss << "Exception: " << message << ", function " << __FUNCTION__ \
            << ", file " << __FILE__ << ", line " << __LINE__; \
        DEBUG_BREAK(); \
        throw std::runtime_error(oss.str()); \
    } while (false)

#endif // ERROR_H