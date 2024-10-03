#ifndef EXPECT_H_
#define EXPECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup expect_utility EXPECT error handling utility
 *
 * This tiny library provides simple utilities to manage error handling in a unified way across many software modules.
 *
 * <b>Rationale.</b>
 *
 * This header-only C library was born in order to provide a unified abstraction for writing sanity checks and handling
 * errors in a way that is easy to read to a human being. The idea was to be able to write short but meaningful statements
 * that can be easily understood and that could document the error checking process - especially the input argument
 * validation.
 *
 * <b>What may happen when you call a function?</b>
 *
 * We distinguish two types of things that may happen if you call a function:
 * - it runs throughout and produces a meaningful result that can be handled
 * - it aborts because an unexpected state was detected, which cannot be handled
 *
 * The first case includes both the situations in which the function ends with a success result as well as errors, that can
 * somehow be managed by the caller. For example, if you open a file for writing it may be successfully opened, but it may
 * also happen that the file is already open or the disk is write protected. Such cases usually end with an error code being
 * returned by the function to indicate, that it was unable to handle the request. This typically also includes situations
 * in which the input to the function is outside the valid range the function can handle - for example the filename was too long.
 * We will call these kind of errors <b><i>recoverable</i></b>
 *
 * But what should happen if during file access it turns out that the whole filesystem on the disk is corrupted and nothing
 * can be done to fix it? We call these situations as <b><i>unrecoverable</i></b> errors. In such case the application
 * might not be able to perform at all and should be aborted. This should however be considered as a rare and unexpected
 * behavior, where fixing it by the application is not feasible.
 *
 * <b>Handling recoverable errors with EXPECT</b>
 *
 * Below are some examples of function input validation with expect:
 *
 * @code
 * // This function returns one of 5 coefficients. For coeffNo >= 5 it always returns 0.0.
 * int getCoefficient(unsigned int coeffNo) {
 *     EXPECT(coeffNo < 5) OR_RETURN (0.0)
 *
 *     static double coeffTab[5] = { 1.0, 1.1, 1.2, 1.3, 1.4 };
 *
 *     return coeffTab[coeffNo];
 * }
 * @endcode
 *
 * <b>Handling unrecoverable errors with EXPECT</b>
 *
 * When unrecoverable error happens the expect library calls the @ref EXPECT_OnAbortHandler function that should be defined
 * by the user. The program must not continue operation after calling this function. A typical behavior of such function is to:
 * - go to a safe state
 * - log the error
 * - halt or reset the application
 *
 * In the above example, to convert to unrecoverable error we would change the implementation this way:
 *
 * @code
 * // This function returns one of 5 coefficients. For coeffNo >= 5 it causes the program to abort.
 * int getCoefficient(unsigned int coeffNo) {
 *     EXPECT(coeffNo < 5) OR_ABORT()
 *
 *     static double coeffTab[5] = { 1.0, 1.1, 1.2, 1.3, 1.4 };
 *
 *     return coeffTab[coeffNo];
 * }
 * @endcode
 *
 * <b>Standard and extra checks</b>
 *
 * For convenience the EXPECT library provides two levels of checks - standard checks and extra checks.
 * Extra checks are defined with the _EXTRA suffix. These checks are only performed if the EXPECT_CHECK_EXTRA
 * definition is visible when building the code.
 *
 * For example:
 * @code
 * int myFunc(const char* str) {
 *     EXPECT_EXTRA(NULL != str) OR_RETURN(0)
 * }
 *
 * @endcode
 *
 * <b>Runtime verbosity of abort handler</b>
 *
 * During initial development of the application, it is benefitial to quickly get the information about the abort location, and reason.
 * To enable this, you have to provide the following macro:
 * @code
 * EXPECT_VERBOSE=1
 * @endcode
 *
 * @{
 */

#ifndef EXPECT_VERBOSE
#    define EXPECT_VERBOSE 0 ///< When enabled, increases verbosity of information passed to EXPECT_OnAbortHandler
                             ///< @note Enabling this macro WILL generate measurable memory overhead
#endif


/// @internal @brief Noreturn attribute
#define EXPECT_INTERNAL_NORETURN __attribute__((noreturn))
/**
 * Abort handler. The program MUST not continue operation after calling this function. Its context MAY be used to safely restart the program
 * @param[in] why message describing reason why the contract was violated
 * @param[in] file printable filename
 * @param[in] line line where check was performed
 */
EXPECT_INTERNAL_NORETURN void EXPECT_OnAbortHandler(char const* why, char const* file, int line);

/**
 * @brief Checks whether expression expr holds true value
 * @note the macro MUST be followed by either OR_RETURN, OR_ABORT, or OR_THROW macro
 */
#define EXPECT(expr) if (!(expr))


/**
 * @def EXPECT_EXTRA(expr)
 * @copydoc EXPECT
 * As the evaluation of the expression may be time consuming, it is possible to globally disable this kind of runtime checks by defining EXPECT_CHECK_EXTRA to 0
 */
#if (defined EXPECT_CHECK_EXTRA) && (EXPECT_CHECK_EXTRA == 1)
#    define EXPECT_EXTRA(expr) EXPECT(expr)
#else
#    define EXPECT_EXTRA(expr) if (0)
#endif

/**
 * @brief Calls directly the abort handler (EXPECT_OnAbortHandler) with given reason
 * @param[in] why Error message
 * This macro must be preceded either EXPECT, or EXPECT_EXTRA macro
 */
#define EXPECT_ABORT(why) EXPECT_INTERNAL_CALL_ABORT_HANDLER(why)

/**
 * @brief Calls the abort handler (EXPECT_OnAbortHandler) with given reason
 * @param[in] why Error message
 * This macro must be preceded either EXPECT, or EXPECT_EXTRA macro
 */
#define OR_ABORT(why) EXPECT_INTERNAL_CALL_ABORT_HANDLER(why)


/**
 * @brief Returns from the function with given error value
 * @param[in] retval Error value to be returned
 * This macro must be preceded either EXPECT, or EXPECT_EXTRA macro
 */
#define OR_RETURN(retval) return retval

#if 1 == EXPECT_VERBOSE
#    define EXPECT_INTERNAL_CALL_ABORT_HANDLER(why) EXPECT_OnAbortHandler("" why, __FILE__, __LINE__)
#else
#    define EXPECT_INTERNAL_CALL_ABORT_HANDLER(why) EXPECT_OnAbortHandler("<verbose-disabled>", "<verbose-disabled>", 0)
#endif


#ifdef __cplusplus
/**
 * @def OR_THROW(expr)
 * @brief Throws exception expr.
 * @param[in] expr instance of exception object to be throw.
 *
 * When exceptions are disabled the operation is the same as OR_ABORT
 */


#    ifdef __cpp_exceptions
#        define OR_THROW(expr) throw(expr)
#    else
#        define EXPECT_INTERNAL_STR(x) #        x
#        define OR_THROW(expr)         EXPECT_OnAbortHandler("Caught exception: " EXPECT_INTERNAL_STR(expr), __FILE__, __LINE__)
#    endif
#endif


#ifdef __cplusplus
}
#endif

/** @} */

#endif // EXPECT_H_
