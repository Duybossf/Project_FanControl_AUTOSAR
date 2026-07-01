/**
 * @file Platform_Types.h
 *
 * @brief This is standard value AUTOSAR Classic Platform
 *
 * @author Duy Dang
 *
 * @date 10-06-2026
 *
 * @version v1.0.0
 *
 */
/****************************************************************************** */
#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H


/**
 * This standard AUTOSAR type shall only be used together with the definitions TRUE
 * and FALSE.
 */
typedef unsigned char boolean;

#ifndef TRUE
#define TRUE (boolean)1U
#endif

#ifndef FALSE
#define FALSE (boolean)0U
#endif

typedef unsigned char boolean;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

/****************************************************************************** */
typedef signed char sint8;
typedef signed short sint16;
typedef signed int sint32;
typedef signed long long sint64;

/****************************************************************************** */
typedef unsigned int uint8_least;
typedef unsigned int uint16_least;
typedef unsigned int uint32_least;
typedef signed int sint8_least;
typedef signed int sint16_least;
typedef signed int sint32_least;

/****************************************************************************** */
typedef float float32;
typedef double float64;

#endif /* PLATFORM_TYPES_H */
