#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <cstdlib>

#if defined(__GNUC__) || defined(__clang__)
    #define BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
    #define BUILTIN_UNREACHABLE __assume(0)
#else
    #define BUILTIN_UNREACHABLE
#endif

#ifndef DEBUG
    #define UNREACHABLE(msg) \
        do { \
            std::cerr << "UNREACHABLE executed at " << __FILE__ << ":" << __LINE__ \
                      << " info: " << msg << std::endl; \
            std::abort(); \
        } while (0)
#else
    #define UNREACHABLE(msg) \
        do { \
            BUILTIN_UNREACHABLE; \
        } while (0)
#endif

enum class DEBUG_LEVEL {
    NONE = 0,
    DEV = 1,
    DIAG = 2,
    DEEP = 3
};

// TODO: move this to logger.cpp
template<typename... Args>
void log_err(Args&&... args) {
    (std::cerr << ... << args) << std::endl;
}

#if defined(__cplusplus) && __cplusplus >= 201103L

    #define COMPILER_STATIC_ASSERT_MSG(condition, msg) static_assert(condition, msg)
    #define COMPILER_STATIC_ASSERT(condition) static_assert(condition, "")

    #define COMPILER_ASSERT(condition, msg) if(!condition) { \
        log_err("ASSERTION FAILED: ", msg );                 \
        exit(0);                                         \
    }

#else
    #error "C++ version is outdated. Typher compiler requires C++ 11 or higher."
#endif

#endif