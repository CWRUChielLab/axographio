/* ----------------------------------------------------------------------------------

	AxoGraph_ReadWrite : functions for reading and writing AxoGraph data files. 
	
	See also : AxoGraph_ReadWrite.h 
	
	This source code and the AxoGraph data file format are in the public domain. 

---------------------------------------------------------------------------------- */

#include <string.h>
#include <stdlib.h>

#include "fileUtils.h"
#include "stringUtils.h"
#include "byteswap.h"

#include "AxoGraph_ReadWrite.h"

int AG_GetFileFormat( const AGDataRef refNum, int *fileFormat )
{
	*fileFormat = 0;
	
	// Read the file header 
	int posn = 0;
	int result = SetFilePosition( refNum, posn );		// Position the mark at start
	if ( result ) 
		return result;
	
	// Read the 4-byte prefix present in all AxoGraph file formats
	unsigned char AxoGraphFileID[4];
	long bytes = 4;	// 4 byte identifier
	result = ReadFromFile( refNum, &bytes, AxoGraphFileID );
	if ( result ) 
		return result;
	
	// Check the prefix 
	if ( memcmp( AxoGraphFileID, kAxoGraph4DocType, 4 ) == 0 ) 
	{
		// Got an AxoGraph version 4 format file. Read the file type.
		short version;
		bytes = sizeof( short );
		result = ReadFromFile( refNum, &bytes, &version );
		if ( result ) 
			return result;
		
#ifdef __LITTLE_ENDIAN__
		ByteSwapShort( &version );
#endif
		
		if ( version != kAxoGraph_Graph_Format && 
			 version != kAxoGraph_Digitized_Format  )  
			return kAG_VersionErr;
		
		// Return the file format 
		*fileFormat = version;
	}
	else if ( memcmp( AxoGraphFileID, kAxoGraphXDocType, 4 ) == 0 ) 
	{
		// Got an AxoGraph X format file. Check the file version.
		int32_t version;
		bytes = sizeof( int32_t );
		result = ReadFromFile( refNum, &bytes, &version );
		if ( result ) 
			return result;
		
#ifdef __LITTLE_ENDIAN__
		ByteSwapLong( &version );
#endif
		
		if ( version < 3 || version > kAxoGraph_X_Format )
		{
			return kAG_VersionErr;
		}
		
		// update to latest version number
		version = kAxoGraph_X_Format;
		
		// Return the file format 
		*fileFormat = version;
	}
	else
	{
		result = kAG_FormatErr;
	}
	
	// pass back the result ( = 0 if all went well) 
	return result;
}



int AG_GetNumberOfColumns( const AGDataRef refNum, const int fileFormat, int32_t *numberOfColumns )
{
	*numberOfColumns = 0;
	
	if ( fileFormat == kAxoGraph_Digitized_Format || fileFormat == kAxoGraph_Graph_Format ) 
	{
		// Read the number of columns (short integer in AxoGraph 4 files)
		short nColumns;
		long bytes = 2;
		int result = ReadFromFile( refNum, &bytes, &nColumns);
		if ( result ) 
			return result;
		
#ifdef __LITTLE_ENDIAN__
		ByteSwapShort( &nColumns );
#endif
		
		*numberOfColumns = nColumns;
		return result;
	}
	else if ( fileFormat == kAxoGraph_X_Format )
	{
		// Read the number of columns (int32_t integer in AxoGraph X files) 
		int32_t nColumns;
		long bytes = 4;
		int result = ReadFromFile( refNum, &bytes, &nColumns);
		if ( result ) 
			return result;
		
#ifdef __LITTLE_ENDIAN__
		ByteSwapLong( &nColumns );
#endif
		
		*numberOfColumns = nColumns;
		return result;
	}
	else
		return -1;
}



int AG_ReadColumn( const AGDataRef refNum, const int fileFormat, const int columnNumber, ColumnData *columnData )
{
	// Initialize in case of error during read
	columnData->points = 0;
	columnData->title = NULL;
	
	switch ( fileFormat ) 
	{
		case kAxoGraph_Graph_Format:
		{
			// Read the standard column header 
			ColumnHeader columnHeader;		
			long bytes = sizeof( ColumnHeader );
			int result = ReadFromFile( refNum, &bytes, &columnHeader );
			if ( result ) 
				return result;
			
#ifdef __LITTLE_ENDIAN__
			ByteSwapLong( &columnHeader.points );
#endif
			
			// Retrieve the title and number of points in the column 
			columnData->type = FloatArrayType;
			columnData->points = columnHeader.points;
			columnData->title = ( unsigned char * )malloc( 80 );
			PascalToCString( columnHeader.title );
			memcpy( columnData->title, columnHeader.title, 80 );
			
			// create a new pointer to receive the data
			long columnBytes = columnHeader.points * sizeof( float );
			columnData->floatArray = ( float * )malloc( columnBytes );
			if ( columnData->floatArray == NULL ) 
				return kAG_MemoryErr;
			
			// Read in the column's data 
			result = ReadFromFile( refNum, &columnBytes, columnData->floatArray );

#ifdef __LITTLE_ENDIAN__
			ByteSwapFloatArray( columnData->floatArray, columnHeader.points );
#endif
			
			return result;
		}
			
		case kAxoGraph_Digitized_Format:
		{
			if ( columnNumber == 0 )
			{
				// Read the column header 
				DigitizedFirstColumnHeader columnHeader;		
				long bytes = sizeof( DigitizedFirstColumnHeader );
				int result = ReadFromFile( refNum, &bytes, &columnHeader );
				if ( result ) 
					return result;
				
#ifdef __LITTLE_ENDIAN__
				ByteSwapLong( &columnHeader.points );
				ByteSwapFloat( &columnHeader.firstPoint );
				ByteSwapFloat( &columnHeader.sampleInterval );
#endif
				
				// Retrieve the title, number of points in the column, and sample interval
				columnData->type = SeriesArrayType;
				columnData->points = columnHeader.points;
				columnData->title = ( unsigned char * )malloc( 80 );
				PascalToCString( columnHeader.title );
				memcpy( columnData->title, columnHeader.title, 80 );
				
				columnData->seriesArray.firstValue = columnHeader.firstPoint;
				columnData->seriesArray.increment = columnHeader.sampleInterval;
				return result;
			}
			else
			{
				// Read the column header 
				DigitizedColumnHeader columnHeader;		
				long bytes = sizeof( DigitizedColumnHeader );
				int result = ReadFromFile( refNum, &bytes, &columnHeader );
				if ( result ) 
					return result;
				
#ifdef __LITTLE_ENDIAN__
				ByteSwapLong( &columnHeader.points );
				ByteSwapFloat( &columnHeader.scalingFactor );
#endif
				
				// Retrieve the title and number of points in the column 
				columnData->type = ScaledShortArrayType;
				columnData->points = columnHeader.points;
				columnData->title = ( unsigned char * )malloc( 80 );
				PascalToCString( columnHeader.title );
				memcpy( columnData->title, columnHeader.title, 80 );
				
				columnData->scaledShortArray.scale = columnHeader.scalingFactor;
				columnData->scaledShortArray.offset = 0;
				
				// create a new pointer to receive the data
				long columnBytes = columnHeader.points * sizeof( short );
				columnData->scaledShortArray.shortArray = ( short * )malloc( columnBytes );
				if ( columnData->scaledShortArray.shortArray == NULL ) 
					return kAG_MemoryErr;
				
				// Read in the column's data 
				result = ReadFromFile( refNum, &columnBytes, columnData->scaledShortArray.shortArray );
				
#ifdef __LITTLE_ENDIAN__
				ByteSwapShortArray( columnData->scaledShortArray.shortArray, columnHeader.points );
#endif
				
				return result;
			}
		}
			
		case kAxoGraph_X_Format:
		{
			// Read the column header 
			AxoGraphXColumnHeader columnHeader;		
			long bytes = sizeof( AxoGraphXColumnHeader );
			int result = ReadFromFile( refNum, &bytes, &columnHeader );
			if ( result ) 
				return result;
			
#ifdef __LITTLE_ENDIAN__
			ByteSwapLong( &columnHeader.points );
			ByteSwapLong( &columnHeader.dataType );
			ByteSwapLong( &columnHeader.titleLength );
#endif
			
			// Retrieve the column type and number of points in the column 
			columnData->type = (ColumnType)columnHeader.dataType;
			columnData->points = columnHeader.points;
			
			// sanity check on column type
			if ( columnData->type < 0 || columnData->type > 14 )
				return -1;
			
			// Read the column title 
			columnData->titleLength = columnHeader.titleLength;
            // need to allocate at least 1 byte for the null terminator of the C string
            if (columnHeader.titleLength > 0) 
    			columnData->title = ( unsigned char * )malloc( columnHeader.titleLength );
            else 
    			columnData->title = ( unsigned char * )malloc( 1 );
			long titleLength = columnHeader.titleLength;
            result = ReadFromFile( refNum, &titleLength, columnData->title );
			if ( result ) 
				return result;
			
			UnicodeToCString( columnData->title, columnData->titleLength );
			
			switch ( columnHeader.dataType ) 
			{
				case ShortArrayType:
				{
					// create a new pointer to receive the data
					long columnBytes = columnHeader.points * sizeof( short );
					columnData->shortArray = ( short * )malloc( columnBytes );
					if ( columnData->shortArray == NULL ) 
						return kAG_MemoryErr;
					
					// Read in the column's data 
					result = ReadFromFile( refNum, &columnBytes, columnData->shortArray );

#ifdef __LITTLE_ENDIAN__
					ByteSwapShortArray( columnData->shortArray, columnHeader.points );
#endif
					
					return result;
				}
				case IntArrayType:
				{
					// create a new pointer to receive the data
					long columnBytes = columnHeader.points * sizeof( int );
					columnData->intArray = ( int * )malloc( columnBytes );
					if ( columnData->intArray == NULL ) 
						return kAG_MemoryErr;
					
					// Read in the column's data 
					result = ReadFromFile( refNum, &columnBytes, columnData->intArray );
					
#ifdef __LITTLE_ENDIAN__
					ByteSwapLongArray( (int32_t *)columnData->intArray, columnHeader.points );
#endif
					
					return result;
				}
				case FloatArrayType:
				{
					// create a new pointer to receive the data
					long columnBytes = columnHeader.points * sizeof( float );
					columnData->floatArray = ( float * )malloc( columnBytes );
					if ( columnData->floatArray == NULL ) 
						return kAG_MemoryErr;
					
					// Read in the column's data 
					result = ReadFromFile( refNum, &columnBytes, columnData->floatArray );
					
#ifdef __LITTLE_ENDIAN__
					ByteSwapFloatArray( columnData->floatArray, columnHeader.points );
#endif
					
					return result;
				}
				case DoubleArrayType:
				{
					// create a new pointer to receive the data
					long columnBytes = columnHeader.points * sizeof( double );
					columnData->doubleArray = ( double * )malloc( columnBytes );
					if ( columnData->doubleArray == NULL ) 
						return kAG_MemoryErr;
					
					// Read in the column's data 
					result = ReadFromFile( refNum, &columnBytes, columnData->doubleArray );
					
#ifdef __LITTLE_ENDIAN__
					ByteSwapDoubleArray( columnData->doubleArray, columnHeader.points );
#endif
					
					return result;
				}
				case SeriesArrayType:
				{
					SeriesArray seriesParameters;
					long bytes = sizeof( SeriesArray );
					result = ReadFromFile( refNum, &bytes, &seriesParameters );
					
#ifdef __LITTLE_ENDIAN__
					ByteSwapDouble( &seriesParameters.firstValue );
					ByteSwapDouble( &seriesParameters.increment );
#endif
					
					columnData->seriesArray.firstValue = seriesParameters.firstValue;
					columnData->seriesArray.increment = seriesParameters.increment;
					return result;
				}
				case ScaledShortArrayType:
				{
					double scale, offset;
					long bytes = sizeof( double );
					result = ReadFromFile( refNum, &bytes, &scale );
					result = ReadFromFile( refNum, &bytes, &offset );
					
#ifdef __LITTLE_ENDIAN__
					ByteSwapDouble( &scale );
					ByteSwapDouble( &offset );
#endif
					
					columnData->scaledShortArray.scale = scale;
					columnData->scaledShortArray.offset = offset;
					
					// create a new pointer to receive the data
					long columnBytes = columnHeader.points * sizeof( short );
					columnData->scaledShortArray.shortArray = ( short * )malloc( columnBytes );
					if ( columnData->scaledShortArray.shortArray == NULL ) 
						return kAG_MemoryErr;
					
					// Read in the column's data 
					result = ReadFromFile( refNum, &columnBytes, columnData->scaledShortArray.shortArray );
					
#ifdef __LITTLE_ENDIAN__
					ByteSwapShortArray( columnData->scaledShortArray.shortArray, columnHeader.points );
#endif
					
					return result;
				}
			}
		}
			
		default:
		{
			return -1;
		}
	}
}


int AG_ReadFloatColumn( const AGDataRef refNum, const int fileFormat, const int columnNumber, ColumnData *columnData )
{
	int result = AG_ReadColumn( refNum, fileFormat, columnNumber, columnData );
	
	// If necessary, convert the columnData to FloatArrayType
	switch ( columnData->type ) 
	{
		case ShortArrayType:
		{
			// create a new pointer to receive the converted data
			long columnBytes = columnData->points * sizeof( float );
			short *shortArray = columnData->shortArray;
			float *floatArray = ( float * )malloc( columnBytes );
			if ( floatArray == NULL ) 
				return kAG_MemoryErr;
			
			// Convert in the column data 
			for ( int32_t i=0; i<columnData->points; i++ )
			{
				floatArray[i] = shortArray[i];
			}
			
			// free old short array
			free( columnData->shortArray );
			columnData->shortArray = NULL;
			
			// pass in new float array
			columnData->floatArray = floatArray;
			columnData->type = FloatArrayType;
			return result;
		}
		case IntArrayType:
		{
			// create a new pointer to receive the converted data
			long columnBytes = columnData->points * sizeof( float );
			int *intArray = columnData->intArray;
			float *floatArray = ( float * )malloc( columnBytes );
			if ( floatArray == NULL ) 
				return kAG_MemoryErr;
			
			// Convert in the column data 
			for ( int32_t i=0; i<columnData->points; i++ )
			{
				floatArray[i] = intArray[i];
			}
			
			// free old short array
			free( columnData->intArray );
			columnData->intArray = NULL;
			
			// pass in new float array
			columnData->floatArray = floatArray;
			columnData->type = FloatArrayType;
			return result;
		}
		case FloatArrayType:
		{
			// Don't need to convert
			return result;
		}
		case DoubleArrayType:
		{
			// create a new pointer to receive the converted data
			long columnBytes = columnData->points * sizeof( float );
			double *doubleArray = columnData->doubleArray;
			float *floatArray = ( float * )malloc( columnBytes );
			if ( floatArray == NULL ) 
				return kAG_MemoryErr;
			
			// Convert in the column data 
			for ( int32_t i=0; i<columnData->points; i++ )
			{
				floatArray[i] = doubleArray[i];
			}
			
			// free old short array
			free( columnData->doubleArray );
			columnData->doubleArray = NULL;
			
			// pass in new float array
			columnData->floatArray = floatArray;
			columnData->type = FloatArrayType;
			return result;
		}
		case SeriesArrayType:
		{
			// create a new pointer to receive the converted data
			long columnBytes = columnData->points * sizeof( float );
			double firstValue = columnData->seriesArray.firstValue;
			double increment = columnData->seriesArray.increment;
			float *floatArray = ( float * )malloc( columnBytes );
			if ( floatArray == NULL ) 
				return kAG_MemoryErr;
			
			// Convert in the column data 
			for ( int32_t i=0; i<columnData->points; i++ )
			{
				floatArray[i] = firstValue + i * increment;
			}
			
			// pass in new float array
			columnData->floatArray = floatArray;
			columnData->type = FloatArrayType;
			return result;
		}
		case ScaledShortArrayType:
		{
			// create a new pointer to receive the converted data
			long columnBytes = columnData->points * sizeof( float );
			double scale = columnData->scaledShortArray.scale;
			double offset = columnData->scaledShortArray.offset;
			short *shortArray = columnData->scaledShortArray.shortArray;
			float *floatArray = ( float * )malloc( columnBytes );
			if ( floatArray == NULL ) 
				return kAG_MemoryErr;
			
			// Convert in the column data 
			for ( int32_t i=0; i<columnData->points; i++ )
			{
				floatArray[i] = shortArray[i] * scale + offset;
			}
			
			// free old short array
			free( columnData->scaledShortArray.shortArray );
			columnData->scaledShortArray.shortArray = NULL;
			
			// pass in new float array
			columnData->floatArray = floatArray;
			columnData->type = FloatArrayType;
			return result;
		}
		default:
		{
			return result;
		}
	}
}


int AG_WriteHeader( const AGDataRef refNum, const int fileFormat, const int32_t numberOfColumns )
{
	if ( fileFormat == kAxoGraph_Digitized_Format || fileFormat == kAxoGraph_Graph_Format ) 
	{
		// Set up the file prefix 
		unsigned char AxoGraphFileID[4];
		memcpy( AxoGraphFileID, kAxoGraph4DocType, 4 );
		
		// Position at start of file 
		int posn = 0;
		int result = SetFilePosition( refNum, posn );		// Position the mark 
		if ( result ) 
			return result;
		
		// Write the file prefix 
		long bytes = 4;
		result = WriteToFile( refNum, &bytes, AxoGraphFileID );	// write the prefix 
		if ( result ) 
			return result;
		
		// Write the header 
		short shortFormat = fileFormat;
		short shortColumns = numberOfColumns;
		
#ifdef __LITTLE_ENDIAN__
		ByteSwapShort( &shortFormat );
		ByteSwapShort( &shortColumns );
#endif
		
		bytes = sizeof( short );
		result = WriteToFile( refNum, &bytes, &shortFormat );
		result = WriteToFile( refNum, &bytes, &shortColumns );
		
		// pass back the result ( = 0 if all went well) 
		return result;
	}
	else if ( fileFormat == kAxoGraph_X_Format )
	{
		// Set up the file prefix 
		unsigned char AxoGraphFileID[4];
		memcpy( AxoGraphFileID, kAxoGraphXDocType, 4 );
		
		// Position at start of file 
		int posn = 0;
		int result = SetFilePosition( refNum, posn );		// Position the mark 
		if ( result ) 
			return result;

		// Write the file prefix 
		long bytes = 4;
		result = WriteToFile( refNum, &bytes, AxoGraphFileID );	// write the prefix 
		if ( result ) 
			return result;
		
		int32_t longFormat = fileFormat;
		int32_t longColumns = numberOfColumns;
		
#ifdef __LITTLE_ENDIAN__
		ByteSwapLong( &longFormat );
		ByteSwapLong( &longColumns );
#endif
		
		bytes = sizeof( int32_t );
		result = WriteToFile( refNum, &bytes, &longFormat );
		result = WriteToFile( refNum, &bytes, &longColumns );
		
		// pass back the result ( = 0 if all went well) 
		return result;
	}
	else 
		return -1;
}



int AG_WriteColumn( const AGDataRef refNum, const int fileFormat, const int columnNumber, ColumnData *columnData )
{
	switch ( fileFormat ) 
	{
		case kAxoGraph_Graph_Format:
		{			
			// Set up header 
			ColumnHeader columnHeader;
			columnHeader.points = columnData->points;
			memcpy( &columnHeader.title, columnData->title, 80 );
			CToPascalString( columnHeader.title );
			
#ifdef __LITTLE_ENDIAN__
			ByteSwapLong( &columnHeader.points );
#endif
			
			// Write ColumnHeader  
			long bytes = sizeof( ColumnHeader );
			int result = WriteToFile( refNum, &bytes, &columnHeader );
			if ( result )
				return result;
			
#ifdef __LITTLE_ENDIAN__
			ByteSwapLong( &columnHeader.points );
			ByteSwapFloatArray( columnData->floatArray, columnHeader.points );
#endif
			
			// Write the data 
			bytes = sizeof( float ) * columnData->points;
			result = WriteToFile( refNum, &bytes, columnData->floatArray );
#ifdef __LITTLE_ENDIAN__
			ByteSwapFloatArray( columnData->floatArray, columnHeader.points );
#endif
			return result;
		}
			
		case kAxoGraph_Digitized_Format:
		{
			if ( columnNumber == 0 )
			{				
				// Set up header 
				DigitizedFirstColumnHeader columnHeader;
				columnHeader.points = columnData->points;
				memcpy( &columnHeader.title, columnData->title, 80 );
				CToPascalString( columnHeader.title );
				columnHeader.firstPoint = columnData->seriesArray.firstValue;
				columnHeader.sampleInterval = columnData->seriesArray.increment;
				
#ifdef __LITTLE_ENDIAN__
				ByteSwapLong( &columnHeader.points );
				ByteSwapFloat( &columnHeader.firstPoint );
				ByteSwapFloat( &columnHeader.sampleInterval );
#endif
				
				// Write ColumnHeader 
				long bytes = sizeof( DigitizedFirstColumnHeader );
				int result = WriteToFile( refNum, &bytes, &columnHeader );				

#ifdef __LITTLE_ENDIAN__
				ByteSwapLong( &columnHeader.points );
				ByteSwapFloat( &columnHeader.firstPoint );
				ByteSwapFloat( &columnHeader.sampleInterval );
#endif
				return result;
			}
			else
			{
				DigitizedColumnHeader columnHeader;
				
				// Set up header 
				columnHeader.points = columnData->points;
				memcpy( &columnHeader.title, columnData->title, 80 );
				CToPascalString( columnHeader.title );
				columnHeader.scalingFactor = columnData->scaledShortArray.scale;
				
#ifdef __LITTLE_ENDIAN__
				ByteSwapLong( &columnHeader.points );
				ByteSwapFloat( &columnHeader.scalingFactor );
#endif
				
				// Write ColumnHeader
				long bytes = sizeof( DigitizedColumnHeader );
				int result = WriteToFile( refNum, &bytes, &columnHeader );
				if ( result ) 
					return result;
				
#ifdef __LITTLE_ENDIAN__
				ByteSwapLong( &columnHeader.points );
				ByteSwapFloat( &columnHeader.scalingFactor );
				ByteSwapShortArray( columnData->scaledShortArray.shortArray, columnHeader.points );
#endif
				
				// Write the data 
				bytes = sizeof( short ) * columnData->points;
				result = WriteToFile( refNum, &bytes, columnData->scaledShortArray.shortArray );
#ifdef __LITTLE_ENDIAN__
				ByteSwapShortArray( columnData->scaledShortArray.shortArray, columnHeader.points );
#endif
				return result;
			}
		}
			
		case kAxoGraph_X_Format:
		{
			AxoGraphXColumnHeader columnHeader;
			
			// Load column 
			columnHeader.points = columnData->points;
			columnHeader.dataType = columnData->type;
			columnHeader.titleLength = columnData->titleLength;
			
#ifdef __LITTLE_ENDIAN__
			ByteSwapLong( &columnHeader.points );
			ByteSwapLong( &columnHeader.dataType );
			ByteSwapLong( &columnHeader.titleLength );
#endif
			
			// Write ColumnHeader
			long bytes = sizeof( AxoGraphXColumnHeader );
			int result = WriteToFile( refNum, &bytes, &columnHeader );
			if ( result )
				return result;
		
#ifdef __LITTLE_ENDIAN__
			ByteSwapLong( &columnHeader.points );
			ByteSwapLong( &columnHeader.dataType );
			ByteSwapLong( &columnHeader.titleLength );
#endif

			// Write Column title
			CStringToUnicode( columnData->title, columnData->titleLength );
			long titleLength = columnData->titleLength;
            result = WriteToFile( refNum, &titleLength, columnData->title );
			if ( result )
				return result;
		
			// Write the data 
			switch ( columnData->type )
			{
				case ShortArrayType:
				{
#ifdef __LITTLE_ENDIAN__
					ByteSwapShortArray( columnData->shortArray, columnHeader.points );
#endif
					
					bytes = sizeof( short ) * columnData->points;
					result = WriteToFile( refNum, &bytes, columnData->shortArray );
#ifdef __LITTLE_ENDIAN__
					ByteSwapShortArray( columnData->shortArray, columnHeader.points );
#endif
					return result;
				}
				case IntArrayType:
				{
#ifdef __LITTLE_ENDIAN__
					ByteSwapLongArray( (int32_t *)columnData->intArray, columnHeader.points );
#endif
					
					bytes = sizeof( int ) * columnData->points;
					result = WriteToFile( refNum, &bytes, columnData->intArray );
#ifdef __LITTLE_ENDIAN__
					ByteSwapLongArray( (int32_t *)columnData->intArray, columnHeader.points );
#endif
					return result;
				}
				case FloatArrayType:
				{
#ifdef __LITTLE_ENDIAN__
					ByteSwapFloatArray( columnData->floatArray, columnHeader.points );
#endif
					
					bytes = sizeof( float ) * columnData->points;
					result = WriteToFile( refNum, &bytes, columnData->floatArray );
#ifdef __LITTLE_ENDIAN__
					ByteSwapFloatArray( columnData->floatArray, columnHeader.points );
#endif
					return result;
				}
				case DoubleArrayType:
				{
#ifdef __LITTLE_ENDIAN__
					ByteSwapDoubleArray( columnData->doubleArray, columnHeader.points );
#endif
					
					bytes = sizeof( double ) * columnData->points;
					result = WriteToFile( refNum, &bytes, columnData->doubleArray );
#ifdef __LITTLE_ENDIAN__
					ByteSwapDoubleArray( columnData->doubleArray, columnHeader.points );
#endif
					return result;
				}
				case SeriesArrayType:
				{
					bytes = sizeof( double );
					double firstValue = columnData->seriesArray.firstValue;
					double increment = columnData->seriesArray.increment;
					
#ifdef __LITTLE_ENDIAN__
					ByteSwapDouble( &firstValue );
					ByteSwapDouble( &increment );
#endif
					
					result = WriteToFile( refNum, &bytes, &firstValue );
					result = WriteToFile( refNum, &bytes, &increment );
					return result;
				}
				case ScaledShortArrayType:
				{
					bytes = sizeof( double );
					double scale = columnData->scaledShortArray.scale;
					double offset = columnData->scaledShortArray.offset;
					
#ifdef __LITTLE_ENDIAN__
					ByteSwapDouble( &scale );
					ByteSwapDouble( &offset );
#endif
					
					result = WriteToFile( refNum, &bytes, &scale );
					result = WriteToFile( refNum, &bytes, &offset );
					
#ifdef __LITTLE_ENDIAN__
					ByteSwapShortArray( columnData->scaledShortArray.shortArray, columnHeader.points );
#endif
					
					bytes = sizeof( short ) * columnData->points;
					result = WriteToFile( refNum, &bytes, columnData->scaledShortArray.shortArray );
#ifdef __LITTLE_ENDIAN__
					ByteSwapShortArray( columnData->scaledShortArray.shortArray, columnHeader.points );
#endif
					return result;
				}
				default:
				{
					return -1;
				}
			}
		}
			
		default:
		{
			return -1;
		}
	}
}

