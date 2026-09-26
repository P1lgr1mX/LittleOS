#ifndef TYPES_H
#define TYPES_H

/* Fixed-width integer types for 32-bit x86 architecture */
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef signed int          int32_t;
typedef unsigned int        uint32_t;
typedef signed long long    int64_t;
typedef unsigned long long  uint64_t;

/* Pointer arithmetic and memory size types */
typedef uint32_t            uintptr_t;
typedef int32_t             intptr_t;
typedef uint32_t            size_t;
typedef int32_t             ssize_t;

/* Standard boolean type definitions compatible across C standards */
#ifndef __bool_true_false_are_defined
#if !defined(__cplusplus) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000L)
#ifndef bool
#define bool                _Bool
#define true                1
#define false               0
#endif
#endif
#define __bool_true_false_are_defined 1
#endif

/* Standard NULL pointer constant */
#ifndef NULL
#define NULL                ((void *)0)
#endif

#endif /* TYPES_H */
