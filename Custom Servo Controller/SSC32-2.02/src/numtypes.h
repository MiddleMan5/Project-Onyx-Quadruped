/***************************************************************************** 
** numtypes.h
** Copyright 2005, Mike Dvorsky.
**
** Typedefs for various numeric data types.  Provides for known-length
** variables.
*****************************************************************************/

#ifndef __NUMTYPES_H_
#define __NUMTYPES_H_

#define FALSE 0
#define TRUE ( !FALSE )

#define OFF 0
#define ON ( !OFF )

typedef float real32_t;

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef int int16_t;
typedef unsigned int uint16_t;

typedef long int32_t;
typedef unsigned long uint32_t;

typedef unsigned char boolean ;

#endif
