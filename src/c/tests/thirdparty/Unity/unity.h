/* =========================================================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007-21 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
============================================================================ */

// #include "unity.h"
/* ==========================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007-21 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
========================================== */

#ifndef UNITY_FRAMEWORK_H
#define UNITY_FRAMEWORK_H
#define UNITY

#define UNITY_VERSION_MAJOR    2
#define UNITY_VERSION_MINOR    5
#define UNITY_VERSION_BUILD    4
#define UNITY_VERSION          ((UNITY_VERSION_MAJOR << 16) | (UNITY_VERSION_MINOR << 8) | UNITY_VERSION_BUILD)

#ifdef __cplusplus
extern "C"
{
#endif

// #include "unity_internals.h"
/* ==========================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007-21 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
========================================== */

#ifndef UNITY_INTERNALS_H
#define UNITY_INTERNALS_H

#ifdef UNITY_INCLUDE_CONFIG_H
#include "unity_config.h"
#endif

#ifndef UNITY_EXCLUDE_SETJMP_H
#include <setjmp.h>
#endif

#ifndef UNITY_EXCLUDE_MATH_H
#include <math.h>
#endif

#ifndef UNITY_EXCLUDE_STDDEF_H
#include <stddef.h>
#endif

#ifdef UNITY_INCLUDE_PRINT_FORMATTED
#include <stdarg.h>
#endif

/* Unity Attempts to Auto-Detect Integer Types
 * Attempt 1: UINT_MAX, ULONG_MAX in <limits.h>, or default to 32 bits
 * Attempt 2: UINTPTR_MAX in <stdint.h>, or default to same size as long
 * The user may override any of these derived constants:
 * UNITY_INT_WIDTH, UNITY_LONG_WIDTH, UNITY_POINTER_WIDTH */
#ifndef UNITY_EXCLUDE_STDINT_H
#include <stdint.h>
#endif

#ifndef UNITY_EXCLUDE_LIMITS_H
#include <limits.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define UNITY_FUNCTION_ATTR(a)    __attribute__((a))
#else
  #define UNITY_FUNCTION_ATTR(a)    /* ignore */
#endif

#ifndef UNITY_NORETURN
  #if defined(__cplusplus)
    #if __cplusplus >= 201103L
      #define UNITY_NORETURN [[ noreturn ]]
    #endif
  #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #include <stdnoreturn.h>
    #define UNITY_NORETURN noreturn
  #endif
#endif
#ifndef UNITY_NORETURN
  #define UNITY_NORETURN UNITY_FUNCTION_ATTR(noreturn)
#endif

/*-------------------------------------------------------
 * Guess Widths If Not Specified
 *-------------------------------------------------------*/

/* Determine the size of an int, if not already specified.
 * We cannot use sizeof(int), because it is not yet defined
 * at this stage in the translation of the C program.
 * Also sizeof(int) does return the size in addressable units on all platforms,
 * which may not necessarily be the size in bytes.
 * Therefore, infer it from UINT_MAX if possible. */
#ifndef UNITY_INT_WIDTH
  #ifdef UINT_MAX
    #if (UINT_MAX == 0xFFFF)
      #define UNITY_INT_WIDTH (16)
    #elif (UINT_MAX == 0xFFFFFFFF)
      #define UNITY_INT_WIDTH (32)
    #elif (UINT_MAX == 0xFFFFFFFFFFFFFFFF)
      #define UNITY_INT_WIDTH (64)
    #endif
  #else /* Set to default */
    #define UNITY_INT_WIDTH (32)
  #endif /* UINT_MAX */
#endif

/* Determine the size of a long, if not already specified. */
#ifndef UNITY_LONG_WIDTH
  #ifdef ULONG_MAX
    #if (ULONG_MAX == 0xFFFF)
      #define UNITY_LONG_WIDTH (16)
    #elif (ULONG_MAX == 0xFFFFFFFF)
      #define UNITY_LONG_WIDTH (32)
    #elif (ULONG_MAX == 0xFFFFFFFFFFFFFFFF)
      #define UNITY_LONG_WIDTH (64)
    #endif
  #else /* Set to default */
    #define UNITY_LONG_WIDTH (32)
  #endif /* ULONG_MAX */
#endif

/* Determine the size of a pointer, if not already specified. */
#ifndef UNITY_POINTER_WIDTH
  #ifdef UINTPTR_MAX
    #if (UINTPTR_MAX <= 0xFFFF)
      #define UNITY_POINTER_WIDTH (16)
    #elif (UINTPTR_MAX <= 0xFFFFFFFF)
      #define UNITY_POINTER_WIDTH (32)
    #elif (UINTPTR_MAX <= 0xFFFFFFFFFFFFFFFF)
      #define UNITY_POINTER_WIDTH (64)
    #endif
  #else /* Set to default */
    #define UNITY_POINTER_WIDTH UNITY_LONG_WIDTH
  #endif /* UINTPTR_MAX */
#endif

/*-------------------------------------------------------
 * Int Support (Define types based on detected sizes)
 *-------------------------------------------------------*/

#if (UNITY_INT_WIDTH == 32)
    typedef unsigned char   UNITY_UINT8;
    typedef unsigned short  UNITY_UINT16;
    typedef unsigned int    UNITY_UINT32;
    typedef signed char     UNITY_INT8;
    typedef signed short    UNITY_INT16;
    typedef signed int      UNITY_INT32;
#elif (UNITY_INT_WIDTH == 16)
    typedef unsigned char   UNITY_UINT8;
    typedef unsigned int    UNITY_UINT16;
    typedef unsigned long   UNITY_UINT32;
    typedef signed char     UNITY_INT8;
    typedef signed int      UNITY_INT16;
    typedef signed long     UNITY_INT32;
#else
    #error Invalid UNITY_INT_WIDTH specified! (16 or 32 are supported)
#endif

/*-------------------------------------------------------
 * 64-bit Support
 *-------------------------------------------------------*/

/* Auto-detect 64 Bit Support */
#ifndef UNITY_SUPPORT_64
  #if UNITY_LONG_WIDTH == 64 || UNITY_POINTER_WIDTH == 64
    #define UNITY_SUPPORT_64
  #endif
#endif

/* 64-Bit Support Dependent Configuration */
#ifndef UNITY_SUPPORT_64
    /* No 64-bit Support */
    typedef UNITY_UINT32 UNITY_UINT;
    typedef UNITY_INT32  UNITY_INT;
    #define UNITY_MAX_NIBBLES (8)  /* Maximum number of nibbles in a UNITY_(U)INT */
#else
  /* 64-bit Support */
  #if (UNITY_LONG_WIDTH == 32)
    typedef unsigned long long UNITY_UINT64;
    typedef signed long long   UNITY_INT64;
  #elif (UNITY_LONG_WIDTH == 64)
    typedef unsigned long      UNITY_UINT64;
    typedef signed long        UNITY_INT64;
  #else
    #error Invalid UNITY_LONG_WIDTH specified! (32 or 64 are supported)
  #endif
    typedef UNITY_UINT64 UNITY_UINT;
    typedef UNITY_INT64  UNITY_INT;
    #define UNITY_MAX_NIBBLES (16) /* Maximum number of nibbles in a UNITY_(U)INT */
#endif

/*-------------------------------------------------------
 * Pointer Support
 *-------------------------------------------------------*/

#if (UNITY_POINTER_WIDTH == 32)
  #define UNITY_PTR_TO_INT UNITY_INT32
  #define UNITY_DISPLAY_STYLE_POINTER UNITY_DISPLAY_STYLE_HEX32
#elif (UNITY_POINTER_WIDTH == 64)
  #define UNITY_PTR_TO_INT UNITY_INT64
  #define UNITY_DISPLAY_STYLE_POINTER UNITY_DISPLAY_STYLE_HEX64
#elif (UNITY_POINTER_WIDTH == 16)
  #define UNITY_PTR_TO_INT UNITY_INT16
  #define UNITY_DISPLAY_STYLE_POINTER UNITY_DISPLAY_STYLE_HEX16
#else
  #error Invalid UNITY_POINTER_WIDTH specified! (16, 32 or 64 are supported)
#endif

#ifndef UNITY_PTR_ATTRIBUTE
  #define UNITY_PTR_ATTRIBUTE
#endif

#ifndef UNITY_INTERNAL_PTR
  #define UNITY_INTERNAL_PTR UNITY_PTR_ATTRIBUTE const void*
#endif

/*-------------------------------------------------------
 * Float Support
 *-------------------------------------------------------*/

#ifdef UNITY_EXCLUDE_FLOAT

/* No Floating Point Support */
#ifndef UNITY_EXCLUDE_DOUBLE
#define UNITY_EXCLUDE_DOUBLE /* Remove double when excluding float support */
#endif
#ifndef UNITY_EXCLUDE_FLOAT_PRINT
#define UNITY_EXCLUDE_FLOAT_PRINT
#endif

#else

/* Floating Point Support */
#ifndef UNITY_FLOAT_PRECISION
#define UNITY_FLOAT_PRECISION (0.00001f)
#endif
#ifndef UNITY_FLOAT_TYPE
#define UNITY_FLOAT_TYPE float
#endif
typedef UNITY_FLOAT_TYPE UNITY_FLOAT;

/* isinf & isnan macros should be provided by math.h */
#ifndef isinf
/* The value of Inf - Inf is NaN */
#define isinf(n) (isnan((n) - (n)) && !isnan(n))
#endif

#ifndef isnan
/* NaN is the only floating point value that does NOT equal itself.
 * Therefore if n != n, then it is NaN. */
#define isnan(n) ((n != n) ? 1 : 0)
#endif

#endif

/*-------------------------------------------------------
 * Double Float Support
 *-------------------------------------------------------*/

/* unlike float, we DON'T include by default */
#if defined(UNITY_EXCLUDE_DOUBLE) || !defined(UNITY_INCLUDE_DOUBLE)

  /* No Floating Point Support */
  #ifndef UNITY_EXCLUDE_DOUBLE
  #define UNITY_EXCLUDE_DOUBLE
  #else
    #undef UNITY_INCLUDE_DOUBLE
  #endif

  #ifndef UNITY_EXCLUDE_FLOAT
    #ifndef UNITY_DOUBLE_TYPE
    #define UNITY_DOUBLE_TYPE double
    #endif
  typedef UNITY_FLOAT UNITY_DOUBLE;
  /* For parameter in UnityPrintFloat(UNITY_DOUBLE), which aliases to double or float */
  #endif

#else

  /* Double Floating Point Support */
  #ifndef UNITY_DOUBLE_PRECISION
  #define UNITY_DOUBLE_PRECISION (1e-12)
  #endif

  #ifndef UNITY_DOUBLE_TYPE
  #define UNITY_DOUBLE_TYPE double
  #endif
  typedef UNITY_DOUBLE_TYPE UNITY_DOUBLE;

#endif

/*-------------------------------------------------------
 * Output Method: stdout (DEFAULT)
 *-------------------------------------------------------*/
#ifndef UNITY_OUTPUT_CHAR
  /* Default to using putchar, which is defined in stdio.h */
  #include <stdio.h>
  #define UNITY_OUTPUT_CHAR(a) (void)putchar(a)
#else
  /* If defined as something else, make sure we declare it here so it's ready for use */
  #ifdef UNITY_OUTPUT_CHAR_HEADER_DECLARATION
    extern void UNITY_OUTPUT_CHAR_HEADER_DECLARATION;
  #endif
#endif

#ifndef UNITY_OUTPUT_FLUSH
  #ifdef UNITY_USE_FLUSH_STDOUT
    /* We want to use the stdout flush utility */
    #include <stdio.h>
    #define UNITY_OUTPUT_FLUSH() (void)fflush(stdout)
  #else
    /* We've specified nothing, therefore flush should just be ignored */
    #define UNITY_OUTPUT_FLUSH()
  #endif
#else
  /* If defined as something else, make sure we declare it here so it's ready for use */
  #ifdef UNITY_OUTPUT_FLUSH_HEADER_DECLARATION
    extern void UNITY_OUTPUT_FLUSH_HEADER_DECLARATION;
  #endif
#endif

#ifndef UNITY_OUTPUT_FLUSH
#define UNITY_FLUSH_CALL()
#else
#define UNITY_FLUSH_CALL() UNITY_OUTPUT_FLUSH()
#endif

#ifndef UNITY_PRINT_EOL
#define UNITY_PRINT_EOL()    UNITY_OUTPUT_CHAR('\n')
#endif

#ifndef UNITY_OUTPUT_START
#define UNITY_OUTPUT_START()
#endif

#ifndef UNITY_OUTPUT_COMPLETE
#define UNITY_OUTPUT_COMPLETE()
#endif

#ifdef UNITY_INCLUDE_EXEC_TIME
  #if !defined(UNITY_EXEC_TIME_START) && \
      !defined(UNITY_EXEC_TIME_STOP) && \
      !defined(UNITY_PRINT_EXEC_TIME) && \
      !defined(UNITY_TIME_TYPE)
      /* If none any of these macros are defined then try to provide a default implementation */

    #if defined(UNITY_CLOCK_MS)
      /* This is a simple way to get a default implementation on platforms that support getting a millisecond counter */
      #define UNITY_TIME_TYPE UNITY_UINT
      #define UNITY_EXEC_TIME_START() Unity.CurrentTestStartTime = UNITY_CLOCK_MS()
      #define UNITY_EXEC_TIME_STOP() Unity.CurrentTestStopTime = UNITY_CLOCK_MS()
      #define UNITY_PRINT_EXEC_TIME() { \
        UNITY_UINT execTimeMs = (Unity.CurrentTestStopTime - Unity.CurrentTestStartTime); \
        UnityPrint(" ("); \
        UnityPrintNumberUnsigned(execTimeMs); \
        UnityPrint(" ms)"); \
        }
    #elif defined(_WIN32)
      #include <time.h>
      #define UNITY_TIME_TYPE clock_t
      #define UNITY_GET_TIME(t) t = (clock_t)((clock() * 1000) / CLOCKS_PER_SEC)
      #define UNITY_EXEC_TIME_START() UNITY_GET_TIME(Unity.CurrentTestStartTime)
      #define UNITY_EXEC_TIME_STOP() UNITY_GET_TIME(Unity.CurrentTestStopTime)
      #define UNITY_PRINT_EXEC_TIME() { \
        UNITY_UINT execTimeMs = (Unity.CurrentTestStopTime - Unity.CurrentTestStartTime); \
        UnityPrint(" ("); \
        UnityPrintNumberUnsigned(execTimeMs); \
        UnityPrint(" ms)"); \
        }
    #elif defined(__unix__) || defined(__APPLE__)
      #include <time.h>
      #define UNITY_TIME_TYPE struct timespec
      #define UNITY_GET_TIME(t) clock_gettime(CLOCK_MONOTONIC, &t)
      #define UNITY_EXEC_TIME_START() UNITY_GET_TIME(Unity.CurrentTestStartTime)
      #define UNITY_EXEC_TIME_STOP() UNITY_GET_TIME(Unity.CurrentTestStopTime)
      #define UNITY_PRINT_EXEC_TIME() { \
        UNITY_UINT execTimeMs = ((Unity.CurrentTestStopTime.tv_sec - Unity.CurrentTestStartTime.tv_sec) * 1000L); \
        execTimeMs += ((Unity.CurrentTestStopTime.tv_nsec - Unity.CurrentTestStartTime.tv_nsec) / 1000000L); \
        UnityPrint(" ("); \
        UnityPrintNumberUnsigned(execTimeMs); \
        UnityPrint(" ms)"); \
        }
    #endif
  #endif
#endif

#ifndef UNITY_EXEC_TIME_START
#define UNITY_EXEC_TIME_START() do{}while(0)
#endif

#ifndef UNITY_EXEC_TIME_STOP
#define UNITY_EXEC_TIME_STOP() do{}while(0)
#endif

#ifndef UNITY_TIME_TYPE
#define UNITY_TIME_TYPE UNITY_UINT
#endif

#ifndef UNITY_PRINT_EXEC_TIME
#define UNITY_PRINT_EXEC_TIME() do{}while(0)
#endif

/*-------------------------------------------------------
 * Footprint
 *-------------------------------------------------------*/

#ifndef UNITY_LINE_TYPE
#define UNITY_LINE_TYPE UNITY_UINT
#endif

#ifndef UNITY_COUNTER_TYPE
#define UNITY_COUNTER_TYPE UNITY_UINT
#endif

/*-------------------------------------------------------
 * Internal Structs Needed
 *-------------------------------------------------------*/

typedef void (*UnityTestFunction)(void);

#define UNITY_DISPLAY_RANGE_INT  (0x10)
#define UNITY_DISPLAY_RANGE_UINT (0x20)
#define UNITY_DISPLAY_RANGE_HEX  (0x40)
#define UNITY_DISPLAY_RANGE_CHAR (0x80)

typedef enum
{
    UNITY_DISPLAY_STYLE_INT      = (UNITY_INT_WIDTH / 8) + UNITY_DISPLAY_RANGE_INT,
    UNITY_DISPLAY_STYLE_INT8     = 1 + UNITY_DISPLAY_RANGE_INT,
    UNITY_DISPLAY_STYLE_INT16    = 2 + UNITY_DISPLAY_RANGE_INT,
    UNITY_DISPLAY_STYLE_INT32    = 4 + UNITY_DISPLAY_RANGE_INT,
#ifdef UNITY_SUPPORT_64
    UNITY_DISPLAY_STYLE_INT64    = 8 + UNITY_DISPLAY_RANGE_INT,
#endif

    UNITY_DISPLAY_STYLE_UINT     = (UNITY_INT_WIDTH / 8) + UNITY_DISPLAY_RANGE_UINT,
    UNITY_DISPLAY_STYLE_UINT8    = 1 + UNITY_DISPLAY_RANGE_UINT,
    UNITY_DISPLAY_STYLE_UINT16   = 2 + UNITY_DISPLAY_RANGE_UINT,
    UNITY_DISPLAY_STYLE_UINT32   = 4 + UNITY_DISPLAY_RANGE_UINT,
#ifdef UNITY_SUPPORT_64
    UNITY_DISPLAY_STYLE_UINT64   = 8 + UNITY_DISPLAY_RANGE_UINT,
#endif

    UNITY_DISPLAY_STYLE_HEX8     = 1 + UNITY_DISPLAY_RANGE_HEX,
    UNITY_DISPLAY_STYLE_HEX16    = 2 + UNITY_DISPLAY_RANGE_HEX,
    UNITY_DISPLAY_STYLE_HEX32    = 4 + UNITY_DISPLAY_RANGE_HEX,
#ifdef UNITY_SUPPORT_64
    UNITY_DISPLAY_STYLE_HEX64    = 8 + UNITY_DISPLAY_RANGE_HEX,
#endif

    UNITY_DISPLAY_STYLE_CHAR     = 1 + UNITY_DISPLAY_RANGE_CHAR + UNITY_DISPLAY_RANGE_INT,

    UNITY_DISPLAY_STYLE_UNKNOWN
} UNITY_DISPLAY_STYLE_T;

typedef enum
{
    UNITY_WITHIN           = 0x0,
    UNITY_EQUAL_TO         = 0x1,
    UNITY_GREATER_THAN     = 0x2,
    UNITY_GREATER_OR_EQUAL = 0x2 + UNITY_EQUAL_TO,
    UNITY_SMALLER_THAN     = 0x4,
    UNITY_SMALLER_OR_EQUAL = 0x4 + UNITY_EQUAL_TO,
    UNITY_NOT_EQUAL        = 0x0,
    UNITY_UNKNOWN
} UNITY_COMPARISON_T;

#ifndef UNITY_EXCLUDE_FLOAT
typedef enum UNITY_FLOAT_TRAIT
{
    UNITY_FLOAT_IS_NOT_INF       = 0,
    UNITY_FLOAT_IS_INF,
    UNITY_FLOAT_IS_NOT_NEG_INF,
    UNITY_FLOAT_IS_NEG_INF,
    UNITY_FLOAT_IS_NOT_NAN,
    UNITY_FLOAT_IS_NAN,
    UNITY_FLOAT_IS_NOT_DET,
    UNITY_FLOAT_IS_DET,
    UNITY_FLOAT_INVALID_TRAIT
} UNITY_FLOAT_TRAIT_T;
#endif

typedef enum
{
    UNITY_ARRAY_TO_VAL = 0,
    UNITY_ARRAY_TO_ARRAY,
    UNITY_ARRAY_UNKNOWN
} UNITY_FLAGS_T;

struct UNITY_STORAGE_T
{
    const char* TestFile;
    const char* CurrentTestName;
#ifndef UNITY_EXCLUDE_DETAILS
    const char* CurrentDetail1;
    const char* CurrentDetail2;
#endif
    UNITY_LINE_TYPE CurrentTestLineNumber;
    UNITY_COUNTER_TYPE NumberOfTests;
    UNITY_COUNTER_TYPE TestFailures;
    UNITY_COUNTER_TYPE TestIgnores;
    UNITY_COUNTER_TYPE CurrentTestFailed;
    UNITY_COUNTER_TYPE CurrentTestIgnored;
#ifdef UNITY_INCLUDE_EXEC_TIME
    UNITY_TIME_TYPE CurrentTestStartTime;
    UNITY_TIME_TYPE CurrentTestStopTime;
#endif
#ifndef UNITY_EXCLUDE_SETJMP_H
    jmp_buf AbortFrame;
#endif
};

extern struct UNITY_STORAGE_T Unity;

/*-------------------------------------------------------
 * Test Suite Management
 *-------------------------------------------------------*/

void UnityBegin(const char* filename);
int  UnityEnd(void);
void UnitySetTestFile(const char* filename);
void UnityConcludeTest(void);

#ifndef RUN_TEST
void UnityDefaultTestRun(UnityTestFunction Func, const char* FuncName, const int FuncLineNum);
#else
#define UNITY_SKIP_DEFAULT_RUNNER
#endif

/*-------------------------------------------------------
 * Details Support
 *-------------------------------------------------------*/

#ifdef UNITY_EXCLUDE_DETAILS
#define UNITY_CLR_DETAILS()
#define UNITY_SET_DETAIL(d1)
#define UNITY_SET_DETAILS(d1,d2)
#else
#define UNITY_CLR_DETAILS()      { Unity.CurrentDetail1 = 0;   Unity.CurrentDetail2 = 0;  }
#define UNITY_SET_DETAIL(d1)     { Unity.CurrentDetail1 = (d1);  Unity.CurrentDetail2 = 0;  }
#define UNITY_SET_DETAILS(d1,d2) { Unity.CurrentDetail1 = (d1);  Unity.CurrentDetail2 = (d2); }

#ifndef UNITY_DETAIL1_NAME
#define UNITY_DETAIL1_NAME "Function"
#endif

#ifndef UNITY_DETAIL2_NAME
#define UNITY_DETAIL2_NAME "Argument"
#endif
#endif

#ifdef UNITY_PRINT_TEST_CONTEXT
void UNITY_PRINT_TEST_CONTEXT(void);
#endif

/*-------------------------------------------------------
 * Test Output
 *-------------------------------------------------------*/

void UnityPrint(const char* string);

#ifdef UNITY_INCLUDE_PRINT_FORMATTED
void UnityPrintF(const UNITY_LINE_TYPE line, const char* format, ...);
#endif

void UnityPrintLen(const char* string, const UNITY_UINT32 length);
void UnityPrintMask(const UNITY_UINT mask, const UNITY_UINT number);
void UnityPrintNumberByStyle(const UNITY_INT number, const UNITY_DISPLAY_STYLE_T style);
void UnityPrintNumber(const UNITY_INT number_to_print);
void UnityPrintNumberUnsigned(const UNITY_UINT number);
void UnityPrintNumberHex(const UNITY_UINT number, const char nibbles_to_print);

#ifndef UNITY_EXCLUDE_FLOAT_PRINT
void UnityPrintFloat(const UNITY_DOUBLE input_number);
#endif

/*-------------------------------------------------------
 * Test Assertion Functions
 *-------------------------------------------------------
 *  Use the macros below this section instead of calling
 *  these directly. The macros have a consistent naming
 *  convention and will pull in file and line information
 *  for you. */

void UnityAssertEqualNumber(const UNITY_INT expected,
                            const UNITY_INT actual,
                            const char* msg,
                            const UNITY_LINE_TYPE lineNumber,
                            const UNITY_DISPLAY_STYLE_T style);

void UnityAssertGreaterOrLessOrEqualNumber(const UNITY_INT threshold,
                                           const UNITY_INT actual,
                                           const UNITY_COMPARISON_T compare,
                                           const char *msg,
                                           const UNITY_LINE_TYPE lineNumber,
                                           const UNITY_DISPLAY_STYLE_T style);

void UnityAssertEqualIntArray(UNITY_INTERNAL_PTR expected,
                              UNITY_INTERNAL_PTR actual,
                              const UNITY_UINT32 num_elements,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber,
                              const UNITY_DISPLAY_STYLE_T style,
                              const UNITY_FLAGS_T flags);

void UnityAssertBits(const UNITY_INT mask,
                     const UNITY_INT expected,
                     const UNITY_INT actual,
                     const char* msg,
                     const UNITY_LINE_TYPE lineNumber);

void UnityAssertEqualString(const char* expected,
                            const char* actual,
                            const char* msg,
                            const UNITY_LINE_TYPE lineNumber);

void UnityAssertEqualStringLen(const char* expected,
                            const char* actual,
                            const UNITY_UINT32 length,
                            const char* msg,
                            const UNITY_LINE_TYPE lineNumber);

void UnityAssertEqualStringArray( UNITY_INTERNAL_PTR expected,
                                  const char** actual,
                                  const UNITY_UINT32 num_elements,
                                  const char* msg,
                                  const UNITY_LINE_TYPE lineNumber,
                                  const UNITY_FLAGS_T flags);

void UnityAssertEqualMemory( UNITY_INTERNAL_PTR expected,
                             UNITY_INTERNAL_PTR actual,
                             const UNITY_UINT32 length,
                             const UNITY_UINT32 num_elements,
                             const char* msg,
                             const UNITY_LINE_TYPE lineNumber,
                             const UNITY_FLAGS_T flags);

void UnityAssertNumbersWithin(const UNITY_UINT delta,
                              const UNITY_INT expected,
                              const UNITY_INT actual,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber,
                              const UNITY_DISPLAY_STYLE_T style);

void UnityAssertNumbersArrayWithin(const UNITY_UINT delta,
                                   UNITY_INTERNAL_PTR expected,
                                   UNITY_INTERNAL_PTR actual,
                                   const UNITY_UINT32 num_elements,
                                   const char* msg,
                                   const UNITY_LINE_TYPE lineNumber,
                                   const UNITY_DISPLAY_STYLE_T style,
                                   const UNITY_FLAGS_T flags);

#ifndef UNITY_EXCLUDE_SETJMP_H
UNITY_NORETURN void UnityFail(const char* message, const UNITY_LINE_TYPE line);
UNITY_NORETURN void UnityIgnore(const char* message, const UNITY_LINE_TYPE line);
#else
void UnityFail(const char* message, const UNITY_LINE_TYPE line);
void UnityIgnore(const char* message, const UNITY_LINE_TYPE line);
#endif

void UnityMessage(const char* message, const UNITY_LINE_TYPE line);

#ifndef UNITY_EXCLUDE_FLOAT
void UnityAssertFloatsWithin(const UNITY_FLOAT delta,
                             const UNITY_FLOAT expected,
                             const UNITY_FLOAT actual,
                             const char* msg,
                             const UNITY_LINE_TYPE lineNumber);

void UnityAssertEqualFloatArray(UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* expected,
                                UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* actual,
                                const UNITY_UINT32 num_elements,
                                const char* msg,
                                const UNITY_LINE_TYPE lineNumber,
                                const UNITY_FLAGS_T flags);

void UnityAssertFloatSpecial(const UNITY_FLOAT actual,
                             const char* msg,
                             const UNITY_LINE_TYPE lineNumber,
                             const UNITY_FLOAT_TRAIT_T style);
#endif

#ifndef UNITY_EXCLUDE_DOUBLE
void UnityAssertDoublesWithin(const UNITY_DOUBLE delta,
                              const UNITY_DOUBLE expected,
                              const UNITY_DOUBLE actual,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber);

void UnityAssertEqualDoubleArray(UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* expected,
                                 UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* actual,
                                 const UNITY_UINT32 num_elements,
                                 const char* msg,
                                 const UNITY_LINE_TYPE lineNumber,
                                 const UNITY_FLAGS_T flags);

void UnityAssertDoubleSpecial(const UNITY_DOUBLE actual,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber,
                              const UNITY_FLOAT_TRAIT_T style);
#endif

/*-------------------------------------------------------
 * Helpers
 *-------------------------------------------------------*/

UNITY_INTERNAL_PTR UnityNumToPtr(const UNITY_INT num, const UNITY_UINT8 size);
#ifndef UNITY_EXCLUDE_FLOAT
UNITY_INTERNAL_PTR UnityFloatToPtr(const float num);
#endif
#ifndef UNITY_EXCLUDE_DOUBLE
UNITY_INTERNAL_PTR UnityDoubleToPtr(const double num);
#endif

/*-------------------------------------------------------
 * Error Strings We Might Need
 *-------------------------------------------------------*/

extern const char UnityStrOk[];
extern const char UnityStrPass[];
extern const char UnityStrFail[];
extern const char UnityStrIgnore[];

extern const char UnityStrErrFloat[];
extern const char UnityStrErrDouble[];
extern const char UnityStrErr64[];
extern const char UnityStrErrShorthand[];

/*-------------------------------------------------------
 * Test Running Macros
 *-------------------------------------------------------*/

#ifndef UNITY_EXCLUDE_SETJMP_H
#define TEST_PROTECT() (setjmp(Unity.AbortFrame) == 0)
#define TEST_ABORT() longjmp(Unity.AbortFrame, 1)
#else
#define TEST_PROTECT() 1
#define TEST_ABORT() return
#endif

/* This tricky series of macros gives us an optional line argument to treat it as RUN_TEST(func, num=__LINE__) */
#ifndef RUN_TEST
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901L
#define UNITY_SUPPORT_VARIADIC_MACROS
#endif
#endif
#ifdef UNITY_SUPPORT_VARIADIC_MACROS
#define RUN_TEST(...) RUN_TEST_AT_LINE(__VA_ARGS__, __LINE__, throwaway)
#define RUN_TEST_AT_LINE(func, line, ...) UnityDefaultTestRun(func, #func, line)
#endif
#endif

/* If we can't do the tricky version, we'll just have to require them to always include the line number */
#ifndef RUN_TEST
#ifdef CMOCK
#define RUN_TEST(func, num) UnityDefaultTestRun(func, #func, num)
#else
#define RUN_TEST(func) UnityDefaultTestRun(func, #func, __LINE__)
#endif
#endif

#define TEST_LINE_NUM (Unity.CurrentTestLineNumber)
#define TEST_IS_IGNORED (Unity.CurrentTestIgnored)
#define UNITY_NEW_TEST(a) \
    Unity.CurrentTestName = (a); \
    Unity.CurrentTestLineNumber = (UNITY_LINE_TYPE)(__LINE__); \
    Unity.NumberOfTests++;

#ifndef UNITY_BEGIN
#define UNITY_BEGIN() UnityBegin(__FILE__)
#endif

#ifndef UNITY_END
#define UNITY_END() UnityEnd()
#endif

#ifndef UNITY_SHORTHAND_AS_INT
#ifndef UNITY_SHORTHAND_AS_MEM
#ifndef UNITY_SHORTHAND_AS_NONE
#ifndef UNITY_SHORTHAND_AS_RAW
#define UNITY_SHORTHAND_AS_OLD
#endif
#endif
#endif
#endif

/*-----------------------------------------------
 * Command Line Argument Support
 *-----------------------------------------------*/

#ifdef UNITY_USE_COMMAND_LINE_ARGS
int UnityParseOptions(int argc, char** argv);
int UnityTestMatches(void);
#endif

/*-------------------------------------------------------
 * Basic Fail and Ignore
 *-------------------------------------------------------*/

#define UNITY_TEST_FAIL(line, message)   UnityFail(   (message), (UNITY_LINE_TYPE)(line))
#define UNITY_TEST_IGNORE(line, message) UnityIgnore( (message), (UNITY_LINE_TYPE)(line))

/*-------------------------------------------------------
 * Test Asserts
 *-------------------------------------------------------*/

#define UNITY_TEST_ASSERT(condition, line, message)                                              do {if (condition) {} else {UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), (message));}} while(0)
#define UNITY_TEST_ASSERT_NULL(pointer, line, message)                                           UNITY_TEST_ASSERT(((pointer) == NULL),  (UNITY_LINE_TYPE)(line), (message))
#define UNITY_TEST_ASSERT_NOT_NULL(pointer, line, message)                                       UNITY_TEST_ASSERT(((pointer) != NULL),  (UNITY_LINE_TYPE)(line), (message))
#define UNITY_TEST_ASSERT_EMPTY(pointer, line, message)                                          UNITY_TEST_ASSERT(((pointer[0]) == 0),  (UNITY_LINE_TYPE)(line), (message))
#define UNITY_TEST_ASSERT_NOT_EMPTY(pointer, line, message)                                      UNITY_TEST_ASSERT(((pointer[0]) != 0),  (UNITY_LINE_TYPE)(line), (message))

#define UNITY_TEST_ASSERT_EQUAL_INT(expected, actual, line, message)                             UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT)
#define UNITY_TEST_ASSERT_EQUAL_INT8(expected, actual, line, message)                            UnityAssertEqualNumber((UNITY_INT)(UNITY_INT8 )(expected), (UNITY_INT)(UNITY_INT8 )(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8)
#define UNITY_TEST_ASSERT_EQUAL_INT16(expected, actual, line, message)                           UnityAssertEqualNumber((UNITY_INT)(UNITY_INT16)(expected), (UNITY_INT)(UNITY_INT16)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16)
#define UNITY_TEST_ASSERT_EQUAL_INT32(expected, actual, line, message)                           UnityAssertEqualNumber((UNITY_INT)(UNITY_INT32)(expected), (UNITY_INT)(UNITY_INT32)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32)
#define UNITY_TEST_ASSERT_EQUAL_UINT(expected, actual, line, message)                            UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT)
#define UNITY_TEST_ASSERT_EQUAL_UINT8(expected, actual, line, message)                           UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT8 )(expected), (UNITY_INT)(UNITY_UINT8 )(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8)
#define UNITY_TEST_ASSERT_EQUAL_UINT16(expected, actual, line, message)                          UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT16)(expected), (UNITY_INT)(UNITY_UINT16)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16)
#define UNITY_TEST_ASSERT_EQUAL_UINT32(expected, actual, line, message)                          UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT32)(expected), (UNITY_INT)(UNITY_UINT32)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32)
#define UNITY_TEST_ASSERT_EQUAL_HEX8(expected, actual, line, message)                            UnityAssertEqualNumber((UNITY_INT)(UNITY_INT8 )(expected), (UNITY_INT)(UNITY_INT8 )(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8)
#define UNITY_TEST_ASSERT_EQUAL_HEX16(expected, actual, line, message)                           UnityAssertEqualNumber((UNITY_INT)(UNITY_INT16)(expected), (UNITY_INT)(UNITY_INT16)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16)
#define UNITY_TEST_ASSERT_EQUAL_HEX32(expected, actual, line, message)                           UnityAssertEqualNumber((UNITY_INT)(UNITY_INT32)(expected), (UNITY_INT)(UNITY_INT32)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32)
#define UNITY_TEST_ASSERT_EQUAL_CHAR(expected, actual, line, message)                            UnityAssertEqualNumber((UNITY_INT)(UNITY_INT8 )(expected), (UNITY_INT)(UNITY_INT8 )(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR)
#define UNITY_TEST_ASSERT_BITS(mask, expected, actual, line, message)                            UnityAssertBits((UNITY_INT)(mask), (UNITY_INT)(expected), (UNITY_INT)(actual), (message), (UNITY_LINE_TYPE)(line))

#define UNITY_TEST_ASSERT_NOT_EQUAL_INT(threshold, actual, line, message)                        UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold),               (UNITY_INT)(actual),               UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT)
#define UNITY_TEST_ASSERT_NOT_EQUAL_INT8(threshold, actual, line, message)                       UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 )(threshold),  (UNITY_INT)(UNITY_INT8 )(actual),  UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8)
#define UNITY_TEST_ASSERT_NOT_EQUAL_INT16(threshold, actual, line, message)                      UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT16)(threshold),  (UNITY_INT)(UNITY_INT16)(actual),  UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16)
#define UNITY_TEST_ASSERT_NOT_EQUAL_INT32(threshold, actual, line, message)                      UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT32)(threshold),  (UNITY_INT)(UNITY_INT32)(actual),  UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32)
#define UNITY_TEST_ASSERT_NOT_EQUAL_UINT(threshold, actual, line, message)                       UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold),               (UNITY_INT)(actual),               UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT)
#define UNITY_TEST_ASSERT_NOT_EQUAL_UINT8(threshold, actual, line, message)                      UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8)
#define UNITY_TEST_ASSERT_NOT_EQUAL_UINT16(threshold, actual, line, message)                     UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16)
#define UNITY_TEST_ASSERT_NOT_EQUAL_UINT32(threshold, actual, line, message)                     UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32)
#define UNITY_TEST_ASSERT_NOT_EQUAL_HEX8(threshold, actual, line, message)                       UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8)
#define UNITY_TEST_ASSERT_NOT_EQUAL_HEX16(threshold, actual, line, message)                      UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16)
#define UNITY_TEST_ASSERT_NOT_EQUAL_HEX32(threshold, actual, line, message)                      UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32)
#define UNITY_TEST_ASSERT_NOT_EQUAL_CHAR(threshold, actual, line, message)                       UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 )(threshold),  (UNITY_INT)(UNITY_INT8 )(actual),  UNITY_NOT_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR)

#define UNITY_TEST_ASSERT_GREATER_THAN_INT(threshold, actual, line, message)                     UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold),              (UNITY_INT)(actual),              UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT)
#define UNITY_TEST_ASSERT_GREATER_THAN_INT8(threshold, actual, line, message)                    UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 )(threshold), (UNITY_INT)(UNITY_INT8 )(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8)
#define UNITY_TEST_ASSERT_GREATER_THAN_INT16(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT16)(threshold), (UNITY_INT)(UNITY_INT16)(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16)
#define UNITY_TEST_ASSERT_GREATER_THAN_INT32(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT32)(threshold), (UNITY_INT)(UNITY_INT32)(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32)
#define UNITY_TEST_ASSERT_GREATER_THAN_UINT(threshold, actual, line, message)                    UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold),              (UNITY_INT)(actual),              UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT)
#define UNITY_TEST_ASSERT_GREATER_THAN_UINT8(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8)
#define UNITY_TEST_ASSERT_GREATER_THAN_UINT16(threshold, actual, line, message)                  UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16)
#define UNITY_TEST_ASSERT_GREATER_THAN_UINT32(threshold, actual, line, message)                  UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32)
#define UNITY_TEST_ASSERT_GREATER_THAN_HEX8(threshold, actual, line, message)                    UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8)
#define UNITY_TEST_ASSERT_GREATER_THAN_HEX16(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16)
#define UNITY_TEST_ASSERT_GREATER_THAN_HEX32(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32)
#define UNITY_TEST_ASSERT_GREATER_THAN_CHAR(threshold, actual, line, message)                    UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 )(threshold), (UNITY_INT)(UNITY_INT8 )(actual), UNITY_GREATER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR)

#define UNITY_TEST_ASSERT_SMALLER_THAN_INT(threshold, actual, line, message)                     UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold),              (UNITY_INT)(actual),              UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT)
#define UNITY_TEST_ASSERT_SMALLER_THAN_INT8(threshold, actual, line, message)                    UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 )(threshold), (UNITY_INT)(UNITY_INT8 )(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8)
#define UNITY_TEST_ASSERT_SMALLER_THAN_INT16(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT16)(threshold), (UNITY_INT)(UNITY_INT16)(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16)
#define UNITY_TEST_ASSERT_SMALLER_THAN_INT32(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT32)(threshold), (UNITY_INT)(UNITY_INT32)(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32)
#define UNITY_TEST_ASSERT_SMALLER_THAN_UINT(threshold, actual, line, message)                    UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold),              (UNITY_INT)(actual),              UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT)
#define UNITY_TEST_ASSERT_SMALLER_THAN_UINT8(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8)
#define UNITY_TEST_ASSERT_SMALLER_THAN_UINT16(threshold, actual, line, message)                  UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16)
#define UNITY_TEST_ASSERT_SMALLER_THAN_UINT32(threshold, actual, line, message)                  UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32)
#define UNITY_TEST_ASSERT_SMALLER_THAN_HEX8(threshold, actual, line, message)                    UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8)
#define UNITY_TEST_ASSERT_SMALLER_THAN_HEX16(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16)
#define UNITY_TEST_ASSERT_SMALLER_THAN_HEX32(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32)
#define UNITY_TEST_ASSERT_SMALLER_THAN_CHAR(threshold, actual, line, message)                    UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 )(threshold), (UNITY_INT)(UNITY_INT8 )(actual), UNITY_SMALLER_THAN, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR)

#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT(threshold, actual, line, message)                 UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)              (threshold), (UNITY_INT)              (actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT8(threshold, actual, line, message)                UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 ) (threshold), (UNITY_INT)(UNITY_INT8 ) (actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT16(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT16) (threshold), (UNITY_INT)(UNITY_INT16) (actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT32(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT32) (threshold), (UNITY_INT)(UNITY_INT32) (actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT(threshold, actual, line, message)                UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)              (threshold), (UNITY_INT)              (actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT8(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT16(threshold, actual, line, message)              UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT32(threshold, actual, line, message)              UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX8(threshold, actual, line, message)                UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX16(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX32(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_CHAR(threshold, actual, line, message)                UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 ) (threshold), (UNITY_INT)(UNITY_INT8 ) (actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR)

#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT(threshold, actual, line, message)                 UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)             (threshold),  (UNITY_INT)              (actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT8(threshold, actual, line, message)                UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 )(threshold),  (UNITY_INT)(UNITY_INT8 ) (actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT16(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT16)(threshold),  (UNITY_INT)(UNITY_INT16) (actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT32(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT32)(threshold),  (UNITY_INT)(UNITY_INT32) (actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT(threshold, actual, line, message)                UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)             (threshold),  (UNITY_INT)              (actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT8(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT16(threshold, actual, line, message)              UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT32(threshold, actual, line, message)              UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX8(threshold, actual, line, message)                UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT8 )(threshold), (UNITY_INT)(UNITY_UINT8 )(actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX16(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT16)(threshold), (UNITY_INT)(UNITY_UINT16)(actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX32(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_UINT32)(threshold), (UNITY_INT)(UNITY_UINT32)(actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_CHAR(threshold, actual, line, message)                UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(UNITY_INT8 )(threshold),  (UNITY_INT)(UNITY_INT8 ) (actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR)

#define UNITY_TEST_ASSERT_INT_WITHIN(delta, expected, actual, line, message)                     UnityAssertNumbersWithin(              (delta), (UNITY_INT)                          (expected), (UNITY_INT)                          (actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT)
#define UNITY_TEST_ASSERT_INT8_WITHIN(delta, expected, actual, line, message)                    UnityAssertNumbersWithin((UNITY_UINT8 )(delta), (UNITY_INT)(UNITY_INT8 )             (expected), (UNITY_INT)(UNITY_INT8 )             (actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8)
#define UNITY_TEST_ASSERT_INT16_WITHIN(delta, expected, actual, line, message)                   UnityAssertNumbersWithin((UNITY_UINT16)(delta), (UNITY_INT)(UNITY_INT16)             (expected), (UNITY_INT)(UNITY_INT16)             (actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16)
#define UNITY_TEST_ASSERT_INT32_WITHIN(delta, expected, actual, line, message)                   UnityAssertNumbersWithin((UNITY_UINT32)(delta), (UNITY_INT)(UNITY_INT32)             (expected), (UNITY_INT)(UNITY_INT32)             (actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32)
#define UNITY_TEST_ASSERT_UINT_WITHIN(delta, expected, actual, line, message)                    UnityAssertNumbersWithin(              (delta), (UNITY_INT)                          (expected), (UNITY_INT)                          (actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT)
#define UNITY_TEST_ASSERT_UINT8_WITHIN(delta, expected, actual, line, message)                   UnityAssertNumbersWithin((UNITY_UINT8 )(delta), (UNITY_INT)(UNITY_UINT)(UNITY_UINT8 )(expected), (UNITY_INT)(UNITY_UINT)(UNITY_UINT8 )(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8)
#define UNITY_TEST_ASSERT_UINT16_WITHIN(delta, expected, actual, line, message)                  UnityAssertNumbersWithin((UNITY_UINT16)(delta), (UNITY_INT)(UNITY_UINT)(UNITY_UINT16)(expected), (UNITY_INT)(UNITY_UINT)(UNITY_UINT16)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16)
#define UNITY_TEST_ASSERT_UINT32_WITHIN(delta, expected, actual, line, message)                  UnityAssertNumbersWithin((UNITY_UINT32)(delta), (UNITY_INT)(UNITY_UINT)(UNITY_UINT32)(expected), (UNITY_INT)(UNITY_UINT)(UNITY_UINT32)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32)
#define UNITY_TEST_ASSERT_HEX8_WITHIN(delta, expected, actual, line, message)                    UnityAssertNumbersWithin((UNITY_UINT8 )(delta), (UNITY_INT)(UNITY_UINT)(UNITY_UINT8 )(expected), (UNITY_INT)(UNITY_UINT)(UNITY_UINT8 )(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8)
#define UNITY_TEST_ASSERT_HEX16_WITHIN(delta, expected, actual, line, message)                   UnityAssertNumbersWithin((UNITY_UINT16)(delta), (UNITY_INT)(UNITY_UINT)(UNITY_UINT16)(expected), (UNITY_INT)(UNITY_UINT)(UNITY_UINT16)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16)
#define UNITY_TEST_ASSERT_HEX32_WITHIN(delta, expected, actual, line, message)                   UnityAssertNumbersWithin((UNITY_UINT32)(delta), (UNITY_INT)(UNITY_UINT)(UNITY_UINT32)(expected), (UNITY_INT)(UNITY_UINT)(UNITY_UINT32)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32)
#define UNITY_TEST_ASSERT_CHAR_WITHIN(delta, expected, actual, line, message)                    UnityAssertNumbersWithin((UNITY_UINT8 )(delta), (UNITY_INT)(UNITY_INT8 )             (expected), (UNITY_INT)(UNITY_INT8 )             (actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR)

#define UNITY_TEST_ASSERT_INT_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)     UnityAssertNumbersArrayWithin(              (delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_INT8_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)    UnityAssertNumbersArrayWithin((UNITY_UINT8 )(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_INT16_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)   UnityAssertNumbersArrayWithin((UNITY_UINT16)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_INT32_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)   UnityAssertNumbersArrayWithin((UNITY_UINT32)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_UINT_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)    UnityAssertNumbersArrayWithin(              (delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_UINT8_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)   UnityAssertNumbersArrayWithin((UNITY_UINT16)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_UINT16_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)  UnityAssertNumbersArrayWithin((UNITY_UINT16)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_UINT32_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)  UnityAssertNumbersArrayWithin((UNITY_UINT32)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_HEX8_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)    UnityAssertNumbersArrayWithin((UNITY_UINT8 )(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_HEX16_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)   UnityAssertNumbersArrayWithin((UNITY_UINT16)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_HEX32_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)   UnityAssertNumbersArrayWithin((UNITY_UINT32)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_CHAR_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)    UnityAssertNumbersArrayWithin((UNITY_UINT8 )(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), ((UNITY_UINT32)(num_elements)), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR, UNITY_ARRAY_TO_ARRAY)


#define UNITY_TEST_ASSERT_EQUAL_PTR(expected, actual, line, message)                             UnityAssertEqualNumber((UNITY_PTR_TO_INT)(expected), (UNITY_PTR_TO_INT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_POINTER)
#define UNITY_TEST_ASSERT_EQUAL_STRING(expected, actual, line, message)                          UnityAssertEqualString((const char*)(expected), (const char*)(actual), (message), (UNITY_LINE_TYPE)(line))
#define UNITY_TEST_ASSERT_EQUAL_STRING_LEN(expected, actual, len, line, message)                 UnityAssertEqualStringLen((const char*)(expected), (const char*)(actual), (UNITY_UINT32)(len), (message), (UNITY_LINE_TYPE)(line))
#define UNITY_TEST_ASSERT_EQUAL_MEMORY(expected, actual, len, line, message)                     UnityAssertEqualMemory((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(len), 1, (message), (UNITY_LINE_TYPE)(line), UNITY_ARRAY_TO_ARRAY)

#define UNITY_TEST_ASSERT_EQUAL_INT_ARRAY(expected, actual, num_elements, line, message)         UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT,     UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8,    UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_INT16_ARRAY(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16,   UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_INT32_ARRAY(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32,   UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_UINT_ARRAY(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT,    UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8,   UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_UINT16_ARRAY(expected, actual, num_elements, line, message)      UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16,  UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, num_elements, line, message)      UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32,  UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8,    UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_HEX16_ARRAY(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16,   UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32,   UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_PTR_ARRAY(expected, actual, num_elements, line, message)         UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_POINTER, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_STRING_ARRAY(expected, actual, num_elements, line, message)      UnityAssertEqualStringArray((UNITY_INTERNAL_PTR)(expected), (const char**)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_MEMORY_ARRAY(expected, actual, len, num_elements, line, message) UnityAssertEqualMemory((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(len), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR,    UNITY_ARRAY_TO_ARRAY)

#define UNITY_TEST_ASSERT_EACH_EQUAL_INT(expected, actual, num_elements, line, message)          UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)              (expected), (UNITY_INT_WIDTH / 8)),          (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT,     UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_INT8(expected, actual, num_elements, line, message)         UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_INT8  )(expected), 1),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT8,    UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_INT16(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_INT16 )(expected), 2),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT16,   UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_INT32(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_INT32 )(expected), 4),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT32,   UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_UINT(expected, actual, num_elements, line, message)         UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)              (expected), (UNITY_INT_WIDTH / 8)),          (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT,    UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_UINT8(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_UINT8 )(expected), 1),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT8,   UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_UINT16(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_UINT16)(expected), 2),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT16,  UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_UINT32(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_UINT32)(expected), 4),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT32,  UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_HEX8(expected, actual, num_elements, line, message)         UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_INT8  )(expected), 1),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX8,    UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_HEX16(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_INT16 )(expected), 2),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX16,   UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_HEX32(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_INT32 )(expected), 4),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX32,   UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_PTR(expected, actual, num_elements, line, message)          UnityAssertEqualIntArray(UnityNumToPtr((UNITY_PTR_TO_INT)       (expected), (UNITY_POINTER_WIDTH / 8)),      (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_POINTER, UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_STRING(expected, actual, num_elements, line, message)       UnityAssertEqualStringArray((UNITY_INTERNAL_PTR)(expected), (const char**)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_MEMORY(expected, actual, len, num_elements, line, message)  UnityAssertEqualMemory((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(len), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_CHAR(expected, actual, num_elements, line, message)         UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_INT8  )(expected), 1),                              (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_CHAR,    UNITY_ARRAY_TO_VAL)

#ifdef UNITY_SUPPORT_64
#define UNITY_TEST_ASSERT_EQUAL_INT64(expected, actual, line, message)                           UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64)
#define UNITY_TEST_ASSERT_EQUAL_UINT64(expected, actual, line, message)                          UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64)
#define UNITY_TEST_ASSERT_EQUAL_HEX64(expected, actual, line, message)                           UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64)
#define UNITY_TEST_ASSERT_EQUAL_INT64_ARRAY(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64,  UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_UINT64_ARRAY(expected, actual, num_elements, line, message)      UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EQUAL_HEX64_ARRAY(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray((UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64,  UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EACH_EQUAL_INT64(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_INT64)(expected), 8), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64,  UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_UINT64(expected, actual, num_elements, line, message)       UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_UINT64)(expected), 8), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64, UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_EACH_EQUAL_HEX64(expected, actual, num_elements, line, message)        UnityAssertEqualIntArray(UnityNumToPtr((UNITY_INT)(UNITY_INT64)(expected), 8), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64,  UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_INT64_WITHIN(delta, expected, actual, line, message)                   UnityAssertNumbersWithin((delta), (UNITY_INT)(expected), (UNITY_INT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64)
#define UNITY_TEST_ASSERT_UINT64_WITHIN(delta, expected, actual, line, message)                  UnityAssertNumbersWithin((delta), (UNITY_INT)(expected), (UNITY_INT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64)
#define UNITY_TEST_ASSERT_HEX64_WITHIN(delta, expected, actual, line, message)                   UnityAssertNumbersWithin((delta), (UNITY_INT)(expected), (UNITY_INT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64)
#define UNITY_TEST_ASSERT_NOT_EQUAL_INT64(threshold, actual, line, message)                      UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_NOT_EQUAL,        (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64)
#define UNITY_TEST_ASSERT_NOT_EQUAL_UINT64(threshold, actual, line, message)                     UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_NOT_EQUAL,        (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64)
#define UNITY_TEST_ASSERT_NOT_EQUAL_HEX64(threshold, actual, line, message)                      UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_NOT_EQUAL,        (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64)
#define UNITY_TEST_ASSERT_GREATER_THAN_INT64(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_GREATER_THAN,     (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64)
#define UNITY_TEST_ASSERT_GREATER_THAN_UINT64(threshold, actual, line, message)                  UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_GREATER_THAN,     (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64)
#define UNITY_TEST_ASSERT_GREATER_THAN_HEX64(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_GREATER_THAN,     (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT64(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT64(threshold, actual, line, message)              UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX64(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_GREATER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64)
#define UNITY_TEST_ASSERT_SMALLER_THAN_INT64(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_SMALLER_THAN,     (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64)
#define UNITY_TEST_ASSERT_SMALLER_THAN_UINT64(threshold, actual, line, message)                  UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_SMALLER_THAN,     (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64)
#define UNITY_TEST_ASSERT_SMALLER_THAN_HEX64(threshold, actual, line, message)                   UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_SMALLER_THAN,     (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT64(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT64(threshold, actual, line, message)              UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX64(threshold, actual, line, message)               UnityAssertGreaterOrLessOrEqualNumber((UNITY_INT)(threshold), (UNITY_INT)(actual), UNITY_SMALLER_OR_EQUAL, (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64)
#define UNITY_TEST_ASSERT_INT64_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)   UnityAssertNumbersArrayWithin((UNITY_UINT64)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_INT64, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_UINT64_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)  UnityAssertNumbersArrayWithin((UNITY_UINT64)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_UINT64, UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_HEX64_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)   UnityAssertNumbersArrayWithin((UNITY_UINT64)(delta), (UNITY_INTERNAL_PTR)(expected), (UNITY_INTERNAL_PTR)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_DISPLAY_STYLE_HEX64, UNITY_ARRAY_TO_ARRAY)
#else
#define UNITY_TEST_ASSERT_EQUAL_INT64(expected, actual, line, message)                           UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_EQUAL_UINT64(expected, actual, line, message)                          UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_EQUAL_HEX64(expected, actual, line, message)                           UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_EQUAL_INT64_ARRAY(expected, actual, num_elements, line, message)       UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_EQUAL_UINT64_ARRAY(expected, actual, num_elements, line, message)      UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_EQUAL_HEX64_ARRAY(expected, actual, num_elements, line, message)       UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_INT64_WITHIN(delta, expected, actual, line, message)                   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_UINT64_WITHIN(delta, expected, actual, line, message)                  UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_HEX64_WITHIN(delta, expected, actual, line, message)                   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_GREATER_THAN_INT64(threshold, actual, line, message)                   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_GREATER_THAN_UINT64(threshold, actual, line, message)                  UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_GREATER_THAN_HEX64(threshold, actual, line, message)                   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT64(threshold, actual, line, message)               UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT64(threshold, actual, line, message)              UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX64(threshold, actual, line, message)               UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_SMALLER_THAN_INT64(threshold, actual, line, message)                   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_SMALLER_THAN_UINT64(threshold, actual, line, message)                  UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_SMALLER_THAN_HEX64(threshold, actual, line, message)                   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT64(threshold, actual, line, message)               UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT64(threshold, actual, line, message)              UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX64(threshold, actual, line, message)               UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_INT64_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_UINT64_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)  UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#define UNITY_TEST_ASSERT_HEX64_ARRAY_WITHIN(delta, expected, actual, num_elements, line, message)   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErr64)
#endif

#ifdef UNITY_EXCLUDE_FLOAT
#define UNITY_TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual, line, message)                   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_EQUAL_FLOAT(expected, actual, line, message)                           UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, actual, num_elements, line, message)       UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_EACH_EQUAL_FLOAT(expected, actual, num_elements, line, message)        UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_FLOAT_IS_INF(actual, line, message)                                    UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_FLOAT_IS_NEG_INF(actual, line, message)                                UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_FLOAT_IS_NAN(actual, line, message)                                    UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_FLOAT_IS_DETERMINATE(actual, line, message)                            UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_FLOAT_IS_NOT_INF(actual, line, message)                                UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_FLOAT_IS_NOT_NEG_INF(actual, line, message)                            UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_FLOAT_IS_NOT_NAN(actual, line, message)                                UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#define UNITY_TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE(actual, line, message)                        UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrFloat)
#else
#define UNITY_TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual, line, message)                   UnityAssertFloatsWithin((UNITY_FLOAT)(delta), (UNITY_FLOAT)(expected), (UNITY_FLOAT)(actual), (message), (UNITY_LINE_TYPE)(line))
#define UNITY_TEST_ASSERT_EQUAL_FLOAT(expected, actual, line, message)                           UNITY_TEST_ASSERT_FLOAT_WITHIN((UNITY_FLOAT)(expected) * (UNITY_FLOAT)UNITY_FLOAT_PRECISION, (UNITY_FLOAT)(expected), (UNITY_FLOAT)(actual), (UNITY_LINE_TYPE)(line), (message))
#define UNITY_TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, actual, num_elements, line, message)       UnityAssertEqualFloatArray((UNITY_FLOAT*)(expected), (UNITY_FLOAT*)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EACH_EQUAL_FLOAT(expected, actual, num_elements, line, message)        UnityAssertEqualFloatArray(UnityFloatToPtr(expected), (UNITY_FLOAT*)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_FLOAT_IS_INF(actual, line, message)                                    UnityAssertFloatSpecial((UNITY_FLOAT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_INF)
#define UNITY_TEST_ASSERT_FLOAT_IS_NEG_INF(actual, line, message)                                UnityAssertFloatSpecial((UNITY_FLOAT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NEG_INF)
#define UNITY_TEST_ASSERT_FLOAT_IS_NAN(actual, line, message)                                    UnityAssertFloatSpecial((UNITY_FLOAT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NAN)
#define UNITY_TEST_ASSERT_FLOAT_IS_DETERMINATE(actual, line, message)                            UnityAssertFloatSpecial((UNITY_FLOAT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_DET)
#define UNITY_TEST_ASSERT_FLOAT_IS_NOT_INF(actual, line, message)                                UnityAssertFloatSpecial((UNITY_FLOAT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NOT_INF)
#define UNITY_TEST_ASSERT_FLOAT_IS_NOT_NEG_INF(actual, line, message)                            UnityAssertFloatSpecial((UNITY_FLOAT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NOT_NEG_INF)
#define UNITY_TEST_ASSERT_FLOAT_IS_NOT_NAN(actual, line, message)                                UnityAssertFloatSpecial((UNITY_FLOAT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NOT_NAN)
#define UNITY_TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE(actual, line, message)                        UnityAssertFloatSpecial((UNITY_FLOAT)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NOT_DET)
#endif

#ifdef UNITY_EXCLUDE_DOUBLE
#define UNITY_TEST_ASSERT_DOUBLE_WITHIN(delta, expected, actual, line, message)                  UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_EQUAL_DOUBLE(expected, actual, line, message)                          UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_EQUAL_DOUBLE_ARRAY(expected, actual, num_elements, line, message)      UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_EACH_EQUAL_DOUBLE(expected, actual, num_elements, line, message)       UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_DOUBLE_IS_INF(actual, line, message)                                   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NEG_INF(actual, line, message)                               UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NAN(actual, line, message)                                   UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_DOUBLE_IS_DETERMINATE(actual, line, message)                           UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NOT_INF(actual, line, message)                               UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NOT_NEG_INF(actual, line, message)                           UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NOT_NAN(actual, line, message)                               UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE(actual, line, message)                       UNITY_TEST_FAIL((UNITY_LINE_TYPE)(line), UnityStrErrDouble)
#else
#define UNITY_TEST_ASSERT_DOUBLE_WITHIN(delta, expected, actual, line, message)                  UnityAssertDoublesWithin((UNITY_DOUBLE)(delta), (UNITY_DOUBLE)(expected), (UNITY_DOUBLE)(actual), (message), (UNITY_LINE_TYPE)(line))
#define UNITY_TEST_ASSERT_EQUAL_DOUBLE(expected, actual, line, message)                          UNITY_TEST_ASSERT_DOUBLE_WITHIN((UNITY_DOUBLE)(expected) * (UNITY_DOUBLE)UNITY_DOUBLE_PRECISION, (UNITY_DOUBLE)(expected), (UNITY_DOUBLE)(actual), (UNITY_LINE_TYPE)(line), (message))
#define UNITY_TEST_ASSERT_EQUAL_DOUBLE_ARRAY(expected, actual, num_elements, line, message)      UnityAssertEqualDoubleArray((UNITY_DOUBLE*)(expected), (UNITY_DOUBLE*)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_ARRAY_TO_ARRAY)
#define UNITY_TEST_ASSERT_EACH_EQUAL_DOUBLE(expected, actual, num_elements, line, message)       UnityAssertEqualDoubleArray(UnityDoubleToPtr(expected), (UNITY_DOUBLE*)(actual), (UNITY_UINT32)(num_elements), (message), (UNITY_LINE_TYPE)(line), UNITY_ARRAY_TO_VAL)
#define UNITY_TEST_ASSERT_DOUBLE_IS_INF(actual, line, message)                                   UnityAssertDoubleSpecial((UNITY_DOUBLE)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_INF)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NEG_INF(actual, line, message)                               UnityAssertDoubleSpecial((UNITY_DOUBLE)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NEG_INF)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NAN(actual, line, message)                                   UnityAssertDoubleSpecial((UNITY_DOUBLE)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NAN)
#define UNITY_TEST_ASSERT_DOUBLE_IS_DETERMINATE(actual, line, message)                           UnityAssertDoubleSpecial((UNITY_DOUBLE)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_DET)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NOT_INF(actual, line, message)                               UnityAssertDoubleSpecial((UNITY_DOUBLE)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NOT_INF)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NOT_NEG_INF(actual, line, message)                           UnityAssertDoubleSpecial((UNITY_DOUBLE)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NOT_NEG_INF)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NOT_NAN(actual, line, message)                               UnityAssertDoubleSpecial((UNITY_DOUBLE)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NOT_NAN)
#define UNITY_TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE(actual, line, message)                       UnityAssertDoubleSpecial((UNITY_DOUBLE)(actual), (message), (UNITY_LINE_TYPE)(line), UNITY_FLOAT_IS_NOT_DET)
#endif

/* End of UNITY_INTERNALS_H */
#endif


/*-------------------------------------------------------
 * Test Setup / Teardown
 *-------------------------------------------------------*/

/* These functions are intended to be called before and after each test.
 * If using unity directly, these will need to be provided for each test
 * executable built. If you are using the test runner generator and/or
 * Ceedling, these are optional. */
void setUp(void);
void tearDown(void);

/* These functions are intended to be called at the beginning and end of an
 * entire test suite.  suiteTearDown() is passed the number of tests that
 * failed, and its return value becomes the exit code of main(). If using
 * Unity directly, you're in charge of calling these if they are desired.
 * If using Ceedling or the test runner generator, these will be called
 * automatically if they exist. */
void suiteSetUp(void);
int suiteTearDown(int num_failures);

/*-------------------------------------------------------
 * Test Reset and Verify
 *-------------------------------------------------------*/

/* These functions are intended to be called before during tests in order
 * to support complex test loops, etc. Both are NOT built into Unity. Instead
 * the test runner generator will create them. resetTest will run teardown and
 * setup again, verifying any end-of-test needs between. verifyTest will only
 * run the verification. */
void resetTest(void);
void verifyTest(void);

/*-------------------------------------------------------
 * Configuration Options
 *-------------------------------------------------------
 * All options described below should be passed as a compiler flag to all files using Unity. If you must add #defines, place them BEFORE the #include above.

 * Integers/longs/pointers
 *     - Unity attempts to automatically discover your integer sizes
 *       - define UNITY_EXCLUDE_STDINT_H to stop attempting to look in <stdint.h>
 *       - define UNITY_EXCLUDE_LIMITS_H to stop attempting to look in <limits.h>
 *     - If you cannot use the automatic methods above, you can force Unity by using these options:
 *       - define UNITY_SUPPORT_64
 *       - set UNITY_INT_WIDTH
 *       - set UNITY_LONG_WIDTH
 *       - set UNITY_POINTER_WIDTH

 * Floats
 *     - define UNITY_EXCLUDE_FLOAT to disallow floating point comparisons
 *     - define UNITY_FLOAT_PRECISION to specify the precision to use when doing TEST_ASSERT_EQUAL_FLOAT
 *     - define UNITY_FLOAT_TYPE to specify doubles instead of single precision floats
 *     - define UNITY_INCLUDE_DOUBLE to allow double floating point comparisons
 *     - define UNITY_EXCLUDE_DOUBLE to disallow double floating point comparisons (default)
 *     - define UNITY_DOUBLE_PRECISION to specify the precision to use when doing TEST_ASSERT_EQUAL_DOUBLE
 *     - define UNITY_DOUBLE_TYPE to specify something other than double
 *     - define UNITY_EXCLUDE_FLOAT_PRINT to trim binary size, won't print floating point values in errors

 * Output
 *     - by default, Unity prints to standard out with putchar.  define UNITY_OUTPUT_CHAR(a) with a different function if desired
 *     - define UNITY_DIFFERENTIATE_FINAL_FAIL to print FAILED (vs. FAIL) at test end summary - for automated search for failure

 * Optimization
 *     - by default, line numbers are stored in unsigned shorts.  Define UNITY_LINE_TYPE with a different type if your files are huge
 *     - by default, test and failure counters are unsigned shorts.  Define UNITY_COUNTER_TYPE with a different type if you want to save space or have more than 65535 Tests.

 * Test Cases
 *     - define UNITY_SUPPORT_TEST_CASES to include the TEST_CASE macro, though really it's mostly about the runner generator script

 * Parameterized Tests
 *     - you'll want to create a define of TEST_CASE(...) which basically evaluates to nothing

 * Tests with Arguments
 *     - you'll want to define UNITY_USE_COMMAND_LINE_ARGS if you have the test runner passing arguments to Unity

 *-------------------------------------------------------
 * Basic Fail and Ignore
 *-------------------------------------------------------*/

#define TEST_FAIL_MESSAGE(message)                                                                 UNITY_TEST_FAIL(__LINE__, (message))
#define TEST_FAIL()                                                                                UNITY_TEST_FAIL(__LINE__, NULL)
#define TEST_IGNORE_MESSAGE(message)                                                               UNITY_TEST_IGNORE(__LINE__, (message))
#define TEST_IGNORE()                                                                              UNITY_TEST_IGNORE(__LINE__, NULL)
#define TEST_MESSAGE(message)                                                                      UnityMessage((message), __LINE__)
#define TEST_ONLY()
#ifdef UNITY_INCLUDE_PRINT_FORMATTED
#define TEST_PRINTF(message, ...)                                                                  UnityPrintF(__LINE__, (message), __VA_ARGS__)
#endif

/* It is not necessary for you to call PASS. A PASS condition is assumed if nothing fails.
 * This method allows you to abort a test immediately with a PASS state, ignoring the remainder of the test. */
#define TEST_PASS()                                                                                TEST_ABORT()
#define TEST_PASS_MESSAGE(message)                                                                 do { UnityMessage((message), __LINE__); TEST_ABORT(); } while(0)

/* This macro does nothing, but it is useful for build tools (like Ceedling) to make use of this to figure out
 * which files should be linked to in order to perform a test. Use it like TEST_FILE("sandwiches.c") */
#define TEST_FILE(a)

/*-------------------------------------------------------
 * Test Asserts (simple)
 *-------------------------------------------------------*/

/* Boolean */
#define TEST_ASSERT(condition)                                                                     UNITY_TEST_ASSERT(       (condition), __LINE__, " Expression Evaluated To FALSE")
#define TEST_ASSERT_TRUE(condition)                                                                UNITY_TEST_ASSERT(       (condition), __LINE__, " Expected TRUE Was FALSE")
#define TEST_ASSERT_UNLESS(condition)                                                              UNITY_TEST_ASSERT(      !(condition), __LINE__, " Expression Evaluated To TRUE")
#define TEST_ASSERT_FALSE(condition)                                                               UNITY_TEST_ASSERT(      !(condition), __LINE__, " Expected FALSE Was TRUE")
#define TEST_ASSERT_NULL(pointer)                                                                  UNITY_TEST_ASSERT_NULL(    (pointer), __LINE__, " Expected NULL")
#define TEST_ASSERT_NOT_NULL(pointer)                                                              UNITY_TEST_ASSERT_NOT_NULL((pointer), __LINE__, " Expected Non-NULL")
#define TEST_ASSERT_EMPTY(pointer)                                                                 UNITY_TEST_ASSERT_EMPTY(    (pointer), __LINE__, " Expected Empty")
#define TEST_ASSERT_NOT_EMPTY(pointer)                                                             UNITY_TEST_ASSERT_NOT_EMPTY((pointer), __LINE__, " Expected Non-Empty")

/* Integers (of all sizes) */
#define TEST_ASSERT_EQUAL_INT(expected, actual)                                                    UNITY_TEST_ASSERT_EQUAL_INT((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT8(expected, actual)                                                   UNITY_TEST_ASSERT_EQUAL_INT8((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT16(expected, actual)                                                  UNITY_TEST_ASSERT_EQUAL_INT16((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT32(expected, actual)                                                  UNITY_TEST_ASSERT_EQUAL_INT32((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT64(expected, actual)                                                  UNITY_TEST_ASSERT_EQUAL_INT64((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT(expected, actual)                                                   UNITY_TEST_ASSERT_EQUAL_UINT( (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT8(expected, actual)                                                  UNITY_TEST_ASSERT_EQUAL_UINT8( (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT16(expected, actual)                                                 UNITY_TEST_ASSERT_EQUAL_UINT16( (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT32(expected, actual)                                                 UNITY_TEST_ASSERT_EQUAL_UINT32( (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT64(expected, actual)                                                 UNITY_TEST_ASSERT_EQUAL_UINT64( (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_size_t(expected, actual)                                                 UNITY_TEST_ASSERT_EQUAL_UINT((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX(expected, actual)                                                    UNITY_TEST_ASSERT_EQUAL_HEX32((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX8(expected, actual)                                                   UNITY_TEST_ASSERT_EQUAL_HEX8( (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX16(expected, actual)                                                  UNITY_TEST_ASSERT_EQUAL_HEX16((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX32(expected, actual)                                                  UNITY_TEST_ASSERT_EQUAL_HEX32((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX64(expected, actual)                                                  UNITY_TEST_ASSERT_EQUAL_HEX64((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_CHAR(expected, actual)                                                   UNITY_TEST_ASSERT_EQUAL_CHAR((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_BITS(mask, expected, actual)                                                   UNITY_TEST_ASSERT_BITS((mask), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_BITS_HIGH(mask, actual)                                                        UNITY_TEST_ASSERT_BITS((mask), (UNITY_UINT)(-1), (actual), __LINE__, NULL)
#define TEST_ASSERT_BITS_LOW(mask, actual)                                                         UNITY_TEST_ASSERT_BITS((mask), (UNITY_UINT)(0), (actual), __LINE__, NULL)
#define TEST_ASSERT_BIT_HIGH(bit, actual)                                                          UNITY_TEST_ASSERT_BITS(((UNITY_UINT)1 << (bit)), (UNITY_UINT)(-1), (actual), __LINE__, NULL)
#define TEST_ASSERT_BIT_LOW(bit, actual)                                                           UNITY_TEST_ASSERT_BITS(((UNITY_UINT)1 << (bit)), (UNITY_UINT)(0), (actual), __LINE__, NULL)

/* Integer Not Equal To (of all sizes) */
#define TEST_ASSERT_NOT_EQUAL_INT(threshold, actual)                                               UNITY_TEST_ASSERT_NOT_EQUAL_INT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_INT8(threshold, actual)                                              UNITY_TEST_ASSERT_NOT_EQUAL_INT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_INT16(threshold, actual)                                             UNITY_TEST_ASSERT_NOT_EQUAL_INT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_INT32(threshold, actual)                                             UNITY_TEST_ASSERT_NOT_EQUAL_INT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_INT64(threshold, actual)                                             UNITY_TEST_ASSERT_NOT_EQUAL_INT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_UINT(threshold, actual)                                              UNITY_TEST_ASSERT_NOT_EQUAL_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_UINT8(threshold, actual)                                             UNITY_TEST_ASSERT_NOT_EQUAL_UINT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_UINT16(threshold, actual)                                            UNITY_TEST_ASSERT_NOT_EQUAL_UINT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_UINT32(threshold, actual)                                            UNITY_TEST_ASSERT_NOT_EQUAL_UINT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_UINT64(threshold, actual)                                            UNITY_TEST_ASSERT_NOT_EQUAL_UINT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_size_t(threshold, actual)                                            UNITY_TEST_ASSERT_NOT_EQUAL_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_HEX8(threshold, actual)                                              UNITY_TEST_ASSERT_NOT_EQUAL_HEX8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_HEX16(threshold, actual)                                             UNITY_TEST_ASSERT_NOT_EQUAL_HEX16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_HEX32(threshold, actual)                                             UNITY_TEST_ASSERT_NOT_EQUAL_HEX32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_HEX64(threshold, actual)                                             UNITY_TEST_ASSERT_NOT_EQUAL_HEX64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL_CHAR(threshold, actual)                                              UNITY_TEST_ASSERT_NOT_EQUAL_CHAR((threshold), (actual), __LINE__, NULL)

/* Integer Greater Than/ Less Than (of all sizes) */
#define TEST_ASSERT_GREATER_THAN(threshold, actual)                                                UNITY_TEST_ASSERT_GREATER_THAN_INT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_INT(threshold, actual)                                            UNITY_TEST_ASSERT_GREATER_THAN_INT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_INT8(threshold, actual)                                           UNITY_TEST_ASSERT_GREATER_THAN_INT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_INT16(threshold, actual)                                          UNITY_TEST_ASSERT_GREATER_THAN_INT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_INT32(threshold, actual)                                          UNITY_TEST_ASSERT_GREATER_THAN_INT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_INT64(threshold, actual)                                          UNITY_TEST_ASSERT_GREATER_THAN_INT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_UINT(threshold, actual)                                           UNITY_TEST_ASSERT_GREATER_THAN_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_UINT8(threshold, actual)                                          UNITY_TEST_ASSERT_GREATER_THAN_UINT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_UINT16(threshold, actual)                                         UNITY_TEST_ASSERT_GREATER_THAN_UINT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_UINT32(threshold, actual)                                         UNITY_TEST_ASSERT_GREATER_THAN_UINT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_UINT64(threshold, actual)                                         UNITY_TEST_ASSERT_GREATER_THAN_UINT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_size_t(threshold, actual)                                         UNITY_TEST_ASSERT_GREATER_THAN_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_HEX8(threshold, actual)                                           UNITY_TEST_ASSERT_GREATER_THAN_HEX8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_HEX16(threshold, actual)                                          UNITY_TEST_ASSERT_GREATER_THAN_HEX16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_HEX32(threshold, actual)                                          UNITY_TEST_ASSERT_GREATER_THAN_HEX32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_HEX64(threshold, actual)                                          UNITY_TEST_ASSERT_GREATER_THAN_HEX64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_THAN_CHAR(threshold, actual)                                           UNITY_TEST_ASSERT_GREATER_THAN_CHAR((threshold), (actual), __LINE__, NULL)

#define TEST_ASSERT_LESS_THAN(threshold, actual)                                                   UNITY_TEST_ASSERT_SMALLER_THAN_INT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_INT(threshold, actual)                                               UNITY_TEST_ASSERT_SMALLER_THAN_INT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_INT8(threshold, actual)                                              UNITY_TEST_ASSERT_SMALLER_THAN_INT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_INT16(threshold, actual)                                             UNITY_TEST_ASSERT_SMALLER_THAN_INT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_INT32(threshold, actual)                                             UNITY_TEST_ASSERT_SMALLER_THAN_INT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_INT64(threshold, actual)                                             UNITY_TEST_ASSERT_SMALLER_THAN_INT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_UINT(threshold, actual)                                              UNITY_TEST_ASSERT_SMALLER_THAN_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_UINT8(threshold, actual)                                             UNITY_TEST_ASSERT_SMALLER_THAN_UINT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_UINT16(threshold, actual)                                            UNITY_TEST_ASSERT_SMALLER_THAN_UINT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_UINT32(threshold, actual)                                            UNITY_TEST_ASSERT_SMALLER_THAN_UINT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_UINT64(threshold, actual)                                            UNITY_TEST_ASSERT_SMALLER_THAN_UINT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_size_t(threshold, actual)                                            UNITY_TEST_ASSERT_SMALLER_THAN_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_HEX8(threshold, actual)                                              UNITY_TEST_ASSERT_SMALLER_THAN_HEX8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_HEX16(threshold, actual)                                             UNITY_TEST_ASSERT_SMALLER_THAN_HEX16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_HEX32(threshold, actual)                                             UNITY_TEST_ASSERT_SMALLER_THAN_HEX32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_HEX64(threshold, actual)                                             UNITY_TEST_ASSERT_SMALLER_THAN_HEX64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_THAN_CHAR(threshold, actual)                                              UNITY_TEST_ASSERT_SMALLER_THAN_CHAR((threshold), (actual), __LINE__, NULL)

#define TEST_ASSERT_GREATER_OR_EQUAL(threshold, actual)                                            UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_INT(threshold, actual)                                        UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_INT8(threshold, actual)                                       UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_INT16(threshold, actual)                                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_INT32(threshold, actual)                                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_INT64(threshold, actual)                                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT(threshold, actual)                                       UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT8(threshold, actual)                                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT16(threshold, actual)                                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT32(threshold, actual)                                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT64(threshold, actual)                                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_size_t(threshold, actual)                                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_HEX8(threshold, actual)                                       UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_HEX16(threshold, actual)                                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_HEX32(threshold, actual)                                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_HEX64(threshold, actual)                                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_GREATER_OR_EQUAL_CHAR(threshold, actual)                                       UNITY_TEST_ASSERT_GREATER_OR_EQUAL_CHAR((threshold), (actual), __LINE__, NULL)

#define TEST_ASSERT_LESS_OR_EQUAL(threshold, actual)                                               UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_INT(threshold, actual)                                           UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_INT8(threshold, actual)                                          UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_INT16(threshold, actual)                                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_INT32(threshold, actual)                                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_INT64(threshold, actual)                                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_UINT(threshold, actual)                                          UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_UINT8(threshold, actual)                                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_UINT16(threshold, actual)                                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_UINT32(threshold, actual)                                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_UINT64(threshold, actual)                                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_size_t(threshold, actual)                                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_HEX8(threshold, actual)                                          UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX8((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_HEX16(threshold, actual)                                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX16((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_HEX32(threshold, actual)                                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX32((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_HEX64(threshold, actual)                                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX64((threshold), (actual), __LINE__, NULL)
#define TEST_ASSERT_LESS_OR_EQUAL_CHAR(threshold, actual)                                          UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_CHAR((threshold), (actual), __LINE__, NULL)

/* Integer Ranges (of all sizes) */
#define TEST_ASSERT_INT_WITHIN(delta, expected, actual)                                            UNITY_TEST_ASSERT_INT_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_INT8_WITHIN(delta, expected, actual)                                           UNITY_TEST_ASSERT_INT8_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_INT16_WITHIN(delta, expected, actual)                                          UNITY_TEST_ASSERT_INT16_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_INT32_WITHIN(delta, expected, actual)                                          UNITY_TEST_ASSERT_INT32_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_INT64_WITHIN(delta, expected, actual)                                          UNITY_TEST_ASSERT_INT64_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_UINT_WITHIN(delta, expected, actual)                                           UNITY_TEST_ASSERT_UINT_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_UINT8_WITHIN(delta, expected, actual)                                          UNITY_TEST_ASSERT_UINT8_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_UINT16_WITHIN(delta, expected, actual)                                         UNITY_TEST_ASSERT_UINT16_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_UINT32_WITHIN(delta, expected, actual)                                         UNITY_TEST_ASSERT_UINT32_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_UINT64_WITHIN(delta, expected, actual)                                         UNITY_TEST_ASSERT_UINT64_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_size_t_WITHIN(delta, expected, actual)                                         UNITY_TEST_ASSERT_UINT_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_HEX_WITHIN(delta, expected, actual)                                            UNITY_TEST_ASSERT_HEX32_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_HEX8_WITHIN(delta, expected, actual)                                           UNITY_TEST_ASSERT_HEX8_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_HEX16_WITHIN(delta, expected, actual)                                          UNITY_TEST_ASSERT_HEX16_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_HEX32_WITHIN(delta, expected, actual)                                          UNITY_TEST_ASSERT_HEX32_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_HEX64_WITHIN(delta, expected, actual)                                          UNITY_TEST_ASSERT_HEX64_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_CHAR_WITHIN(delta, expected, actual)                                           UNITY_TEST_ASSERT_CHAR_WITHIN((delta), (expected), (actual), __LINE__, NULL)

/* Integer Array Ranges (of all sizes) */
#define TEST_ASSERT_INT_ARRAY_WITHIN(delta, expected, actual, num_elements)                        UNITY_TEST_ASSERT_INT_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_INT8_ARRAY_WITHIN(delta, expected, actual, num_elements)                       UNITY_TEST_ASSERT_INT8_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_INT16_ARRAY_WITHIN(delta, expected, actual, num_elements)                      UNITY_TEST_ASSERT_INT16_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_INT32_ARRAY_WITHIN(delta, expected, actual, num_elements)                      UNITY_TEST_ASSERT_INT32_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_INT64_ARRAY_WITHIN(delta, expected, actual, num_elements)                      UNITY_TEST_ASSERT_INT64_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_UINT_ARRAY_WITHIN(delta, expected, actual, num_elements)                       UNITY_TEST_ASSERT_UINT_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_UINT8_ARRAY_WITHIN(delta, expected, actual, num_elements)                      UNITY_TEST_ASSERT_UINT8_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_UINT16_ARRAY_WITHIN(delta, expected, actual, num_elements)                     UNITY_TEST_ASSERT_UINT16_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_UINT32_ARRAY_WITHIN(delta, expected, actual, num_elements)                     UNITY_TEST_ASSERT_UINT32_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_UINT64_ARRAY_WITHIN(delta, expected, actual, num_elements)                     UNITY_TEST_ASSERT_UINT64_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_size_t_ARRAY_WITHIN(delta, expected, actual, num_elements)                     UNITY_TEST_ASSERT_UINT_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_HEX_ARRAY_WITHIN(delta, expected, actual, num_elements)                        UNITY_TEST_ASSERT_HEX32_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_HEX8_ARRAY_WITHIN(delta, expected, actual, num_elements)                       UNITY_TEST_ASSERT_HEX8_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_HEX16_ARRAY_WITHIN(delta, expected, actual, num_elements)                      UNITY_TEST_ASSERT_HEX16_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_HEX32_ARRAY_WITHIN(delta, expected, actual, num_elements)                      UNITY_TEST_ASSERT_HEX32_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_HEX64_ARRAY_WITHIN(delta, expected, actual, num_elements)                      UNITY_TEST_ASSERT_HEX64_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)
#define TEST_ASSERT_CHAR_ARRAY_WITHIN(delta, expected, actual, num_elements)                       UNITY_TEST_ASSERT_CHAR_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, NULL)


/* Structs and Strings */
#define TEST_ASSERT_EQUAL_PTR(expected, actual)                                                    UNITY_TEST_ASSERT_EQUAL_PTR((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_STRING(expected, actual)                                                 UNITY_TEST_ASSERT_EQUAL_STRING((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_STRING_LEN(expected, actual, len)                                        UNITY_TEST_ASSERT_EQUAL_STRING_LEN((expected), (actual), (len), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_MEMORY(expected, actual, len)                                            UNITY_TEST_ASSERT_EQUAL_MEMORY((expected), (actual), (len), __LINE__, NULL)

/* Arrays */
#define TEST_ASSERT_EQUAL_INT_ARRAY(expected, actual, num_elements)                                UNITY_TEST_ASSERT_EQUAL_INT_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EQUAL_INT8_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT16_ARRAY(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EQUAL_INT16_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT32_ARRAY(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EQUAL_INT32_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_INT64_ARRAY(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EQUAL_INT64_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT_ARRAY(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EQUAL_UINT_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT16_ARRAY(expected, actual, num_elements)                             UNITY_TEST_ASSERT_EQUAL_UINT16_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, num_elements)                             UNITY_TEST_ASSERT_EQUAL_UINT32_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_UINT64_ARRAY(expected, actual, num_elements)                             UNITY_TEST_ASSERT_EQUAL_UINT64_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_size_t_ARRAY(expected, actual, num_elements)                             UNITY_TEST_ASSERT_EQUAL_UINT_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX_ARRAY(expected, actual, num_elements)                                UNITY_TEST_ASSERT_EQUAL_HEX32_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EQUAL_HEX8_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX16_ARRAY(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EQUAL_HEX16_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EQUAL_HEX32_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_HEX64_ARRAY(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EQUAL_HEX64_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_PTR_ARRAY(expected, actual, num_elements)                                UNITY_TEST_ASSERT_EQUAL_PTR_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_STRING_ARRAY(expected, actual, num_elements)                             UNITY_TEST_ASSERT_EQUAL_STRING_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_MEMORY_ARRAY(expected, actual, len, num_elements)                        UNITY_TEST_ASSERT_EQUAL_MEMORY_ARRAY((expected), (actual), (len), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EQUAL_CHAR_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)

/* Arrays Compared To Single Value */
#define TEST_ASSERT_EACH_EQUAL_INT(expected, actual, num_elements)                                 UNITY_TEST_ASSERT_EACH_EQUAL_INT((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_INT8(expected, actual, num_elements)                                UNITY_TEST_ASSERT_EACH_EQUAL_INT8((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_INT16(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EACH_EQUAL_INT16((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_INT32(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EACH_EQUAL_INT32((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_INT64(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EACH_EQUAL_INT64((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_UINT(expected, actual, num_elements)                                UNITY_TEST_ASSERT_EACH_EQUAL_UINT((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_UINT8(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EACH_EQUAL_UINT8((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_UINT16(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EACH_EQUAL_UINT16((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_UINT32(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EACH_EQUAL_UINT32((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_UINT64(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EACH_EQUAL_UINT64((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_size_t(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EACH_EQUAL_UINT((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_HEX(expected, actual, num_elements)                                 UNITY_TEST_ASSERT_EACH_EQUAL_HEX32((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_HEX8(expected, actual, num_elements)                                UNITY_TEST_ASSERT_EACH_EQUAL_HEX8((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_HEX16(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EACH_EQUAL_HEX16((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_HEX32(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EACH_EQUAL_HEX32((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_HEX64(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EACH_EQUAL_HEX64((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_PTR(expected, actual, num_elements)                                 UNITY_TEST_ASSERT_EACH_EQUAL_PTR((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_STRING(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EACH_EQUAL_STRING((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_MEMORY(expected, actual, len, num_elements)                         UNITY_TEST_ASSERT_EACH_EQUAL_MEMORY((expected), (actual), (len), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_CHAR(expected, actual, num_elements)                                UNITY_TEST_ASSERT_EACH_EQUAL_CHAR((expected), (actual), (num_elements), __LINE__, NULL)

/* Floating Point (If Enabled) */
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)                                          UNITY_TEST_ASSERT_FLOAT_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_FLOAT(expected, actual)                                                  UNITY_TEST_ASSERT_EQUAL_FLOAT((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_FLOAT_ARRAY(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EQUAL_FLOAT_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_FLOAT(expected, actual, num_elements)                               UNITY_TEST_ASSERT_EACH_EQUAL_FLOAT((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_FLOAT_IS_INF(actual)                                                           UNITY_TEST_ASSERT_FLOAT_IS_INF((actual), __LINE__, NULL)
#define TEST_ASSERT_FLOAT_IS_NEG_INF(actual)                                                       UNITY_TEST_ASSERT_FLOAT_IS_NEG_INF((actual), __LINE__, NULL)
#define TEST_ASSERT_FLOAT_IS_NAN(actual)                                                           UNITY_TEST_ASSERT_FLOAT_IS_NAN((actual), __LINE__, NULL)
#define TEST_ASSERT_FLOAT_IS_DETERMINATE(actual)                                                   UNITY_TEST_ASSERT_FLOAT_IS_DETERMINATE((actual), __LINE__, NULL)
#define TEST_ASSERT_FLOAT_IS_NOT_INF(actual)                                                       UNITY_TEST_ASSERT_FLOAT_IS_NOT_INF((actual), __LINE__, NULL)
#define TEST_ASSERT_FLOAT_IS_NOT_NEG_INF(actual)                                                   UNITY_TEST_ASSERT_FLOAT_IS_NOT_NEG_INF((actual), __LINE__, NULL)
#define TEST_ASSERT_FLOAT_IS_NOT_NAN(actual)                                                       UNITY_TEST_ASSERT_FLOAT_IS_NOT_NAN((actual), __LINE__, NULL)
#define TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE(actual)                                               UNITY_TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE((actual), __LINE__, NULL)

/* Double (If Enabled) */
#define TEST_ASSERT_DOUBLE_WITHIN(delta, expected, actual)                                         UNITY_TEST_ASSERT_DOUBLE_WITHIN((delta), (expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_DOUBLE(expected, actual)                                                 UNITY_TEST_ASSERT_EQUAL_DOUBLE((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_EQUAL_DOUBLE_ARRAY(expected, actual, num_elements)                             UNITY_TEST_ASSERT_EQUAL_DOUBLE_ARRAY((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_EACH_EQUAL_DOUBLE(expected, actual, num_elements)                              UNITY_TEST_ASSERT_EACH_EQUAL_DOUBLE((expected), (actual), (num_elements), __LINE__, NULL)
#define TEST_ASSERT_DOUBLE_IS_INF(actual)                                                          UNITY_TEST_ASSERT_DOUBLE_IS_INF((actual), __LINE__, NULL)
#define TEST_ASSERT_DOUBLE_IS_NEG_INF(actual)                                                      UNITY_TEST_ASSERT_DOUBLE_IS_NEG_INF((actual), __LINE__, NULL)
#define TEST_ASSERT_DOUBLE_IS_NAN(actual)                                                          UNITY_TEST_ASSERT_DOUBLE_IS_NAN((actual), __LINE__, NULL)
#define TEST_ASSERT_DOUBLE_IS_DETERMINATE(actual)                                                  UNITY_TEST_ASSERT_DOUBLE_IS_DETERMINATE((actual), __LINE__, NULL)
#define TEST_ASSERT_DOUBLE_IS_NOT_INF(actual)                                                      UNITY_TEST_ASSERT_DOUBLE_IS_NOT_INF((actual), __LINE__, NULL)
#define TEST_ASSERT_DOUBLE_IS_NOT_NEG_INF(actual)                                                  UNITY_TEST_ASSERT_DOUBLE_IS_NOT_NEG_INF((actual), __LINE__, NULL)
#define TEST_ASSERT_DOUBLE_IS_NOT_NAN(actual)                                                      UNITY_TEST_ASSERT_DOUBLE_IS_NOT_NAN((actual), __LINE__, NULL)
#define TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE(actual)                                              UNITY_TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE((actual), __LINE__, NULL)

/* Shorthand */
#ifdef UNITY_SHORTHAND_AS_OLD
#define TEST_ASSERT_EQUAL(expected, actual)                                                        UNITY_TEST_ASSERT_EQUAL_INT((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL(expected, actual)                                                    UNITY_TEST_ASSERT(((expected) != (actual)), __LINE__, " Expected Not-Equal")
#endif
#ifdef UNITY_SHORTHAND_AS_INT
#define TEST_ASSERT_EQUAL(expected, actual)                                                        UNITY_TEST_ASSERT_EQUAL_INT((expected), (actual), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL(expected, actual)                                                    UNITY_TEST_FAIL(__LINE__, UnityStrErrShorthand)
#endif
#ifdef UNITY_SHORTHAND_AS_MEM
#define TEST_ASSERT_EQUAL(expected, actual)                                                        UNITY_TEST_ASSERT_EQUAL_MEMORY((&expected), (&actual), sizeof(expected), __LINE__, NULL)
#define TEST_ASSERT_NOT_EQUAL(expected, actual)                                                    UNITY_TEST_FAIL(__LINE__, UnityStrErrShorthand)
#endif
#ifdef UNITY_SHORTHAND_AS_RAW
#define TEST_ASSERT_EQUAL(expected, actual)                                                        UNITY_TEST_ASSERT(((expected) == (actual)), __LINE__, " Expected Equal")
#define TEST_ASSERT_NOT_EQUAL(expected, actual)                                                    UNITY_TEST_ASSERT(((expected) != (actual)), __LINE__, " Expected Not-Equal")
#endif
#ifdef UNITY_SHORTHAND_AS_NONE
#define TEST_ASSERT_EQUAL(expected, actual)                                                        UNITY_TEST_FAIL(__LINE__, UnityStrErrShorthand)
#define TEST_ASSERT_NOT_EQUAL(expected, actual)                                                    UNITY_TEST_FAIL(__LINE__, UnityStrErrShorthand)
#endif

/*-------------------------------------------------------
 * Test Asserts (with additional messages)
 *-------------------------------------------------------*/

/* Boolean */
#define TEST_ASSERT_MESSAGE(condition, message)                                                    UNITY_TEST_ASSERT(       (condition), __LINE__, (message))
#define TEST_ASSERT_TRUE_MESSAGE(condition, message)                                               UNITY_TEST_ASSERT(       (condition), __LINE__, (message))
#define TEST_ASSERT_UNLESS_MESSAGE(condition, message)                                             UNITY_TEST_ASSERT(      !(condition), __LINE__, (message))
#define TEST_ASSERT_FALSE_MESSAGE(condition, message)                                              UNITY_TEST_ASSERT(      !(condition), __LINE__, (message))
#define TEST_ASSERT_NULL_MESSAGE(pointer, message)                                                 UNITY_TEST_ASSERT_NULL(    (pointer), __LINE__, (message))
#define TEST_ASSERT_NOT_NULL_MESSAGE(pointer, message)                                             UNITY_TEST_ASSERT_NOT_NULL((pointer), __LINE__, (message))
#define TEST_ASSERT_EMPTY_MESSAGE(pointer, message)                                                UNITY_TEST_ASSERT_EMPTY(    (pointer), __LINE__, (message))
#define TEST_ASSERT_NOT_EMPTY_MESSAGE(pointer, message)                                            UNITY_TEST_ASSERT_NOT_EMPTY((pointer), __LINE__, (message))

/* Integers (of all sizes) */
#define TEST_ASSERT_EQUAL_INT_MESSAGE(expected, actual, message)                                   UNITY_TEST_ASSERT_EQUAL_INT((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_INT8_MESSAGE(expected, actual, message)                                  UNITY_TEST_ASSERT_EQUAL_INT8((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_INT16_MESSAGE(expected, actual, message)                                 UNITY_TEST_ASSERT_EQUAL_INT16((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_INT32_MESSAGE(expected, actual, message)                                 UNITY_TEST_ASSERT_EQUAL_INT32((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_INT64_MESSAGE(expected, actual, message)                                 UNITY_TEST_ASSERT_EQUAL_INT64((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT_MESSAGE(expected, actual, message)                                  UNITY_TEST_ASSERT_EQUAL_UINT( (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected, actual, message)                                 UNITY_TEST_ASSERT_EQUAL_UINT8( (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected, actual, message)                                UNITY_TEST_ASSERT_EQUAL_UINT16( (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, actual, message)                                UNITY_TEST_ASSERT_EQUAL_UINT32( (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT64_MESSAGE(expected, actual, message)                                UNITY_TEST_ASSERT_EQUAL_UINT64( (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_size_t_MESSAGE(expected, actual, message)                                UNITY_TEST_ASSERT_EQUAL_UINT( (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX_MESSAGE(expected, actual, message)                                   UNITY_TEST_ASSERT_EQUAL_HEX32((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected, actual, message)                                  UNITY_TEST_ASSERT_EQUAL_HEX8( (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX16_MESSAGE(expected, actual, message)                                 UNITY_TEST_ASSERT_EQUAL_HEX16((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX32_MESSAGE(expected, actual, message)                                 UNITY_TEST_ASSERT_EQUAL_HEX32((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX64_MESSAGE(expected, actual, message)                                 UNITY_TEST_ASSERT_EQUAL_HEX64((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_BITS_MESSAGE(mask, expected, actual, message)                                  UNITY_TEST_ASSERT_BITS((mask), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_BITS_HIGH_MESSAGE(mask, actual, message)                                       UNITY_TEST_ASSERT_BITS((mask), (UNITY_UINT32)(-1), (actual), __LINE__, (message))
#define TEST_ASSERT_BITS_LOW_MESSAGE(mask, actual, message)                                        UNITY_TEST_ASSERT_BITS((mask), (UNITY_UINT32)(0), (actual), __LINE__, (message))
#define TEST_ASSERT_BIT_HIGH_MESSAGE(bit, actual, message)                                         UNITY_TEST_ASSERT_BITS(((UNITY_UINT32)1 << (bit)), (UNITY_UINT32)(-1), (actual), __LINE__, (message))
#define TEST_ASSERT_BIT_LOW_MESSAGE(bit, actual, message)                                          UNITY_TEST_ASSERT_BITS(((UNITY_UINT32)1 << (bit)), (UNITY_UINT32)(0), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_CHAR_MESSAGE(expected, actual, message)                                  UNITY_TEST_ASSERT_EQUAL_CHAR((expected), (actual), __LINE__, (message))

/* Integer Not Equal To (of all sizes) */
#define TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(threshold, actual, message)                              UNITY_TEST_ASSERT_NOT_EQUAL_INT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_INT8_MESSAGE(threshold, actual, message)                             UNITY_TEST_ASSERT_NOT_EQUAL_INT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_INT16_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_NOT_EQUAL_INT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_INT32_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_NOT_EQUAL_INT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_INT64_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_NOT_EQUAL_INT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_UINT_MESSAGE(threshold, actual, message)                             UNITY_TEST_ASSERT_NOT_EQUAL_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_NOT_EQUAL_UINT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_UINT16_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_NOT_EQUAL_UINT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_UINT32_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_NOT_EQUAL_UINT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_UINT64_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_NOT_EQUAL_UINT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_size_t_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_NOT_EQUAL_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_HEX8_MESSAGE(threshold, actual, message)                             UNITY_TEST_ASSERT_NOT_EQUAL_HEX8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_HEX16_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_NOT_EQUAL_HEX16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_HEX32_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_NOT_EQUAL_HEX32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_HEX64_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_NOT_EQUAL_HEX64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_CHAR_MESSAGE(threshold, actual, message)                             UNITY_TEST_ASSERT_NOT_EQUAL_CHAR((threshold), (actual), __LINE__, (message))


/* Integer Greater Than/ Less Than (of all sizes) */
#define TEST_ASSERT_GREATER_THAN_MESSAGE(threshold, actual, message)                               UNITY_TEST_ASSERT_GREATER_THAN_INT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_INT_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_GREATER_THAN_INT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_INT8_MESSAGE(threshold, actual, message)                          UNITY_TEST_ASSERT_GREATER_THAN_INT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_INT16_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_GREATER_THAN_INT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_INT32_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_GREATER_THAN_INT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_INT64_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_GREATER_THAN_INT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_UINT_MESSAGE(threshold, actual, message)                          UNITY_TEST_ASSERT_GREATER_THAN_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_UINT8_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_GREATER_THAN_UINT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_UINT16_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_GREATER_THAN_UINT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_UINT32_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_GREATER_THAN_UINT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_UINT64_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_GREATER_THAN_UINT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_size_t_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_GREATER_THAN_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_HEX8_MESSAGE(threshold, actual, message)                          UNITY_TEST_ASSERT_GREATER_THAN_HEX8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_HEX16_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_GREATER_THAN_HEX16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_HEX32_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_GREATER_THAN_HEX32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_HEX64_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_GREATER_THAN_HEX64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_THAN_CHAR_MESSAGE(threshold, actual, message)                          UNITY_TEST_ASSERT_GREATER_THAN_CHAR((threshold), (actual), __LINE__, (message))

#define TEST_ASSERT_LESS_THAN_MESSAGE(threshold, actual, message)                                  UNITY_TEST_ASSERT_SMALLER_THAN_INT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_INT_MESSAGE(threshold, actual, message)                              UNITY_TEST_ASSERT_SMALLER_THAN_INT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_INT8_MESSAGE(threshold, actual, message)                             UNITY_TEST_ASSERT_SMALLER_THAN_INT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_INT16_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_SMALLER_THAN_INT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_INT32_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_SMALLER_THAN_INT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_INT64_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_SMALLER_THAN_INT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_UINT_MESSAGE(threshold, actual, message)                             UNITY_TEST_ASSERT_SMALLER_THAN_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_UINT8_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_SMALLER_THAN_UINT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_UINT16_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_SMALLER_THAN_UINT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_UINT32_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_SMALLER_THAN_UINT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_UINT64_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_SMALLER_THAN_UINT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_size_t_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_SMALLER_THAN_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_HEX8_MESSAGE(threshold, actual, message)                             UNITY_TEST_ASSERT_SMALLER_THAN_HEX8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_HEX16_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_SMALLER_THAN_HEX16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_HEX32_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_SMALLER_THAN_HEX32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_HEX64_MESSAGE(threshold, actual, message)                            UNITY_TEST_ASSERT_SMALLER_THAN_HEX64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_THAN_CHAR_MESSAGE(threshold, actual, message)                             UNITY_TEST_ASSERT_SMALLER_THAN_CHAR((threshold), (actual), __LINE__, (message))

#define TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(threshold, actual, message)                           UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(threshold, actual, message)                       UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_INT8_MESSAGE(threshold, actual, message)                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_INT16_MESSAGE(threshold, actual, message)                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_INT32_MESSAGE(threshold, actual, message)                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_INT64_MESSAGE(threshold, actual, message)                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_INT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT_MESSAGE(threshold, actual, message)                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT8_MESSAGE(threshold, actual, message)                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT16_MESSAGE(threshold, actual, message)                    UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT32_MESSAGE(threshold, actual, message)                    UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT64_MESSAGE(threshold, actual, message)                    UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_size_t_MESSAGE(threshold, actual, message)                    UNITY_TEST_ASSERT_GREATER_OR_EQUAL_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_HEX8_MESSAGE(threshold, actual, message)                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_HEX16_MESSAGE(threshold, actual, message)                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_HEX32_MESSAGE(threshold, actual, message)                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_HEX64_MESSAGE(threshold, actual, message)                     UNITY_TEST_ASSERT_GREATER_OR_EQUAL_HEX64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_GREATER_OR_EQUAL_CHAR_MESSAGE(threshold, actual, message)                      UNITY_TEST_ASSERT_GREATER_OR_EQUAL_CHAR((threshold), (actual), __LINE__, (message))

#define TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(threshold, actual, message)                              UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_INT_MESSAGE(threshold, actual, message)                          UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_INT8_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_INT16_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_INT32_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_INT64_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_INT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_UINT_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_UINT8_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_UINT16_MESSAGE(threshold, actual, message)                       UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_UINT32_MESSAGE(threshold, actual, message)                       UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_UINT64_MESSAGE(threshold, actual, message)                       UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_size_t_MESSAGE(threshold, actual, message)                       UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_HEX8_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX8((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_HEX16_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX16((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_HEX32_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX32((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_HEX64_MESSAGE(threshold, actual, message)                        UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_HEX64((threshold), (actual), __LINE__, (message))
#define TEST_ASSERT_LESS_OR_EQUAL_CHAR_MESSAGE(threshold, actual, message)                         UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_CHAR((threshold), (actual), __LINE__, (message))

/* Integer Ranges (of all sizes) */
#define TEST_ASSERT_INT_WITHIN_MESSAGE(delta, expected, actual, message)                           UNITY_TEST_ASSERT_INT_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_INT8_WITHIN_MESSAGE(delta, expected, actual, message)                          UNITY_TEST_ASSERT_INT8_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_INT16_WITHIN_MESSAGE(delta, expected, actual, message)                         UNITY_TEST_ASSERT_INT16_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_INT32_WITHIN_MESSAGE(delta, expected, actual, message)                         UNITY_TEST_ASSERT_INT32_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_INT64_WITHIN_MESSAGE(delta, expected, actual, message)                         UNITY_TEST_ASSERT_INT64_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_UINT_WITHIN_MESSAGE(delta, expected, actual, message)                          UNITY_TEST_ASSERT_UINT_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_UINT8_WITHIN_MESSAGE(delta, expected, actual, message)                         UNITY_TEST_ASSERT_UINT8_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_UINT16_WITHIN_MESSAGE(delta, expected, actual, message)                        UNITY_TEST_ASSERT_UINT16_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_UINT32_WITHIN_MESSAGE(delta, expected, actual, message)                        UNITY_TEST_ASSERT_UINT32_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_UINT64_WITHIN_MESSAGE(delta, expected, actual, message)                        UNITY_TEST_ASSERT_UINT64_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_size_t_WITHIN_MESSAGE(delta, expected, actual, message)                        UNITY_TEST_ASSERT_UINT_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_HEX_WITHIN_MESSAGE(delta, expected, actual, message)                           UNITY_TEST_ASSERT_HEX32_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_HEX8_WITHIN_MESSAGE(delta, expected, actual, message)                          UNITY_TEST_ASSERT_HEX8_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_HEX16_WITHIN_MESSAGE(delta, expected, actual, message)                         UNITY_TEST_ASSERT_HEX16_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_HEX32_WITHIN_MESSAGE(delta, expected, actual, message)                         UNITY_TEST_ASSERT_HEX32_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_HEX64_WITHIN_MESSAGE(delta, expected, actual, message)                         UNITY_TEST_ASSERT_HEX64_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_CHAR_WITHIN_MESSAGE(delta, expected, actual, message)                          UNITY_TEST_ASSERT_CHAR_WITHIN((delta), (expected), (actual), __LINE__, (message))

/* Integer Array Ranges (of all sizes) */
#define TEST_ASSERT_INT_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)       UNITY_TEST_ASSERT_INT_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_INT8_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)      UNITY_TEST_ASSERT_INT8_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_INT16_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)     UNITY_TEST_ASSERT_INT16_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_INT32_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)     UNITY_TEST_ASSERT_INT32_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_INT64_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)     UNITY_TEST_ASSERT_INT64_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_UINT_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)      UNITY_TEST_ASSERT_UINT_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_UINT8_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)     UNITY_TEST_ASSERT_UINT8_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_UINT16_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)    UNITY_TEST_ASSERT_UINT16_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_UINT32_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)    UNITY_TEST_ASSERT_UINT32_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_UINT64_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)    UNITY_TEST_ASSERT_UINT64_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_size_t_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)    UNITY_TEST_ASSERT_UINT_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_HEX_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)       UNITY_TEST_ASSERT_HEX32_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_HEX8_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)      UNITY_TEST_ASSERT_HEX8_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_HEX16_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)     UNITY_TEST_ASSERT_HEX16_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_HEX32_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)     UNITY_TEST_ASSERT_HEX32_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_HEX64_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)     UNITY_TEST_ASSERT_HEX64_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))
#define TEST_ASSERT_CHAR_ARRAY_WITHIN_MESSAGE(delta, expected, actual, num_elements, message)      UNITY_TEST_ASSERT_CHAR_ARRAY_WITHIN((delta), (expected), (actual), num_elements, __LINE__, (message))


/* Structs and Strings */
#define TEST_ASSERT_EQUAL_PTR_MESSAGE(expected, actual, message)                                   UNITY_TEST_ASSERT_EQUAL_PTR((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, actual, message)                                UNITY_TEST_ASSERT_EQUAL_STRING((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(expected, actual, len, message)                       UNITY_TEST_ASSERT_EQUAL_STRING_LEN((expected), (actual), (len), __LINE__, (message))
#define TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected, actual, len, message)                           UNITY_TEST_ASSERT_EQUAL_MEMORY((expected), (actual), (len), __LINE__, (message))

/* Arrays */
#define TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected, actual, num_elements, message)               UNITY_TEST_ASSERT_EQUAL_INT_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EQUAL_INT8_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_INT16_ARRAY_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EQUAL_INT16_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_INT32_ARRAY_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EQUAL_INT32_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_INT64_ARRAY_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EQUAL_INT64_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT_ARRAY_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EQUAL_UINT_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT16_ARRAY_MESSAGE(expected, actual, num_elements, message)            UNITY_TEST_ASSERT_EQUAL_UINT16_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT32_ARRAY_MESSAGE(expected, actual, num_elements, message)            UNITY_TEST_ASSERT_EQUAL_UINT32_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_UINT64_ARRAY_MESSAGE(expected, actual, num_elements, message)            UNITY_TEST_ASSERT_EQUAL_UINT64_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_size_t_ARRAY_MESSAGE(expected, actual, num_elements, message)            UNITY_TEST_ASSERT_EQUAL_UINT_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX_ARRAY_MESSAGE(expected, actual, num_elements, message)               UNITY_TEST_ASSERT_EQUAL_HEX32_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EQUAL_HEX8_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX16_ARRAY_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EQUAL_HEX16_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX32_ARRAY_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EQUAL_HEX32_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_HEX64_ARRAY_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EQUAL_HEX64_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_PTR_ARRAY_MESSAGE(expected, actual, num_elements, message)               UNITY_TEST_ASSERT_EQUAL_PTR_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_STRING_ARRAY_MESSAGE(expected, actual, num_elements, message)            UNITY_TEST_ASSERT_EQUAL_STRING_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_MEMORY_ARRAY_MESSAGE(expected, actual, len, num_elements, message)       UNITY_TEST_ASSERT_EQUAL_MEMORY_ARRAY((expected), (actual), (len), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EQUAL_CHAR_ARRAY((expected), (actual), (num_elements), __LINE__, (message))

/* Arrays Compared To Single Value*/
#define TEST_ASSERT_EACH_EQUAL_INT_MESSAGE(expected, actual, num_elements, message)                UNITY_TEST_ASSERT_EACH_EQUAL_INT((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_INT8_MESSAGE(expected, actual, num_elements, message)               UNITY_TEST_ASSERT_EACH_EQUAL_INT8((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_INT16_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EACH_EQUAL_INT16((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_INT32_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EACH_EQUAL_INT32((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_INT64_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EACH_EQUAL_INT64((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_UINT_MESSAGE(expected, actual, num_elements, message)               UNITY_TEST_ASSERT_EACH_EQUAL_UINT((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_UINT8_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EACH_EQUAL_UINT8((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_UINT16_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EACH_EQUAL_UINT16((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_UINT32_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EACH_EQUAL_UINT32((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_UINT64_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EACH_EQUAL_UINT64((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_size_t_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EACH_EQUAL_UINT((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_HEX_MESSAGE(expected, actual, num_elements, message)                UNITY_TEST_ASSERT_EACH_EQUAL_HEX32((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_HEX8_MESSAGE(expected, actual, num_elements, message)               UNITY_TEST_ASSERT_EACH_EQUAL_HEX8((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_HEX16_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EACH_EQUAL_HEX16((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_HEX32_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EACH_EQUAL_HEX32((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_HEX64_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EACH_EQUAL_HEX64((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_PTR_MESSAGE(expected, actual, num_elements, message)                UNITY_TEST_ASSERT_EACH_EQUAL_PTR((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_STRING_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EACH_EQUAL_STRING((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_MEMORY_MESSAGE(expected, actual, len, num_elements, message)        UNITY_TEST_ASSERT_EACH_EQUAL_MEMORY((expected), (actual), (len), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(expected, actual, num_elements, message)               UNITY_TEST_ASSERT_EACH_EQUAL_CHAR((expected), (actual), (num_elements), __LINE__, (message))

/* Floating Point (If Enabled) */
#define TEST_ASSERT_FLOAT_WITHIN_MESSAGE(delta, expected, actual, message)                         UNITY_TEST_ASSERT_FLOAT_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_FLOAT_MESSAGE(expected, actual, message)                                 UNITY_TEST_ASSERT_EQUAL_FLOAT((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_FLOAT_ARRAY_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EQUAL_FLOAT_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_FLOAT_MESSAGE(expected, actual, num_elements, message)              UNITY_TEST_ASSERT_EACH_EQUAL_FLOAT((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_FLOAT_IS_INF_MESSAGE(actual, message)                                          UNITY_TEST_ASSERT_FLOAT_IS_INF((actual), __LINE__, (message))
#define TEST_ASSERT_FLOAT_IS_NEG_INF_MESSAGE(actual, message)                                      UNITY_TEST_ASSERT_FLOAT_IS_NEG_INF((actual), __LINE__, (message))
#define TEST_ASSERT_FLOAT_IS_NAN_MESSAGE(actual, message)                                          UNITY_TEST_ASSERT_FLOAT_IS_NAN((actual), __LINE__, (message))
#define TEST_ASSERT_FLOAT_IS_DETERMINATE_MESSAGE(actual, message)                                  UNITY_TEST_ASSERT_FLOAT_IS_DETERMINATE((actual), __LINE__, (message))
#define TEST_ASSERT_FLOAT_IS_NOT_INF_MESSAGE(actual, message)                                      UNITY_TEST_ASSERT_FLOAT_IS_NOT_INF((actual), __LINE__, (message))
#define TEST_ASSERT_FLOAT_IS_NOT_NEG_INF_MESSAGE(actual, message)                                  UNITY_TEST_ASSERT_FLOAT_IS_NOT_NEG_INF((actual), __LINE__, (message))
#define TEST_ASSERT_FLOAT_IS_NOT_NAN_MESSAGE(actual, message)                                      UNITY_TEST_ASSERT_FLOAT_IS_NOT_NAN((actual), __LINE__, (message))
#define TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE_MESSAGE(actual, message)                              UNITY_TEST_ASSERT_FLOAT_IS_NOT_DETERMINATE((actual), __LINE__, (message))

/* Double (If Enabled) */
#define TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(delta, expected, actual, message)                        UNITY_TEST_ASSERT_DOUBLE_WITHIN((delta), (expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected, actual, message)                                UNITY_TEST_ASSERT_EQUAL_DOUBLE((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected, actual, num_elements, message)            UNITY_TEST_ASSERT_EQUAL_DOUBLE_ARRAY((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_EACH_EQUAL_DOUBLE_MESSAGE(expected, actual, num_elements, message)             UNITY_TEST_ASSERT_EACH_EQUAL_DOUBLE((expected), (actual), (num_elements), __LINE__, (message))
#define TEST_ASSERT_DOUBLE_IS_INF_MESSAGE(actual, message)                                         UNITY_TEST_ASSERT_DOUBLE_IS_INF((actual), __LINE__, (message))
#define TEST_ASSERT_DOUBLE_IS_NEG_INF_MESSAGE(actual, message)                                     UNITY_TEST_ASSERT_DOUBLE_IS_NEG_INF((actual), __LINE__, (message))
#define TEST_ASSERT_DOUBLE_IS_NAN_MESSAGE(actual, message)                                         UNITY_TEST_ASSERT_DOUBLE_IS_NAN((actual), __LINE__, (message))
#define TEST_ASSERT_DOUBLE_IS_DETERMINATE_MESSAGE(actual, message)                                 UNITY_TEST_ASSERT_DOUBLE_IS_DETERMINATE((actual), __LINE__, (message))
#define TEST_ASSERT_DOUBLE_IS_NOT_INF_MESSAGE(actual, message)                                     UNITY_TEST_ASSERT_DOUBLE_IS_NOT_INF((actual), __LINE__, (message))
#define TEST_ASSERT_DOUBLE_IS_NOT_NEG_INF_MESSAGE(actual, message)                                 UNITY_TEST_ASSERT_DOUBLE_IS_NOT_NEG_INF((actual), __LINE__, (message))
#define TEST_ASSERT_DOUBLE_IS_NOT_NAN_MESSAGE(actual, message)                                     UNITY_TEST_ASSERT_DOUBLE_IS_NOT_NAN((actual), __LINE__, (message))
#define TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE_MESSAGE(actual, message)                             UNITY_TEST_ASSERT_DOUBLE_IS_NOT_DETERMINATE((actual), __LINE__, (message))

/* Shorthand */
#ifdef UNITY_SHORTHAND_AS_OLD
#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, message)                                       UNITY_TEST_ASSERT_EQUAL_INT((expected), (actual), __LINE__, (message))
#define TEST_ASSERT_NOT_EQUAL_MESSAGE(expected, actual, message)                                   UNITY_TEST_ASSERT(((expected) != (actual)), __LINE__, (message))
#endif
#ifdef UNITY_SHORTHAND_AS_INT
#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, message)                                       UNITY_TEST_ASSERT_EQUAL_INT((expected), (actual), __LINE__, message)
#define TEST_ASSERT_NOT_EQUAL_MESSAGE(expected, actual, message)                                   UNITY_TEST_FAIL(__LINE__, UnityStrErrShorthand)
#endif
#ifdef  UNITY_SHORTHAND_AS_MEM
#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, message)                                       UNITY_TEST_ASSERT_EQUAL_MEMORY((&expected), (&actual), sizeof(expected), __LINE__, message)
#define TEST_ASSERT_NOT_EQUAL_MESSAGE(expected, actual, message)                                   UNITY_TEST_FAIL(__LINE__, UnityStrErrShorthand)
#endif
#ifdef  UNITY_SHORTHAND_AS_RAW
#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, message)                                       UNITY_TEST_ASSERT(((expected) == (actual)), __LINE__, message)
#define TEST_ASSERT_NOT_EQUAL_MESSAGE(expected, actual, message)                                   UNITY_TEST_ASSERT(((expected) != (actual)), __LINE__, message)
#endif
#ifdef UNITY_SHORTHAND_AS_NONE
#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, message)                                       UNITY_TEST_FAIL(__LINE__, UnityStrErrShorthand)
#define TEST_ASSERT_NOT_EQUAL_MESSAGE(expected, actual, message)                                   UNITY_TEST_FAIL(__LINE__, UnityStrErrShorthand)
#endif

/* end of UNITY_FRAMEWORK_H */
#ifdef __cplusplus
}
#endif
#endif

#include <stddef.h>

#ifdef AVR
#include <avr/pgmspace.h>
#else
#define PROGMEM
#endif

/* If omitted from header, declare overrideable prototypes here so they're ready for use */
#ifdef UNITY_OMIT_OUTPUT_CHAR_HEADER_DECLARATION
void UNITY_OUTPUT_CHAR(int);
#endif

/* Helpful macros for us to use here in Assert functions */
#define UNITY_FAIL_AND_BAIL   { Unity.CurrentTestFailed  = 1; UNITY_OUTPUT_FLUSH(); TEST_ABORT(); }
#define UNITY_IGNORE_AND_BAIL { Unity.CurrentTestIgnored = 1; UNITY_OUTPUT_FLUSH(); TEST_ABORT(); }
#define RETURN_IF_FAIL_OR_IGNORE if (Unity.CurrentTestFailed || Unity.CurrentTestIgnored) TEST_ABORT()

struct UNITY_STORAGE_T Unity;

#ifdef UNITY_OUTPUT_COLOR
const char PROGMEM UnityStrOk[]                            = "\033[32mOK\033[00m";
const char PROGMEM UnityStrPass[]                          = "\033[32mPASS\033[00m";
const char PROGMEM UnityStrFail[]                          = "\033[31mFAIL\033[00m";
const char PROGMEM UnityStrIgnore[]                        = "\033[33mIGNORE\033[00m";
#else
const char PROGMEM UnityStrOk[]                            = "OK";
const char PROGMEM UnityStrPass[]                          = "PASS";
const char PROGMEM UnityStrFail[]                          = "FAIL";
const char PROGMEM UnityStrIgnore[]                        = "IGNORE";
#endif
static const char PROGMEM UnityStrNull[]                   = "NULL";
static const char PROGMEM UnityStrSpacer[]                 = ". ";
static const char PROGMEM UnityStrExpected[]               = " Expected ";
static const char PROGMEM UnityStrWas[]                    = " Was ";
static const char PROGMEM UnityStrGt[]                     = " to be greater than ";
static const char PROGMEM UnityStrLt[]                     = " to be less than ";
static const char PROGMEM UnityStrOrEqual[]                = "or equal to ";
static const char PROGMEM UnityStrNotEqual[]               = " to be not equal to ";
static const char PROGMEM UnityStrElement[]                = " Element ";
static const char PROGMEM UnityStrByte[]                   = " Byte ";
static const char PROGMEM UnityStrMemory[]                 = " Memory Mismatch.";
static const char PROGMEM UnityStrDelta[]                  = " Values Not Within Delta ";
static const char PROGMEM UnityStrPointless[]              = " You Asked Me To Compare Nothing, Which Was Pointless.";
static const char PROGMEM UnityStrNullPointerForExpected[] = " Expected pointer to be NULL";
static const char PROGMEM UnityStrNullPointerForActual[]   = " Actual pointer was NULL";
#ifndef UNITY_EXCLUDE_FLOAT
static const char PROGMEM UnityStrNot[]                    = "Not ";
static const char PROGMEM UnityStrInf[]                    = "Infinity";
static const char PROGMEM UnityStrNegInf[]                 = "Negative Infinity";
static const char PROGMEM UnityStrNaN[]                    = "NaN";
static const char PROGMEM UnityStrDet[]                    = "Determinate";
static const char PROGMEM UnityStrInvalidFloatTrait[]      = "Invalid Float Trait";
#endif
const char PROGMEM UnityStrErrShorthand[]                  = "Unity Shorthand Support Disabled";
const char PROGMEM UnityStrErrFloat[]                      = "Unity Floating Point Disabled";
const char PROGMEM UnityStrErrDouble[]                     = "Unity Double Precision Disabled";
const char PROGMEM UnityStrErr64[]                         = "Unity 64-bit Support Disabled";
static const char PROGMEM UnityStrBreaker[]                = "-----------------------";
static const char PROGMEM UnityStrResultsTests[]           = " Tests ";
static const char PROGMEM UnityStrResultsFailures[]        = " Failures ";
static const char PROGMEM UnityStrResultsIgnored[]         = " Ignored ";
#ifndef UNITY_EXCLUDE_DETAILS
static const char PROGMEM UnityStrDetail1Name[]            = UNITY_DETAIL1_NAME " ";
static const char PROGMEM UnityStrDetail2Name[]            = " " UNITY_DETAIL2_NAME " ";
#endif
/*-----------------------------------------------
 * Pretty Printers & Test Result Output Handlers
 *-----------------------------------------------*/

/*-----------------------------------------------*/
/* Local helper function to print characters. */
static void UnityPrintChar(const char* pch)
{
    /* printable characters plus CR & LF are printed */
    if ((*pch <= 126) && (*pch >= 32))
    {
        UNITY_OUTPUT_CHAR(*pch);
    }
    /* write escaped carriage returns */
    else if (*pch == 13)
    {
        UNITY_OUTPUT_CHAR('\\');
        UNITY_OUTPUT_CHAR('r');
    }
    /* write escaped line feeds */
    else if (*pch == 10)
    {
        UNITY_OUTPUT_CHAR('\\');
        UNITY_OUTPUT_CHAR('n');
    }
    /* unprintable characters are shown as codes */
    else
    {
        UNITY_OUTPUT_CHAR('\\');
        UNITY_OUTPUT_CHAR('x');
        UnityPrintNumberHex((UNITY_UINT)*pch, 2);
    }
}

/*-----------------------------------------------*/
/* Local helper function to print ANSI escape strings e.g. "\033[42m". */
#ifdef UNITY_OUTPUT_COLOR
static UNITY_UINT UnityPrintAnsiEscapeString(const char* string)
{
    const char* pch = string;
    UNITY_UINT count = 0;

    while (*pch && (*pch != 'm'))
    {
        UNITY_OUTPUT_CHAR(*pch);
        pch++;
        count++;
    }
    UNITY_OUTPUT_CHAR('m');
    count++;

    return count;
}
#endif

/*-----------------------------------------------*/
void UnityPrint(const char* string)
{
    const char* pch = string;

    if (pch != NULL)
    {
        while (*pch)
        {
#ifdef UNITY_OUTPUT_COLOR
            /* print ANSI escape code */
            if ((*pch == 27) && (*(pch + 1) == '['))
            {
                pch += UnityPrintAnsiEscapeString(pch);
                continue;
            }
#endif
            UnityPrintChar(pch);
            pch++;
        }
    }
}
/*-----------------------------------------------*/
void UnityPrintLen(const char* string, const UNITY_UINT32 length)
{
    const char* pch = string;

    if (pch != NULL)
    {
        while (*pch && ((UNITY_UINT32)(pch - string) < length))
        {
            /* printable characters plus CR & LF are printed */
            if ((*pch <= 126) && (*pch >= 32))
            {
                UNITY_OUTPUT_CHAR(*pch);
            }
            /* write escaped carriage returns */
            else if (*pch == 13)
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('r');
            }
            /* write escaped line feeds */
            else if (*pch == 10)
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('n');
            }
            /* unprintable characters are shown as codes */
            else
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('x');
                UnityPrintNumberHex((UNITY_UINT)*pch, 2);
            }
            pch++;
        }
    }
}

/*-----------------------------------------------*/
void UnityPrintNumberByStyle(const UNITY_INT number, const UNITY_DISPLAY_STYLE_T style)
{
    if ((style & UNITY_DISPLAY_RANGE_INT) == UNITY_DISPLAY_RANGE_INT)
    {
        if (style == UNITY_DISPLAY_STYLE_CHAR)
        {
            /* printable characters plus CR & LF are printed */
            UNITY_OUTPUT_CHAR('\'');
            if ((number <= 126) && (number >= 32))
            {
                UNITY_OUTPUT_CHAR((int)number);
            }
            /* write escaped carriage returns */
            else if (number == 13)
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('r');
            }
            /* write escaped line feeds */
            else if (number == 10)
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('n');
            }
            /* unprintable characters are shown as codes */
            else
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('x');
                UnityPrintNumberHex((UNITY_UINT)number, 2);
            }
            UNITY_OUTPUT_CHAR('\'');
        }
        else
        {
            UnityPrintNumber(number);
        }
    }
    else if ((style & UNITY_DISPLAY_RANGE_UINT) == UNITY_DISPLAY_RANGE_UINT)
    {
        UnityPrintNumberUnsigned((UNITY_UINT)number);
    }
    else
    {
        UNITY_OUTPUT_CHAR('0');
        UNITY_OUTPUT_CHAR('x');
        UnityPrintNumberHex((UNITY_UINT)number, (char)((style & 0xF) * 2));
    }
}

/*-----------------------------------------------*/
void UnityPrintNumber(const UNITY_INT number_to_print)
{
    UNITY_UINT number = (UNITY_UINT)number_to_print;

    if (number_to_print < 0)
    {
        /* A negative number, including MIN negative */
        UNITY_OUTPUT_CHAR('-');
        number = (~number) + 1;
    }
    UnityPrintNumberUnsigned(number);
}

/*-----------------------------------------------
 * basically do an itoa using as little ram as possible */
void UnityPrintNumberUnsigned(const UNITY_UINT number)
{
    UNITY_UINT divisor = 1;

    /* figure out initial divisor */
    while (number / divisor > 9)
    {
        divisor *= 10;
    }

    /* now mod and print, then divide divisor */
    do
    {
        UNITY_OUTPUT_CHAR((char)('0' + (number / divisor % 10)));
        divisor /= 10;
    } while (divisor > 0);
}

/*-----------------------------------------------*/
void UnityPrintNumberHex(const UNITY_UINT number, const char nibbles_to_print)
{
    int nibble;
    char nibbles = nibbles_to_print;

    if ((unsigned)nibbles > UNITY_MAX_NIBBLES)
    {
        nibbles = UNITY_MAX_NIBBLES;
    }

    while (nibbles > 0)
    {
        nibbles--;
        nibble = (int)(number >> (nibbles * 4)) & 0x0F;
        if (nibble <= 9)
        {
            UNITY_OUTPUT_CHAR((char)('0' + nibble));
        }
        else
        {
            UNITY_OUTPUT_CHAR((char)('A' - 10 + nibble));
        }
    }
}

/*-----------------------------------------------*/
void UnityPrintMask(const UNITY_UINT mask, const UNITY_UINT number)
{
    UNITY_UINT current_bit = (UNITY_UINT)1 << (UNITY_INT_WIDTH - 1);
    UNITY_INT32 i;

    for (i = 0; i < UNITY_INT_WIDTH; i++)
    {
        if (current_bit & mask)
        {
            if (current_bit & number)
            {
                UNITY_OUTPUT_CHAR('1');
            }
            else
            {
                UNITY_OUTPUT_CHAR('0');
            }
        }
        else
        {
            UNITY_OUTPUT_CHAR('X');
        }
        current_bit = current_bit >> 1;
    }
}

/*-----------------------------------------------*/
#ifndef UNITY_EXCLUDE_FLOAT_PRINT
/*
 * This function prints a floating-point value in a format similar to
 * printf("%.7g") on a single-precision machine or printf("%.9g") on a
 * double-precision machine.  The 7th digit won't always be totally correct
 * in single-precision operation (for that level of accuracy, a more
 * complicated algorithm would be needed).
 */
void UnityPrintFloat(const UNITY_DOUBLE input_number)
{
#ifdef UNITY_INCLUDE_DOUBLE
    static const int sig_digits = 9;
    static const UNITY_INT32 min_scaled = 100000000;
    static const UNITY_INT32 max_scaled = 1000000000;
#else
    static const int sig_digits = 7;
    static const UNITY_INT32 min_scaled = 1000000;
    static const UNITY_INT32 max_scaled = 10000000;
#endif

    UNITY_DOUBLE number = input_number;

    /* print minus sign (does not handle negative zero) */
    if (number < 0.0f)
    {
        UNITY_OUTPUT_CHAR('-');
        number = -number;
    }

    /* handle zero, NaN, and +/- infinity */
    if (number == 0.0f)
    {
        UnityPrint("0");
    }
    else if (isnan(number))
    {
        UnityPrint("nan");
    }
    else if (isinf(number))
    {
        UnityPrint("inf");
    }
    else
    {
        UNITY_INT32 n_int = 0, n;
        int exponent = 0;
        int decimals, digits;
        char buf[16] = {0};

        /*
         * Scale up or down by powers of 10.  To minimize rounding error,
         * start with a factor/divisor of 10^10, which is the largest
         * power of 10 that can be represented exactly.  Finally, compute
         * (exactly) the remaining power of 10 and perform one more
         * multiplication or division.
         */
        if (number < 1.0f)
        {
            UNITY_DOUBLE factor = 1.0f;

            while (number < (UNITY_DOUBLE)max_scaled / 1e10f)  { number *= 1e10f; exponent -= 10; }
            while (number * factor < (UNITY_DOUBLE)min_scaled) { factor *= 10.0f; exponent--; }

            number *= factor;
        }
        else if (number > (UNITY_DOUBLE)max_scaled)
        {
            UNITY_DOUBLE divisor = 1.0f;

            while (number > (UNITY_DOUBLE)min_scaled * 1e10f)   { number  /= 1e10f; exponent += 10; }
            while (number / divisor > (UNITY_DOUBLE)max_scaled) { divisor *= 10.0f; exponent++; }

            number /= divisor;
        }
        else
        {
            /*
             * In this range, we can split off the integer part before
             * doing any multiplications.  This reduces rounding error by
             * freeing up significant bits in the fractional part.
             */
            UNITY_DOUBLE factor = 1.0f;
            n_int = (UNITY_INT32)number;
            number -= (UNITY_DOUBLE)n_int;

            while (n_int < min_scaled) { n_int *= 10; factor *= 10.0f; exponent--; }

            number *= factor;
        }

        /* round to nearest integer */
        n = ((UNITY_INT32)(number + number) + 1) / 2;

#ifndef UNITY_ROUND_TIES_AWAY_FROM_ZERO
        /* round to even if exactly between two integers */
        if ((n & 1) && (((UNITY_DOUBLE)n - number) == 0.5f))
            n--;
#endif

        n += n_int;

        if (n >= max_scaled)
        {
            n = min_scaled;
            exponent++;
        }

        /* determine where to place decimal point */
        decimals = ((exponent <= 0) && (exponent >= -(sig_digits + 3))) ? (-exponent) : (sig_digits - 1);
        exponent += decimals;

        /* truncate trailing zeroes after decimal point */
        while ((decimals > 0) && ((n % 10) == 0))
        {
            n /= 10;
            decimals--;
        }

        /* build up buffer in reverse order */
        digits = 0;
        while ((n != 0) || (digits <= decimals))
        {
            buf[digits++] = (char)('0' + n % 10);
            n /= 10;
        }
        while (digits > 0)
        {
            if (digits == decimals) { UNITY_OUTPUT_CHAR('.'); }
            UNITY_OUTPUT_CHAR(buf[--digits]);
        }

        /* print exponent if needed */
        if (exponent != 0)
        {
            UNITY_OUTPUT_CHAR('e');

            if (exponent < 0)
            {
                UNITY_OUTPUT_CHAR('-');
                exponent = -exponent;
            }
            else
            {
                UNITY_OUTPUT_CHAR('+');
            }

            digits = 0;
            while ((exponent != 0) || (digits < 2))
            {
                buf[digits++] = (char)('0' + exponent % 10);
                exponent /= 10;
            }
            while (digits > 0)
            {
                UNITY_OUTPUT_CHAR(buf[--digits]);
            }
        }
    }
}
#endif /* ! UNITY_EXCLUDE_FLOAT_PRINT */

/*-----------------------------------------------*/
static void UnityTestResultsBegin(const char* file, const UNITY_LINE_TYPE line)
{
#ifdef UNITY_OUTPUT_FOR_ECLIPSE
    UNITY_OUTPUT_CHAR('(');
    UnityPrint(file);
    UNITY_OUTPUT_CHAR(':');
    UnityPrintNumber((UNITY_INT)line);
    UNITY_OUTPUT_CHAR(')');
    UNITY_OUTPUT_CHAR(' ');
    UnityPrint(Unity.CurrentTestName);
    UNITY_OUTPUT_CHAR(':');
#else
#ifdef UNITY_OUTPUT_FOR_IAR_WORKBENCH
    UnityPrint("<SRCREF line=");
    UnityPrintNumber((UNITY_INT)line);
    UnityPrint(" file=\"");
    UnityPrint(file);
    UNITY_OUTPUT_CHAR('"');
    UNITY_OUTPUT_CHAR('>');
    UnityPrint(Unity.CurrentTestName);
    UnityPrint("</SRCREF> ");
#else
#ifdef UNITY_OUTPUT_FOR_QT_CREATOR
    UnityPrint("file://");
    UnityPrint(file);
    UNITY_OUTPUT_CHAR(':');
    UnityPrintNumber((UNITY_INT)line);
    UNITY_OUTPUT_CHAR(' ');
    UnityPrint(Unity.CurrentTestName);
    UNITY_OUTPUT_CHAR(':');
#else
    UnityPrint(file);
    UNITY_OUTPUT_CHAR(':');
    UnityPrintNumber((UNITY_INT)line);
    UNITY_OUTPUT_CHAR(':');
    UnityPrint(Unity.CurrentTestName);
    UNITY_OUTPUT_CHAR(':');
#endif
#endif
#endif
}

/*-----------------------------------------------*/
static void UnityTestResultsFailBegin(const UNITY_LINE_TYPE line)
{
    UnityTestResultsBegin(Unity.TestFile, line);
    UnityPrint(UnityStrFail);
    UNITY_OUTPUT_CHAR(':');
}

/*-----------------------------------------------*/
void UnityConcludeTest(void)
{
    if (Unity.CurrentTestIgnored)
    {
        Unity.TestIgnores++;
    }
    else if (!Unity.CurrentTestFailed)
    {
        UnityTestResultsBegin(Unity.TestFile, Unity.CurrentTestLineNumber);
        UnityPrint(UnityStrPass);
    }
    else
    {
        Unity.TestFailures++;
    }

    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;
    UNITY_PRINT_EXEC_TIME();
    UNITY_PRINT_EOL();
    UNITY_FLUSH_CALL();
}

/*-----------------------------------------------*/
static void UnityAddMsgIfSpecified(const char* msg)
{
    if (msg)
    {
        UnityPrint(UnityStrSpacer);

#ifdef UNITY_PRINT_TEST_CONTEXT
        UNITY_PRINT_TEST_CONTEXT();
#endif
#ifndef UNITY_EXCLUDE_DETAILS
        if (Unity.CurrentDetail1)
        {
            UnityPrint(UnityStrDetail1Name);
            UnityPrint(Unity.CurrentDetail1);
            if (Unity.CurrentDetail2)
            {
                UnityPrint(UnityStrDetail2Name);
                UnityPrint(Unity.CurrentDetail2);
            }
            UnityPrint(UnityStrSpacer);
        }
#endif
        UnityPrint(msg);
    }
}

/*-----------------------------------------------*/
static void UnityPrintExpectedAndActualStrings(const char* expected, const char* actual)
{
    UnityPrint(UnityStrExpected);
    if (expected != NULL)
    {
        UNITY_OUTPUT_CHAR('\'');
        UnityPrint(expected);
        UNITY_OUTPUT_CHAR('\'');
    }
    else
    {
        UnityPrint(UnityStrNull);
    }
    UnityPrint(UnityStrWas);
    if (actual != NULL)
    {
        UNITY_OUTPUT_CHAR('\'');
        UnityPrint(actual);
        UNITY_OUTPUT_CHAR('\'');
    }
    else
    {
        UnityPrint(UnityStrNull);
    }
}

/*-----------------------------------------------*/
static void UnityPrintExpectedAndActualStringsLen(const char* expected,
                                                  const char* actual,
                                                  const UNITY_UINT32 length)
{
    UnityPrint(UnityStrExpected);
    if (expected != NULL)
    {
        UNITY_OUTPUT_CHAR('\'');
        UnityPrintLen(expected, length);
        UNITY_OUTPUT_CHAR('\'');
    }
    else
    {
        UnityPrint(UnityStrNull);
    }
    UnityPrint(UnityStrWas);
    if (actual != NULL)
    {
        UNITY_OUTPUT_CHAR('\'');
        UnityPrintLen(actual, length);
        UNITY_OUTPUT_CHAR('\'');
    }
    else
    {
        UnityPrint(UnityStrNull);
    }
}

/*-----------------------------------------------
 * Assertion & Control Helpers
 *-----------------------------------------------*/

/*-----------------------------------------------*/
static int UnityIsOneArrayNull(UNITY_INTERNAL_PTR expected,
                               UNITY_INTERNAL_PTR actual,
                               const UNITY_LINE_TYPE lineNumber,
                               const char* msg)
{
    /* Both are NULL or same pointer */
    if (expected == actual) { return 0; }

    /* print and return true if just expected is NULL */
    if (expected == NULL)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrNullPointerForExpected);
        UnityAddMsgIfSpecified(msg);
        return 1;
    }

    /* print and return true if just actual is NULL */
    if (actual == NULL)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrNullPointerForActual);
        UnityAddMsgIfSpecified(msg);
        return 1;
    }

    return 0; /* return false if neither is NULL */
}

/*-----------------------------------------------
 * Assertion Functions
 *-----------------------------------------------*/

/*-----------------------------------------------*/
void UnityAssertBits(const UNITY_INT mask,
                     const UNITY_INT expected,
                     const UNITY_INT actual,
                     const char* msg,
                     const UNITY_LINE_TYPE lineNumber)
{
    RETURN_IF_FAIL_OR_IGNORE;

    if ((mask & expected) != (mask & actual))
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrExpected);
        UnityPrintMask((UNITY_UINT)mask, (UNITY_UINT)expected);
        UnityPrint(UnityStrWas);
        UnityPrintMask((UNITY_UINT)mask, (UNITY_UINT)actual);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertEqualNumber(const UNITY_INT expected,
                            const UNITY_INT actual,
                            const char* msg,
                            const UNITY_LINE_TYPE lineNumber,
                            const UNITY_DISPLAY_STYLE_T style)
{
    RETURN_IF_FAIL_OR_IGNORE;

    if (expected != actual)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrExpected);
        UnityPrintNumberByStyle(expected, style);
        UnityPrint(UnityStrWas);
        UnityPrintNumberByStyle(actual, style);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertGreaterOrLessOrEqualNumber(const UNITY_INT threshold,
                                           const UNITY_INT actual,
                                           const UNITY_COMPARISON_T compare,
                                           const char *msg,
                                           const UNITY_LINE_TYPE lineNumber,
                                           const UNITY_DISPLAY_STYLE_T style)
{
    int failed = 0;
    RETURN_IF_FAIL_OR_IGNORE;

    if ((threshold == actual) && (compare & UNITY_EQUAL_TO)) { return; }
    if ((threshold == actual))                               { failed = 1; }

    if ((style & UNITY_DISPLAY_RANGE_INT) == UNITY_DISPLAY_RANGE_INT)
    {
        if ((actual > threshold) && (compare & UNITY_SMALLER_THAN)) { failed = 1; }
        if ((actual < threshold) && (compare & UNITY_GREATER_THAN)) { failed = 1; }
    }
    else /* UINT or HEX */
    {
        if (((UNITY_UINT)actual > (UNITY_UINT)threshold) && (compare & UNITY_SMALLER_THAN)) { failed = 1; }
        if (((UNITY_UINT)actual < (UNITY_UINT)threshold) && (compare & UNITY_GREATER_THAN)) { failed = 1; }
    }

    if (failed)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrExpected);
        UnityPrintNumberByStyle(actual, style);
        if (compare & UNITY_GREATER_THAN) { UnityPrint(UnityStrGt);       }
        if (compare & UNITY_SMALLER_THAN) { UnityPrint(UnityStrLt);       }
        if (compare & UNITY_EQUAL_TO)     { UnityPrint(UnityStrOrEqual);  }
        if (compare == UNITY_NOT_EQUAL)   { UnityPrint(UnityStrNotEqual); }
        UnityPrintNumberByStyle(threshold, style);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

#define UnityPrintPointlessAndBail()       \
{                                          \
    UnityTestResultsFailBegin(lineNumber); \
    UnityPrint(UnityStrPointless);         \
    UnityAddMsgIfSpecified(msg);           \
    UNITY_FAIL_AND_BAIL; }

/*-----------------------------------------------*/
void UnityAssertEqualIntArray(UNITY_INTERNAL_PTR expected,
                              UNITY_INTERNAL_PTR actual,
                              const UNITY_UINT32 num_elements,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber,
                              const UNITY_DISPLAY_STYLE_T style,
                              const UNITY_FLAGS_T flags)
{
    UNITY_UINT32 elements  = num_elements;
    unsigned int length    = style & 0xF;
    unsigned int increment = 0;

    RETURN_IF_FAIL_OR_IGNORE;

    if (num_elements == 0)
    {
        UnityPrintPointlessAndBail();
    }

    if (expected == actual)
    {
        return; /* Both are NULL or same pointer */
    }

    if (UnityIsOneArrayNull(expected, actual, lineNumber, msg))
    {
        UNITY_FAIL_AND_BAIL;
    }

    while ((elements > 0) && (elements--))
    {
        UNITY_INT expect_val;
        UNITY_INT actual_val;

        switch (length)
        {
            case 1:
                expect_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT8*)expected;
                actual_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT8*)actual;
                increment  = sizeof(UNITY_INT8);
                break;

            case 2:
                expect_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT16*)expected;
                actual_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT16*)actual;
                increment  = sizeof(UNITY_INT16);
                break;

#ifdef UNITY_SUPPORT_64
            case 8:
                expect_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT64*)expected;
                actual_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT64*)actual;
                increment  = sizeof(UNITY_INT64);
                break;
#endif

            default: /* default is length 4 bytes */
            case 4:
                expect_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT32*)expected;
                actual_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT32*)actual;
                increment  = sizeof(UNITY_INT32);
                length = 4;
                break;
        }

        if (expect_val != actual_val)
        {
            if ((style & UNITY_DISPLAY_RANGE_UINT) && (length < (UNITY_INT_WIDTH / 8)))
            {   /* For UINT, remove sign extension (padding 1's) from signed type casts above */
                UNITY_INT mask = 1;
                mask = (mask << 8 * length) - 1;
                expect_val &= mask;
                actual_val &= mask;
            }
            UnityTestResultsFailBegin(lineNumber);
            UnityPrint(UnityStrElement);
            UnityPrintNumberUnsigned(num_elements - elements - 1);
            UnityPrint(UnityStrExpected);
            UnityPrintNumberByStyle(expect_val, style);
            UnityPrint(UnityStrWas);
            UnityPrintNumberByStyle(actual_val, style);
            UnityAddMsgIfSpecified(msg);
            UNITY_FAIL_AND_BAIL;
        }
        /* Walk through array by incrementing the pointers */
        if (flags == UNITY_ARRAY_TO_ARRAY)
        {
            expected = (UNITY_INTERNAL_PTR)((const char*)expected + increment);
        }
        actual = (UNITY_INTERNAL_PTR)((const char*)actual + increment);
    }
}

/*-----------------------------------------------*/
#ifndef UNITY_EXCLUDE_FLOAT
/* Wrap this define in a function with variable types as float or double */
#define UNITY_FLOAT_OR_DOUBLE_WITHIN(delta, expected, actual, diff)                           \
    if (isinf(expected) && isinf(actual) && (((expected) < 0) == ((actual) < 0))) return 1;   \
    if (UNITY_NAN_CHECK) return 1;                                                            \
    (diff) = (actual) - (expected);                                                           \
    if ((diff) < 0) (diff) = -(diff);                                                         \
    if ((delta) < 0) (delta) = -(delta);                                                      \
    return !(isnan(diff) || isinf(diff) || ((diff) > (delta)))
    /* This first part of this condition will catch any NaN or Infinite values */
#ifndef UNITY_NAN_NOT_EQUAL_NAN
  #define UNITY_NAN_CHECK isnan(expected) && isnan(actual)
#else
  #define UNITY_NAN_CHECK 0
#endif

#ifndef UNITY_EXCLUDE_FLOAT_PRINT
  #define UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(expected, actual) \
  {                                                               \
    UnityPrint(UnityStrExpected);                                 \
    UnityPrintFloat(expected);                                    \
    UnityPrint(UnityStrWas);                                      \
    UnityPrintFloat(actual); }
#else
  #define UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(expected, actual) \
    UnityPrint(UnityStrDelta)
#endif /* UNITY_EXCLUDE_FLOAT_PRINT */

/*-----------------------------------------------*/
static int UnityFloatsWithin(UNITY_FLOAT delta, UNITY_FLOAT expected, UNITY_FLOAT actual)
{
    UNITY_FLOAT diff;
    UNITY_FLOAT_OR_DOUBLE_WITHIN(delta, expected, actual, diff);
}

/*-----------------------------------------------*/
void UnityAssertEqualFloatArray(UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* expected,
                                UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* actual,
                                const UNITY_UINT32 num_elements,
                                const char* msg,
                                const UNITY_LINE_TYPE lineNumber,
                                const UNITY_FLAGS_T flags)
{
    UNITY_UINT32 elements = num_elements;
    UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* ptr_expected = expected;
    UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* ptr_actual = actual;

    RETURN_IF_FAIL_OR_IGNORE;

    if (elements == 0)
    {
        UnityPrintPointlessAndBail();
    }

    if (expected == actual)
    {
        return; /* Both are NULL or same pointer */
    }

    if (UnityIsOneArrayNull((UNITY_INTERNAL_PTR)expected, (UNITY_INTERNAL_PTR)actual, lineNumber, msg))
    {
        UNITY_FAIL_AND_BAIL;
    }

    while (elements--)
    {
        if (!UnityFloatsWithin(*ptr_expected * UNITY_FLOAT_PRECISION, *ptr_expected, *ptr_actual))
        {
            UnityTestResultsFailBegin(lineNumber);
            UnityPrint(UnityStrElement);
            UnityPrintNumberUnsigned(num_elements - elements - 1);
            UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT((UNITY_DOUBLE)*ptr_expected, (UNITY_DOUBLE)*ptr_actual);
            UnityAddMsgIfSpecified(msg);
            UNITY_FAIL_AND_BAIL;
        }
        if (flags == UNITY_ARRAY_TO_ARRAY)
        {
            ptr_expected++;
        }
        ptr_actual++;
    }
}

/*-----------------------------------------------*/
void UnityAssertFloatsWithin(const UNITY_FLOAT delta,
                             const UNITY_FLOAT expected,
                             const UNITY_FLOAT actual,
                             const char* msg,
                             const UNITY_LINE_TYPE lineNumber)
{
    RETURN_IF_FAIL_OR_IGNORE;


    if (!UnityFloatsWithin(delta, expected, actual))
    {
        UnityTestResultsFailBegin(lineNumber);
        UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT((UNITY_DOUBLE)expected, (UNITY_DOUBLE)actual);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertFloatSpecial(const UNITY_FLOAT actual,
                             const char* msg,
                             const UNITY_LINE_TYPE lineNumber,
                             const UNITY_FLOAT_TRAIT_T style)
{
    const char* trait_names[] = {UnityStrInf, UnityStrNegInf, UnityStrNaN, UnityStrDet};
    UNITY_INT should_be_trait = ((UNITY_INT)style & 1);
    UNITY_INT is_trait        = !should_be_trait;
    UNITY_INT trait_index     = (UNITY_INT)(style >> 1);

    RETURN_IF_FAIL_OR_IGNORE;

    switch (style)
    {
        case UNITY_FLOAT_IS_INF:
        case UNITY_FLOAT_IS_NOT_INF:
            is_trait = isinf(actual) && (actual > 0);
            break;
        case UNITY_FLOAT_IS_NEG_INF:
        case UNITY_FLOAT_IS_NOT_NEG_INF:
            is_trait = isinf(actual) && (actual < 0);
            break;

        case UNITY_FLOAT_IS_NAN:
        case UNITY_FLOAT_IS_NOT_NAN:
            is_trait = isnan(actual) ? 1 : 0;
            break;

        case UNITY_FLOAT_IS_DET: /* A determinate number is non infinite and not NaN. */
        case UNITY_FLOAT_IS_NOT_DET:
            is_trait = !isinf(actual) && !isnan(actual);
            break;

        default: /* including UNITY_FLOAT_INVALID_TRAIT */
            trait_index = 0;
            trait_names[0] = UnityStrInvalidFloatTrait;
            break;
    }

    if (is_trait != should_be_trait)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrExpected);
        if (!should_be_trait)
        {
            UnityPrint(UnityStrNot);
        }
        UnityPrint(trait_names[trait_index]);
        UnityPrint(UnityStrWas);
#ifndef UNITY_EXCLUDE_FLOAT_PRINT
        UnityPrintFloat((UNITY_DOUBLE)actual);
#else
        if (should_be_trait)
        {
            UnityPrint(UnityStrNot);
        }
        UnityPrint(trait_names[trait_index]);
#endif
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

#endif /* not UNITY_EXCLUDE_FLOAT */

/*-----------------------------------------------*/
#ifndef UNITY_EXCLUDE_DOUBLE
static int UnityDoublesWithin(UNITY_DOUBLE delta, UNITY_DOUBLE expected, UNITY_DOUBLE actual)
{
    UNITY_DOUBLE diff;
    UNITY_FLOAT_OR_DOUBLE_WITHIN(delta, expected, actual, diff);
}

/*-----------------------------------------------*/
void UnityAssertEqualDoubleArray(UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* expected,
                                 UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* actual,
                                 const UNITY_UINT32 num_elements,
                                 const char* msg,
                                 const UNITY_LINE_TYPE lineNumber,
                                 const UNITY_FLAGS_T flags)
{
    UNITY_UINT32 elements = num_elements;
    UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* ptr_expected = expected;
    UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* ptr_actual = actual;

    RETURN_IF_FAIL_OR_IGNORE;

    if (elements == 0)
    {
        UnityPrintPointlessAndBail();
    }

    if (expected == actual)
    {
        return; /* Both are NULL or same pointer */
    }

    if (UnityIsOneArrayNull((UNITY_INTERNAL_PTR)expected, (UNITY_INTERNAL_PTR)actual, lineNumber, msg))
    {
        UNITY_FAIL_AND_BAIL;
    }

    while (elements--)
    {
        if (!UnityDoublesWithin(*ptr_expected * UNITY_DOUBLE_PRECISION, *ptr_expected, *ptr_actual))
        {
            UnityTestResultsFailBegin(lineNumber);
            UnityPrint(UnityStrElement);
            UnityPrintNumberUnsigned(num_elements - elements - 1);
            UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(*ptr_expected, *ptr_actual);
            UnityAddMsgIfSpecified(msg);
            UNITY_FAIL_AND_BAIL;
        }
        if (flags == UNITY_ARRAY_TO_ARRAY)
        {
            ptr_expected++;
        }
        ptr_actual++;
    }
}

/*-----------------------------------------------*/
void UnityAssertDoublesWithin(const UNITY_DOUBLE delta,
                              const UNITY_DOUBLE expected,
                              const UNITY_DOUBLE actual,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber)
{
    RETURN_IF_FAIL_OR_IGNORE;

    if (!UnityDoublesWithin(delta, expected, actual))
    {
        UnityTestResultsFailBegin(lineNumber);
        UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(expected, actual);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertDoubleSpecial(const UNITY_DOUBLE actual,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber,
                              const UNITY_FLOAT_TRAIT_T style)
{
    const char* trait_names[] = {UnityStrInf, UnityStrNegInf, UnityStrNaN, UnityStrDet};
    UNITY_INT should_be_trait = ((UNITY_INT)style & 1);
    UNITY_INT is_trait        = !should_be_trait;
    UNITY_INT trait_index     = (UNITY_INT)(style >> 1);

    RETURN_IF_FAIL_OR_IGNORE;

    switch (style)
    {
        case UNITY_FLOAT_IS_INF:
        case UNITY_FLOAT_IS_NOT_INF:
            is_trait = isinf(actual) && (actual > 0);
            break;
        case UNITY_FLOAT_IS_NEG_INF:
        case UNITY_FLOAT_IS_NOT_NEG_INF:
            is_trait = isinf(actual) && (actual < 0);
            break;

        case UNITY_FLOAT_IS_NAN:
        case UNITY_FLOAT_IS_NOT_NAN:
            is_trait = isnan(actual) ? 1 : 0;
            break;

        case UNITY_FLOAT_IS_DET: /* A determinate number is non infinite and not NaN. */
        case UNITY_FLOAT_IS_NOT_DET:
            is_trait = !isinf(actual) && !isnan(actual);
            break;

        default: /* including UNITY_FLOAT_INVALID_TRAIT */
            trait_index = 0;
            trait_names[0] = UnityStrInvalidFloatTrait;
            break;
    }

    if (is_trait != should_be_trait)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrExpected);
        if (!should_be_trait)
        {
            UnityPrint(UnityStrNot);
        }
        UnityPrint(trait_names[trait_index]);
        UnityPrint(UnityStrWas);
#ifndef UNITY_EXCLUDE_FLOAT_PRINT
        UnityPrintFloat(actual);
#else
        if (should_be_trait)
        {
            UnityPrint(UnityStrNot);
        }
        UnityPrint(trait_names[trait_index]);
#endif
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

#endif /* not UNITY_EXCLUDE_DOUBLE */

/*-----------------------------------------------*/
void UnityAssertNumbersWithin(const UNITY_UINT delta,
                              const UNITY_INT expected,
                              const UNITY_INT actual,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber,
                              const UNITY_DISPLAY_STYLE_T style)
{
    RETURN_IF_FAIL_OR_IGNORE;

    if ((style & UNITY_DISPLAY_RANGE_INT) == UNITY_DISPLAY_RANGE_INT)
    {
        if (actual > expected)
        {
            Unity.CurrentTestFailed = (((UNITY_UINT)actual - (UNITY_UINT)expected) > delta);
        }
        else
        {
            Unity.CurrentTestFailed = (((UNITY_UINT)expected - (UNITY_UINT)actual) > delta);
        }
    }
    else
    {
        if ((UNITY_UINT)actual > (UNITY_UINT)expected)
        {
            Unity.CurrentTestFailed = (((UNITY_UINT)actual - (UNITY_UINT)expected) > delta);
        }
        else
        {
            Unity.CurrentTestFailed = (((UNITY_UINT)expected - (UNITY_UINT)actual) > delta);
        }
    }

    if (Unity.CurrentTestFailed)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrDelta);
        UnityPrintNumberByStyle((UNITY_INT)delta, style);
        UnityPrint(UnityStrExpected);
        UnityPrintNumberByStyle(expected, style);
        UnityPrint(UnityStrWas);
        UnityPrintNumberByStyle(actual, style);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertNumbersArrayWithin(const UNITY_UINT delta,
                                   UNITY_INTERNAL_PTR expected,
                                   UNITY_INTERNAL_PTR actual,
                                   const UNITY_UINT32 num_elements,
                                   const char* msg,
                                   const UNITY_LINE_TYPE lineNumber,
                                   const UNITY_DISPLAY_STYLE_T style,
                                   const UNITY_FLAGS_T flags)
{
    UNITY_UINT32 elements = num_elements;
    unsigned int length   = style & 0xF;
    unsigned int increment = 0;

    RETURN_IF_FAIL_OR_IGNORE;

    if (num_elements == 0)
    {
        UnityPrintPointlessAndBail();
    }

    if (expected == actual)
    {
        return; /* Both are NULL or same pointer */
    }

    if (UnityIsOneArrayNull(expected, actual, lineNumber, msg))
    {
        UNITY_FAIL_AND_BAIL;
    }

    while ((elements > 0) && (elements--))
    {
        UNITY_INT expect_val;
        UNITY_INT actual_val;

        switch (length)
        {
            case 1:
                expect_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT8*)expected;
                actual_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT8*)actual;
                increment  = sizeof(UNITY_INT8);
                break;

            case 2:
                expect_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT16*)expected;
                actual_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT16*)actual;
                increment  = sizeof(UNITY_INT16);
                break;

#ifdef UNITY_SUPPORT_64
            case 8:
                expect_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT64*)expected;
                actual_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT64*)actual;
                increment  = sizeof(UNITY_INT64);
                break;
#endif

            default: /* default is length 4 bytes */
            case 4:
                expect_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT32*)expected;
                actual_val = *(UNITY_PTR_ATTRIBUTE const UNITY_INT32*)actual;
                increment  = sizeof(UNITY_INT32);
                length = 4;
                break;
        }

        if ((style & UNITY_DISPLAY_RANGE_INT) == UNITY_DISPLAY_RANGE_INT)
        {
            if (actual_val > expect_val)
            {
                Unity.CurrentTestFailed = (((UNITY_UINT)actual_val - (UNITY_UINT)expect_val) > delta);
            }
            else
            {
                Unity.CurrentTestFailed = (((UNITY_UINT)expect_val - (UNITY_UINT)actual_val) > delta);
            }
        }
        else
        {
            if ((UNITY_UINT)actual_val > (UNITY_UINT)expect_val)
            {
                Unity.CurrentTestFailed = (((UNITY_UINT)actual_val - (UNITY_UINT)expect_val) > delta);
            }
            else
            {
                Unity.CurrentTestFailed = (((UNITY_UINT)expect_val - (UNITY_UINT)actual_val) > delta);
            }
        }

        if (Unity.CurrentTestFailed)
        {
            if ((style & UNITY_DISPLAY_RANGE_UINT) && (length < (UNITY_INT_WIDTH / 8)))
            {   /* For UINT, remove sign extension (padding 1's) from signed type casts above */
                UNITY_INT mask = 1;
                mask = (mask << 8 * length) - 1;
                expect_val &= mask;
                actual_val &= mask;
            }
            UnityTestResultsFailBegin(lineNumber);
            UnityPrint(UnityStrDelta);
            UnityPrintNumberByStyle((UNITY_INT)delta, style);
            UnityPrint(UnityStrElement);
            UnityPrintNumberUnsigned(num_elements - elements - 1);
            UnityPrint(UnityStrExpected);
            UnityPrintNumberByStyle(expect_val, style);
            UnityPrint(UnityStrWas);
            UnityPrintNumberByStyle(actual_val, style);
            UnityAddMsgIfSpecified(msg);
            UNITY_FAIL_AND_BAIL;
        }
        /* Walk through array by incrementing the pointers */
        if (flags == UNITY_ARRAY_TO_ARRAY)
        {
            expected = (UNITY_INTERNAL_PTR)((const char*)expected + increment);
        }
        actual = (UNITY_INTERNAL_PTR)((const char*)actual + increment);
    }
}

/*-----------------------------------------------*/
void UnityAssertEqualString(const char* expected,
                            const char* actual,
                            const char* msg,
                            const UNITY_LINE_TYPE lineNumber)
{
    UNITY_UINT32 i;

    RETURN_IF_FAIL_OR_IGNORE;

    /* if both pointers not null compare the strings */
    if (expected && actual)
    {
        for (i = 0; expected[i] || actual[i]; i++)
        {
            if (expected[i] != actual[i])
            {
                Unity.CurrentTestFailed = 1;
                break;
            }
        }
    }
    else
    { /* handle case of one pointers being null (if both null, test should pass) */
        if (expected != actual)
        {
            Unity.CurrentTestFailed = 1;
        }
    }

    if (Unity.CurrentTestFailed)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrintExpectedAndActualStrings(expected, actual);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertEqualStringLen(const char* expected,
                               const char* actual,
                               const UNITY_UINT32 length,
                               const char* msg,
                               const UNITY_LINE_TYPE lineNumber)
{
    UNITY_UINT32 i;

    RETURN_IF_FAIL_OR_IGNORE;

    /* if both pointers not null compare the strings */
    if (expected && actual)
    {
        for (i = 0; (i < length) && (expected[i] || actual[i]); i++)
        {
            if (expected[i] != actual[i])
            {
                Unity.CurrentTestFailed = 1;
                break;
            }
        }
    }
    else
    { /* handle case of one pointers being null (if both null, test should pass) */
        if (expected != actual)
        {
            Unity.CurrentTestFailed = 1;
        }
    }

    if (Unity.CurrentTestFailed)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrintExpectedAndActualStringsLen(expected, actual, length);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertEqualStringArray(UNITY_INTERNAL_PTR expected,
                                 const char** actual,
                                 const UNITY_UINT32 num_elements,
                                 const char* msg,
                                 const UNITY_LINE_TYPE lineNumber,
                                 const UNITY_FLAGS_T flags)
{
    UNITY_UINT32 i = 0;
    UNITY_UINT32 j = 0;
    const char* expd = NULL;
    const char* act = NULL;

    RETURN_IF_FAIL_OR_IGNORE;

    /* if no elements, it's an error */
    if (num_elements == 0)
    {
        UnityPrintPointlessAndBail();
    }

    if ((const void*)expected == (const void*)actual)
    {
        return; /* Both are NULL or same pointer */
    }

    if (UnityIsOneArrayNull((UNITY_INTERNAL_PTR)expected, (UNITY_INTERNAL_PTR)actual, lineNumber, msg))
    {
        UNITY_FAIL_AND_BAIL;
    }

    if (flags != UNITY_ARRAY_TO_ARRAY)
    {
        expd = (const char*)expected;
    }

    do
    {
        act = actual[j];
        if (flags == UNITY_ARRAY_TO_ARRAY)
        {
            expd = ((const char* const*)expected)[j];
        }

        /* if both pointers not null compare the strings */
        if (expd && act)
        {
            for (i = 0; expd[i] || act[i]; i++)
            {
                if (expd[i] != act[i])
                {
                    Unity.CurrentTestFailed = 1;
                    break;
                }
            }
        }
        else
        { /* handle case of one pointers being null (if both null, test should pass) */
            if (expd != act)
            {
                Unity.CurrentTestFailed = 1;
            }
        }

        if (Unity.CurrentTestFailed)
        {
            UnityTestResultsFailBegin(lineNumber);
            if (num_elements > 1)
            {
                UnityPrint(UnityStrElement);
                UnityPrintNumberUnsigned(j);
            }
            UnityPrintExpectedAndActualStrings(expd, act);
            UnityAddMsgIfSpecified(msg);
            UNITY_FAIL_AND_BAIL;
        }
    } while (++j < num_elements);
}

/*-----------------------------------------------*/
void UnityAssertEqualMemory(UNITY_INTERNAL_PTR expected,
                            UNITY_INTERNAL_PTR actual,
                            const UNITY_UINT32 length,
                            const UNITY_UINT32 num_elements,
                            const char* msg,
                            const UNITY_LINE_TYPE lineNumber,
                            const UNITY_FLAGS_T flags)
{
    UNITY_PTR_ATTRIBUTE const unsigned char* ptr_exp = (UNITY_PTR_ATTRIBUTE const unsigned char*)expected;
    UNITY_PTR_ATTRIBUTE const unsigned char* ptr_act = (UNITY_PTR_ATTRIBUTE const unsigned char*)actual;
    UNITY_UINT32 elements = num_elements;
    UNITY_UINT32 bytes;

    RETURN_IF_FAIL_OR_IGNORE;

    if ((elements == 0) || (length == 0))
    {
        UnityPrintPointlessAndBail();
    }

    if (expected == actual)
    {
        return; /* Both are NULL or same pointer */
    }

    if (UnityIsOneArrayNull(expected, actual, lineNumber, msg))
    {
        UNITY_FAIL_AND_BAIL;
    }

    while (elements--)
    {
        bytes = length;
        while (bytes--)
        {
            if (*ptr_exp != *ptr_act)
            {
                UnityTestResultsFailBegin(lineNumber);
                UnityPrint(UnityStrMemory);
                if (num_elements > 1)
                {
                    UnityPrint(UnityStrElement);
                    UnityPrintNumberUnsigned(num_elements - elements - 1);
                }
                UnityPrint(UnityStrByte);
                UnityPrintNumberUnsigned(length - bytes - 1);
                UnityPrint(UnityStrExpected);
                UnityPrintNumberByStyle(*ptr_exp, UNITY_DISPLAY_STYLE_HEX8);
                UnityPrint(UnityStrWas);
                UnityPrintNumberByStyle(*ptr_act, UNITY_DISPLAY_STYLE_HEX8);
                UnityAddMsgIfSpecified(msg);
                UNITY_FAIL_AND_BAIL;
            }
            ptr_exp++;
            ptr_act++;
        }
        if (flags == UNITY_ARRAY_TO_VAL)
        {
            ptr_exp = (UNITY_PTR_ATTRIBUTE const unsigned char*)expected;
        }
    }
}

/*-----------------------------------------------*/

static union
{
    UNITY_INT8 i8;
    UNITY_INT16 i16;
    UNITY_INT32 i32;
#ifdef UNITY_SUPPORT_64
    UNITY_INT64 i64;
#endif
#ifndef UNITY_EXCLUDE_FLOAT
    float f;
#endif
#ifndef UNITY_EXCLUDE_DOUBLE
    double d;
#endif
} UnityQuickCompare;

UNITY_INTERNAL_PTR UnityNumToPtr(const UNITY_INT num, const UNITY_UINT8 size)
{
    switch(size)
    {
        case 1:
            UnityQuickCompare.i8 = (UNITY_INT8)num;
            return (UNITY_INTERNAL_PTR)(&UnityQuickCompare.i8);

        case 2:
            UnityQuickCompare.i16 = (UNITY_INT16)num;
            return (UNITY_INTERNAL_PTR)(&UnityQuickCompare.i16);

#ifdef UNITY_SUPPORT_64
        case 8:
            UnityQuickCompare.i64 = (UNITY_INT64)num;
            return (UNITY_INTERNAL_PTR)(&UnityQuickCompare.i64);
#endif

        default: /* 4 bytes */
            UnityQuickCompare.i32 = (UNITY_INT32)num;
            return (UNITY_INTERNAL_PTR)(&UnityQuickCompare.i32);
    }
}

#ifndef UNITY_EXCLUDE_FLOAT
/*-----------------------------------------------*/
UNITY_INTERNAL_PTR UnityFloatToPtr(const float num)
{
    UnityQuickCompare.f = num;
    return (UNITY_INTERNAL_PTR)(&UnityQuickCompare.f);
}
#endif

#ifndef UNITY_EXCLUDE_DOUBLE
/*-----------------------------------------------*/
UNITY_INTERNAL_PTR UnityDoubleToPtr(const double num)
{
    UnityQuickCompare.d = num;
    return (UNITY_INTERNAL_PTR)(&UnityQuickCompare.d);
}
#endif

/*-----------------------------------------------
 * printf helper function
 *-----------------------------------------------*/
#ifdef UNITY_INCLUDE_PRINT_FORMATTED
static void UnityPrintFVA(const char* format, va_list va)
{
    const char* pch = format;
    if (pch != NULL)
    {
        while (*pch)
        {
            /* format identification character */
            if (*pch == '%')
            {
                pch++;

                if (pch != NULL)
                {
                    switch (*pch)
                    {
                        case 'd':
                        case 'i':
                            {
                                const int number = va_arg(va, int);
                                UnityPrintNumber((UNITY_INT)number);
                                break;
                            }
#ifndef UNITY_EXCLUDE_FLOAT_PRINT
                        case 'f':
                        case 'g':
                            {
                                const double number = va_arg(va, double);
                                UnityPrintFloat((UNITY_DOUBLE)number);
                                break;
                            }
#endif
                        case 'u':
                            {
                                const unsigned int number = va_arg(va, unsigned int);
                                UnityPrintNumberUnsigned((UNITY_UINT)number);
                                break;
                            }
                        case 'b':
                            {
                                const unsigned int number = va_arg(va, unsigned int);
                                const UNITY_UINT mask = (UNITY_UINT)0 - (UNITY_UINT)1;
                                UNITY_OUTPUT_CHAR('0');
                                UNITY_OUTPUT_CHAR('b');
                                UnityPrintMask(mask, (UNITY_UINT)number);
                                break;
                            }
                        case 'x':
                        case 'X':
                        case 'p':
                            {
                                const unsigned int number = va_arg(va, unsigned int);
                                UNITY_OUTPUT_CHAR('0');
                                UNITY_OUTPUT_CHAR('x');
                                UnityPrintNumberHex((UNITY_UINT)number, 8);
                                break;
                            }
                        case 'c':
                            {
                                const int ch = va_arg(va, int);
                                UnityPrintChar((const char *)&ch);
                                break;
                            }
                        case 's':
                            {
                                const char * string = va_arg(va, const char *);
                                UnityPrint(string);
                                break;
                            }
                        case '%':
                            {
                                UnityPrintChar(pch);
                                break;
                            }
                        default:
                            {
                                /* print the unknown format character */
                                UNITY_OUTPUT_CHAR('%');
                                UnityPrintChar(pch);
                                break;
                            }
                    }
                }
            }
#ifdef UNITY_OUTPUT_COLOR
            /* print ANSI escape code */
            else if ((*pch == 27) && (*(pch + 1) == '['))
            {
                pch += UnityPrintAnsiEscapeString(pch);
                continue;
            }
#endif
            else if (*pch == '\n')
            {
                UNITY_PRINT_EOL();
            }
            else
            {
                UnityPrintChar(pch);
            }

            pch++;
        }
    }
}

void UnityPrintF(const UNITY_LINE_TYPE line, const char* format, ...)
{
    UnityTestResultsBegin(Unity.TestFile, line);
    UnityPrint("INFO");
    if(format != NULL)
    {
        UnityPrint(": ");
        va_list va;
        va_start(va, format);
        UnityPrintFVA(format, va);
        va_end(va);
    }
    UNITY_PRINT_EOL();
}
#endif /* ! UNITY_INCLUDE_PRINT_FORMATTED */


/*-----------------------------------------------
 * Control Functions
 *-----------------------------------------------*/

/*-----------------------------------------------*/
void UnityFail(const char* msg, const UNITY_LINE_TYPE line)
{
    RETURN_IF_FAIL_OR_IGNORE;

    UnityTestResultsBegin(Unity.TestFile, line);
    UnityPrint(UnityStrFail);
    if (msg != NULL)
    {
        UNITY_OUTPUT_CHAR(':');

#ifdef UNITY_PRINT_TEST_CONTEXT
        UNITY_PRINT_TEST_CONTEXT();
#endif
#ifndef UNITY_EXCLUDE_DETAILS
        if (Unity.CurrentDetail1)
        {
            UnityPrint(UnityStrDetail1Name);
            UnityPrint(Unity.CurrentDetail1);
            if (Unity.CurrentDetail2)
            {
                UnityPrint(UnityStrDetail2Name);
                UnityPrint(Unity.CurrentDetail2);
            }
            UnityPrint(UnityStrSpacer);
        }
#endif
        if (msg[0] != ' ')
        {
            UNITY_OUTPUT_CHAR(' ');
        }
        UnityPrint(msg);
    }

    UNITY_FAIL_AND_BAIL;
}

/*-----------------------------------------------*/
void UnityIgnore(const char* msg, const UNITY_LINE_TYPE line)
{
    RETURN_IF_FAIL_OR_IGNORE;

    UnityTestResultsBegin(Unity.TestFile, line);
    UnityPrint(UnityStrIgnore);
    if (msg != NULL)
    {
        UNITY_OUTPUT_CHAR(':');
        UNITY_OUTPUT_CHAR(' ');
        UnityPrint(msg);
    }
    UNITY_IGNORE_AND_BAIL;
}

/*-----------------------------------------------*/
void UnityMessage(const char* msg, const UNITY_LINE_TYPE line)
{
    UnityTestResultsBegin(Unity.TestFile, line);
    UnityPrint("INFO");
    if (msg != NULL)
    {
      UNITY_OUTPUT_CHAR(':');
      UNITY_OUTPUT_CHAR(' ');
      UnityPrint(msg);
    }
    UNITY_PRINT_EOL();
}

/*-----------------------------------------------*/
/* If we have not defined our own test runner, then include our default test runner to make life easier */
#ifndef UNITY_SKIP_DEFAULT_RUNNER
void UnityDefaultTestRun(UnityTestFunction Func, const char* FuncName, const int FuncLineNum)
{
    Unity.CurrentTestName = FuncName;
    Unity.CurrentTestLineNumber = (UNITY_LINE_TYPE)FuncLineNum;
    Unity.NumberOfTests++;
    UNITY_CLR_DETAILS();
    UNITY_EXEC_TIME_START();
    if (TEST_PROTECT())
    {
        setUp();
        Func();
    }
    if (TEST_PROTECT())
    {
        tearDown();
    }
    UNITY_EXEC_TIME_STOP();
    UnityConcludeTest();
}
#endif

/*-----------------------------------------------*/
void UnitySetTestFile(const char* filename)
{
    Unity.TestFile = filename;
}

/*-----------------------------------------------*/
void UnityBegin(const char* filename)
{
    Unity.TestFile = filename;
    Unity.CurrentTestName = NULL;
    Unity.CurrentTestLineNumber = 0;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;

    UNITY_CLR_DETAILS();
    UNITY_OUTPUT_START();
}

/*-----------------------------------------------*/
int UnityEnd(void)
{
    UNITY_PRINT_EOL();
    UnityPrint(UnityStrBreaker);
    UNITY_PRINT_EOL();
    UnityPrintNumber((UNITY_INT)(Unity.NumberOfTests));
    UnityPrint(UnityStrResultsTests);
    UnityPrintNumber((UNITY_INT)(Unity.TestFailures));
    UnityPrint(UnityStrResultsFailures);
    UnityPrintNumber((UNITY_INT)(Unity.TestIgnores));
    UnityPrint(UnityStrResultsIgnored);
    UNITY_PRINT_EOL();
    if (Unity.TestFailures == 0U)
    {
        UnityPrint(UnityStrOk);
    }
    else
    {
        UnityPrint(UnityStrFail);
#ifdef UNITY_DIFFERENTIATE_FINAL_FAIL
        UNITY_OUTPUT_CHAR('E'); UNITY_OUTPUT_CHAR('D');
#endif
    }
    UNITY_PRINT_EOL();
    UNITY_FLUSH_CALL();
    UNITY_OUTPUT_COMPLETE();
    return (int)(Unity.TestFailures);
}

/*-----------------------------------------------
 * Command Line Argument Support
 *-----------------------------------------------*/
#ifdef UNITY_USE_COMMAND_LINE_ARGS

char* UnityOptionIncludeNamed = NULL;
char* UnityOptionExcludeNamed = NULL;
int UnityVerbosity            = 1;

/*-----------------------------------------------*/
int UnityParseOptions(int argc, char** argv)
{
    int i;
    UnityOptionIncludeNamed = NULL;
    UnityOptionExcludeNamed = NULL;

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
                case 'l': /* list tests */
                    return -1;
                case 'n': /* include tests with name including this string */
                case 'f': /* an alias for -n */
                    if (argv[i][2] == '=')
                    {
                        UnityOptionIncludeNamed = &argv[i][3];
                    }
                    else if (++i < argc)
                    {
                        UnityOptionIncludeNamed = argv[i];
                    }
                    else
                    {
                        UnityPrint("ERROR: No Test String to Include Matches For");
                        UNITY_PRINT_EOL();
                        return 1;
                    }
                    break;
                case 'q': /* quiet */
                    UnityVerbosity = 0;
                    break;
                case 'v': /* verbose */
                    UnityVerbosity = 2;
                    break;
                case 'x': /* exclude tests with name including this string */
                    if (argv[i][2] == '=')
                    {
                        UnityOptionExcludeNamed = &argv[i][3];
                    }
                    else if (++i < argc)
                    {
                        UnityOptionExcludeNamed = argv[i];
                    }
                    else
                    {
                        UnityPrint("ERROR: No Test String to Exclude Matches For");
                        UNITY_PRINT_EOL();
                        return 1;
                    }
                    break;
                default:
                    UnityPrint("ERROR: Unknown Option ");
                    UNITY_OUTPUT_CHAR(argv[i][1]);
                    UNITY_PRINT_EOL();
                    return 1;
            }
        }
    }

    return 0;
}

/*-----------------------------------------------*/
int IsStringInBiggerString(const char* longstring, const char* shortstring)
{
    const char* lptr = longstring;
    const char* sptr = shortstring;
    const char* lnext = lptr;

    if (*sptr == '*')
    {
        return 1;
    }

    while (*lptr)
    {
        lnext = lptr + 1;

        /* If they current bytes match, go on to the next bytes */
        while (*lptr && *sptr && (*lptr == *sptr))
        {
            lptr++;
            sptr++;

            /* We're done if we match the entire string or up to a wildcard */
            if (*sptr == '*')
                return 1;
            if (*sptr == ',')
                return 1;
            if (*sptr == '"')
                return 1;
            if (*sptr == '\'')
                return 1;
            if (*sptr == ':')
                return 2;
            if (*sptr == 0)
                return 1;
        }

        /* Otherwise we start in the long pointer 1 character further and try again */
        lptr = lnext;
        sptr = shortstring;
    }

    return 0;
}

/*-----------------------------------------------*/
int UnityStringArgumentMatches(const char* str)
{
    int retval;
    const char* ptr1;
    const char* ptr2;
    const char* ptrf;

    /* Go through the options and get the substrings for matching one at a time */
    ptr1 = str;
    while (ptr1[0] != 0)
    {
        if ((ptr1[0] == '"') || (ptr1[0] == '\''))
        {
            ptr1++;
        }

        /* look for the start of the next partial */
        ptr2 = ptr1;
        ptrf = 0;
        do
        {
            ptr2++;
            if ((ptr2[0] == ':') && (ptr2[1] != 0) && (ptr2[0] != '\'') && (ptr2[0] != '"') && (ptr2[0] != ','))
            {
                ptrf = &ptr2[1];
            }
        } while ((ptr2[0] != 0) && (ptr2[0] != '\'') && (ptr2[0] != '"') && (ptr2[0] != ','));

        while ((ptr2[0] != 0) && ((ptr2[0] == ':') || (ptr2[0] == '\'') || (ptr2[0] == '"') || (ptr2[0] == ',')))
        {
            ptr2++;
        }

        /* done if complete filename match */
        retval = IsStringInBiggerString(Unity.TestFile, ptr1);
        if (retval == 1)
        {
            return retval;
        }

        /* done if testname match after filename partial match */
        if ((retval == 2) && (ptrf != 0))
        {
            if (IsStringInBiggerString(Unity.CurrentTestName, ptrf))
            {
                return 1;
            }
        }

        /* done if complete testname match */
        if (IsStringInBiggerString(Unity.CurrentTestName, ptr1) == 1)
        {
            return 1;
        }

        ptr1 = ptr2;
    }

    /* we couldn't find a match for any substrings */
    return 0;
}

/*-----------------------------------------------*/
int UnityTestMatches(void)
{
    /* Check if this test name matches the included test pattern */
    int retval;
    if (UnityOptionIncludeNamed)
    {
        retval = UnityStringArgumentMatches(UnityOptionIncludeNamed);
    }
    else
    {
        retval = 1;
    }

    /* Check if this test name matches the excluded test pattern */
    if (UnityOptionExcludeNamed)
    {
        if (UnityStringArgumentMatches(UnityOptionExcludeNamed))
        {
            retval = 0;
        }
    }

    return retval;
}

#endif /* UNITY_USE_COMMAND_LINE_ARGS */
/*-----------------------------------------------*/
