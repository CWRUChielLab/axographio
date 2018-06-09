// ******************************************************************************************
// Graph Document Routines 
// Copyright © 1996 Dr. John Clements.  All rights reserved. 
// ******************************************************************************************

#include "byteswap.h"

void ByteSwapShort( int16_t *shortNumber )
{
	uint16_t *uShortNumber = ( uint16_t * )shortNumber;
	
	*uShortNumber = ( ( *uShortNumber >> 8 ) | ( *uShortNumber << 8 ) );
}


void ByteSwapLong( int32_t *longNumber )
{
	uint32_t *uLongNumber = ( uint32_t * )longNumber;
	
	*uLongNumber = ( ( ( *uLongNumber & 0x000000FF )<<24 ) + ( ( *uLongNumber & 0x0000FF00 )<<8 ) +
					 ( ( *uLongNumber & 0x00FF0000 )>>8 ) +  ( ( *uLongNumber & 0xFF000000 )>>24 ) );
}


void ByteSwapFloat( float *floatNumber )
{
	uint32_t *uLongNumber = ( uint32_t * )floatNumber;
	
	*uLongNumber = ( ( ( *uLongNumber & 0x000000FF )<<24 ) + ( ( *uLongNumber & 0x0000FF00 )<<8 ) +
					 ( ( *uLongNumber & 0x00FF0000 )>>8 ) +  ( ( *uLongNumber & 0xFF000000 )>>24 ) );
}


void ByteSwapDouble( double *doubleNumber )
{
	// cast the double to an array of two unsigned ints
	uint32_t *uLongArray = ( uint32_t * )doubleNumber;
	
	// swap the bytes in each int32_t
	ByteSwapLong( ( int32_t * )&uLongArray[0] );
	ByteSwapLong( ( int32_t * )&uLongArray[1] );
	
	// swap the two longs
	uint32_t saveLong0 = uLongArray[0];
	uLongArray[0] = uLongArray[1];
	uLongArray[1] = saveLong0;
}


void ByteSwapShortArray( int16_t *shortArray, int arraySize )
{
	for ( int i = 0; i < arraySize; i++ )
	{
		ByteSwapShort( shortArray++ );
	}
}


void ByteSwapLongArray( int32_t *longArray, int arraySize )
{
	for ( int i = 0; i < arraySize; i++ )
	{
		ByteSwapLong( longArray++ );
	}
}


void ByteSwapFloatArray( float *floatArray, int arraySize )
{
	for ( int i = 0; i < arraySize; i++ )
	{
		ByteSwapFloat( floatArray++ );
	}
}


void ByteSwapDoubleArray( double *doubleArray, int arraySize )
{
	for ( int i = 0; i < arraySize; i++ )
	{
		ByteSwapDouble( doubleArray++ );
	}
}


