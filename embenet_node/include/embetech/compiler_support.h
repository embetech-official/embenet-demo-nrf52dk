#ifndef EMBEUTILS_COMPILER_SUPPORT_H_
#define EMBEUTILS_COMPILER_SUPPORT_H_

#ifdef __GNUC__

///@brief Forces compiler to produce structure without padding (sizeof(structure) equals sum of sizeof(structure fields))
#    define EMBEUTILS_PACKED __attribute__((packed))

///@brief Informs compiler that the function may not return (i.e. has infinite loop, or throws)
#    define EMBEUTILS_NORETURN __attribute__((noreturn))

///@brief Informs compiler that the symbol MUST be emitted as weak symbol
#    define EMBEUTILS_WEAK __attribute__((weak))

///@brief Warns if the return value of the function is not used
#    define EMBEUTILS_NODISCARD __attribute__((unused))

///@brief Indicates that the fall through from the previous case label is intentional and should not be diagnosed by a compiler that warns on fallthrough
#    define EMBEUTILS_FALLTHROUGH __attribute__((fallthrough))


#    define EMBEUTILS_INLINE __attribute__((always_inline))

#    ifdef __cplusplus
///@brief provides portable mean of using C99 restrict keyword
#        define EMBEUTILS_RESTRICT __restrict
#    else
///@brief alias to C99 restrict keyword
#        define EMBEUTILS_RESTRICT restrict
#    endif

/**
 * @brief Informs compiler that the argument is unused
 * @param[in] x name of the argument to be marked as unused
 *
 * Usage: void function(int arg1, int EMBEUTILS_UNUSED(arg2)) { ... }
 */
#    define EMBEUTILS_UNUSED(x) x __attribute__((unused))

/**
 * @brief Warns when nullptr is provided as argument on given position
 * @param[in] ... list of arguments indices to be checked against nullptr. When no argument is provided, ALL of the pointer arguments are marked as non-null
 *
 * Usage:
 * void func1(void* dst, void* src) EMBEUTILS_NONNULL() - both dst and src MUST NOT take nullptr
 * void func2(void* dst, void* src) EMBEUTILS_NONNULL(2) - only src MUST NOT take nullptr
 * @note GNU compiler checks this only with -Wnonnull flag (which is included in -Wall)
 */
// #    define EMBEUTILS_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#    define EMBEUTILS_NONNULL(...)

#else
#    error "Compiler not supported (yet)"
#endif

#endif
