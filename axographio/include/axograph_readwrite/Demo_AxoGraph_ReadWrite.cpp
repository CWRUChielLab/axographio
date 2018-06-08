//	----------------------------------------------------------------------------------
//
//	Overview
//	--------
//
//	This example program demonstrates how to use the AxoGraph_ReadWrite
//	functions for reading and writing AxoGraph binary data files. 
//	The program and support files are in the public domain, meaning
//	they can be used and modified for any purpose. 
//	
//	Three different types of AxoGraph file are imported.
//	For each file, every data column is multiplied by 2, 
//	and results are saved in a new file.
//	
//    For this program to work correctly, three files must 
//	be present in the same directory as the program. 
//	The filese are named...
//
//        AxoGraph Graph File
//        AxoGraph Digitized File
//        AxoGraph X File.axgx
//	
//
//	The import functions support three different AxoGraph file formats...
//
//		AxoGraph (version 1-4)	graph file
//		AxoGraph (version 4)	digitized file
//		AxoGraph X				graph / digitized file
//
//	The program detects what type of file has been read,
//	and uses the same type for the subsequent export. 
//	It also preserves the format of the data columns.
//	This is relatively complex due to the variety of data column
//	formats supported. 
//	
//	For a simpler example, see the program "Simple_AxoGraph_ReadWrite". 
//	In this program, each imported data column is converted 
//	to 4 byte floating point array. Some precision may be lost 
//	in the conversion, and acquired data files may more than 
//	double in size. But standardizing on float arrays 
//	may simplify the design of AxoGraph file import code. 
//
//	Once a file is open, it can be read in with calls to only three functions. 
//	Here is an example outlining AxoGraph file import...
//
//	dataRefNum = OpenFile( fileName );
//	result = AG_GetFileFormat( dataRefNum, &fileFormat )
//	result = AG_GetNColumns( dataRefNum, fileFormat, &numberOfColumns )
//	for ( long columnNumber=0; columnNumber<numberOfColumns; columnNumber++ )
//  {
//		result = AG_ReadColumn( dataRefNum, fileFormat, columnNumber, &column );
//  }
//
//	The "column" variable has a complex structure. It is 'union' that can take
//	six different forms depending on the type of data that is read in. 
//	
//    The detailed description of AxoGraph file structure is supplied in
//    the header file "AxoGraph_ReadWrite.h".
//
//	---------------------------------------------------------------------------------
//
//	Notes on cross-platform porting
//    -------------------------------
//
//	All the Mac OS specific code is located in "fileUtils.cpp".
//	This code creates, opens and closes files. It works under OS 9 and OS X. 
//  To port this example program to Windows, or other platforms, the
//	file manipulation code in "fileUtils.cpp" will need to be replaced.
//
//    Computers that use little-endian math (Intel) will need to byte-swap
//    numeric data values as they are read in. This will require modifications 
//    to the code in "AxoGraph_ReadWrite.cpp"
//
//	Note that where necessary, 'long' data types have been used 
//	in preference to 'int', to minimize the possibility for integer 
//	size conflict on different platforms, and with different compilers.
//
//	----------------------------------------------------------------------------------

#include <stdio.h>

#include "fileUtils.h"
#include "AxoGraph_ReadWrite.h"

int main()
{
	// Perform three passes. Read in a different type of AxoGraph file during each pass.
	
	for ( int pass=1; pass<=3; pass++ )
	{
		// =====================================================================================================================
		//
		// Open an AxoGraph file and read in the data
		//
		// =====================================================================================================================
		
		const char *fileName;
		
		// Select file name for this pass
		if ( pass == 1 ) fileName = "AxoGraph Graph File";
		if ( pass == 2 ) fileName = "AxoGraph Digitized File";
		if ( pass == 3 ) fileName = "AxoGraph X File.axgx";
		
		// Open the example file
		AGDataRef dataRefNum = OpenFile( fileName ); 
		if ( dataRefNum == 0 ) 
		{
			printf( "\nError: Could not find a file named '%s'.\n", fileName );
			printf( "Copy an AxoGraph file with this name into the same folder as the application.\n");
			continue;
		}
		
		printf( "\nOpened file: %s\n", fileName );
		
		// check the AxoGraph header, and get the number of columns to be read 	
		int fileFormat = 0;
		int result = AG_GetFileFormat( dataRefNum, &fileFormat );
		if ( result ) 
		{ 
			printf( "\nError from AG_GetFileFormat - ");
			
			if ( result == kAG_FormatErr )
			{
				printf( "file is not in AxoGraph format\n" );
			}
			else if ( result == kAG_VersionErr )
			{
				printf( "file is of a more recent version than supported by this code\n" );
			}
			else
			{
				printf( "error number %d\n", result );
			}
			CloseFile( dataRefNum );
			continue;
		}
		
		int32_t numberOfColumns = 0;
		result = AG_GetNumberOfColumns( dataRefNum, fileFormat, &numberOfColumns );
		if ( result ) 
		{ 
			printf( "\nError from AG_GetNumberOfColumns - error number %d\n", result );
			CloseFile( dataRefNum );
			continue;
		}
		
		// Sanity check
		if ( numberOfColumns <= 0 )  	// negative columns 
		{
			printf ( "\nFile format error: number of columns is set negative in AxoGraph data file\n" );
			CloseFile( dataRefNum );
			continue;
		}
		
		//  Create an array of column data structures to receive the data columns
		ColumnData* columnArray = new ColumnData[numberOfColumns];
		
		//	AG_ReadColumn reads column data into a column structure
		for ( int columnNumber=0; columnNumber<numberOfColumns; columnNumber++ )
		{	
			ColumnData column;
			result = AG_ReadColumn( dataRefNum, fileFormat, columnNumber, &column );
			
			if ( result ) 
			{ 
				printf( "\nError from AG_ReadColumn - error number %d\n", result );
				CloseFile( dataRefNum );
				continue;
			}
			
			printf( "Column #%d: %s\n", columnNumber, column.title );
			
			columnArray[columnNumber] = column;
		}

		// Close the import file
		CloseFile( dataRefNum );
		
		
		// To make things interesting, multiply all x- and y-columns by two 
		for ( long columnNumber=0;  columnNumber<numberOfColumns; columnNumber++ )
		{
			switch ( columnArray[columnNumber].type )
			{
				case ShortArrayType:
				{
					short *array = columnArray[columnNumber].shortArray;
					for ( long i=0; i<columnArray[columnNumber].points; i++ )
					{
						array[i] = array[i] * 2;
					}
					break;
				}
				case IntArrayType:
				{
					int *array = columnArray[columnNumber].intArray;
					for ( long i=0; i<columnArray[columnNumber].points; i++ )
					{
						array[i] = array[i] * 2;
					}
					break;
				}
				case FloatArrayType:
				{
					float *array = columnArray[columnNumber].floatArray;
					for ( long i=0; i<columnArray[columnNumber].points; i++ )
					{
						array[i] = array[i] * 2;
					}
					break;
				}
				case DoubleArrayType:
				{
					double *array = columnArray[columnNumber].doubleArray;
					for ( long i=0; i<columnArray[columnNumber].points; i++ )
					{
						array[i] = array[i] * 2;
					}
					break;
				}
				case SeriesArrayType:
				{
					columnArray[columnNumber].seriesArray.firstValue = columnArray[columnNumber].seriesArray.firstValue * 2;
					columnArray[columnNumber].seriesArray.increment = columnArray[columnNumber].seriesArray.increment * 2;
					break;
				}
				case ScaledShortArrayType:
				{
					columnArray[columnNumber].scaledShortArray.scale = columnArray[columnNumber].scaledShortArray.scale * 2;
					columnArray[columnNumber].scaledShortArray.offset = columnArray[columnNumber].scaledShortArray.offset * 2;
					break;
				}
				default:
				{
					printf( "\nError: unknown column type %d\n", columnArray[columnNumber].type );
					break;
				}
			}
		}
		
		// =====================================================================================================================
		//
		// Create a new file and write out the scaled data
		//
		// =====================================================================================================================
		
		const char *outputFileName;
		
		// Select file name for this pass
		if ( pass == 1 ) outputFileName = "AxoGraph Graph Result";
		if ( pass == 2 ) outputFileName = "AxoGraph Digitized Result";
		if ( pass == 3 ) outputFileName = "AxoGraph X Result.axgx";
		
		// create the new file 
		dataRefNum = NewFile( outputFileName );
		
		if ( dataRefNum == 0 ) 
		{
			printf( "\nError: Could not create a new file named '%s'.\n", outputFileName );
			printf( "Make sure that you have write permission for the current directory.\n");
			continue;
		}
		
		// Write the re-scaled data out to the new file 
		result = AG_WriteHeader( dataRefNum, fileFormat, numberOfColumns );
		
		if ( result ) 
		{ 
			printf( "\nError from AG_WriteHeader - error number %d\n", result );
			CloseFile( dataRefNum );	
			continue;
		}
		
		// Write each column to the new file 
		for ( long columnNumber=0;  columnNumber<numberOfColumns; columnNumber++ )
		{
			result = AG_WriteColumn( dataRefNum, fileFormat, columnNumber, &columnArray[columnNumber] );
			
			if ( result ) 
			{ 
				printf( "\nError from AG_WriteColumn - error number %d\n", result );
				CloseFile( dataRefNum );	
				continue;
			}
		}

		// Close the file
		CloseFile( dataRefNum );	
	
		// Free the array
		delete columnArray;
	}
	
	return 0;
}




