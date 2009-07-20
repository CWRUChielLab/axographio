
#ifndef BYTESWAP_H
#define BYTESWAP_H

#include "config.h"

//------------------------ Byte Swap Routines  -------------------------

// swap bytes in a int16_t (2 byte) variable
void ByteSwapShort( int16_t *shortNumber );

// swap bytes in an int32_t (4 byte) variable
void ByteSwapLong( int32_t *int32_tNumber );

// swap bytes in a float (4 byte) variable
void ByteSwapFloat( float *floatNumber );

// swap bytes in a double (8 byte) variable
void ByteSwapDouble( double *doubleNumber );

// swap bytes in a int16_t (2 byte) array
void ByteSwapShortArray( int16_t *shortArray, int32_t arraySize );

// swap bytes in an int32_t (4 byte) array
void ByteSwapLongArray( int32_t *int32_tArray, int32_t arraySize );

// swap bytes in a float (4 byte) array
void ByteSwapFloatArray( float *floatArray, int32_t arraySize );

// swap bytes in a double (8 byte) array
void ByteSwapDoubleArray( double *doubleArray, int32_t arraySize );


#endif
