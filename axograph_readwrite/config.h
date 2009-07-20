#ifndef CONFIG_H
#define CONFIG_H 

/* ----------------------------------------------------------------------------------

    config.h : define the endianess and standard integer sizes on the current machine
	
	To run on little endian hardware (Intel, etc.) __LITTLE_ENDIAN__ must be defined
	This is done automatically under OS X / XCode, Windows / Visual C++, and Linux / gcc

    If your compiler is not C99 compliant, you may also need to define int16_t and
    int32_t; see the example for Visual C++ for how to do this.  

----------------------------------------------------------------------------------*/

// OS/X defines __LITTLE_ENDIAN__ automatically
// on Linux, we can check a standard header file to see if we're big or little endian
#ifdef linux 
#include <endian.h>
#ifdef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN__
#endif
#endif

// Most (possibly all?) of the Microsoft Visual C++ targets are little endian as of 2009
#ifdef _MSC_VER
#define __LITTLE_ENDIAN__
#endif

// If you're running something else that's little endian, uncomment the following
// line ( to force byte swaps data before reading or writing ).
// #define __LITTLE_ENDIAN__


// set up a set of integer types with known sizes
#ifdef _MSC_VER
// as of 2009, Visual C++ doesn't support the C99 standard so we'll have to manually define these types
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#else
// most everything else complies with the C99 standard, so we can use the types defined in the standard
#include <stdint.h>
#endif


#endif /* CONFIG_H */
