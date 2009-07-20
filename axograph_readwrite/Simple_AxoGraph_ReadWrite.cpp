/* ----------------------------------------------------------------------------------

Overview
--------

	This example program demonstrates how to use the AxoGraph_ReadWrite
	functions for reading and writing AxoGraph binary data files. 
	The program and support files are in the public domain, meaning
	they can be modified and used for any purpose. 
	
	Three different types of AxoGraph file are imported.
	For each file, every data column is multiplied by 2, 
	and results are saved in a new file.
	
    For this program to work correctly, three files 
	must be present in the same directory as the program. 
	The filese are named...

        AxoGraph Graph File
        AxoGraph Digitized File
        AxoGraph X File.axgx

	The import functions support three different AxoGraph file formats...
	AxoGraph (version 1-4) graph file
	AxoGraph (version 4) digitized file
	AxoGraph X graph/digitized file

	The program detects what type of file has been read,
	but uses a standard AxoGraph graph file for the subsequent export. 
	Each imported data column is converted to 4 byte floating point array. 
	Some precision may be lost in the conversion, and acquired data files
	may more than double in size. But standardizing on float arrays 
	may simplify the design of AxoGraph file import code. 

	For a more sophisticated example, see the program "Demo_AxoGraph_ReadWrite".
	This program preserves the format of the data columns and the file type. 

	Once a file is open, it can be read in with calls to only three functions. 
	Here is a pseudo-code example outlining AxoGraph file import...

	dataRefNum = OpenFile( fileName );
	result = AG_GetFileFormat( dataRefNum, &fileFormat )
	result = AG_GetNColumns( dataRefNum, fileFormat, &numberOfColumns )
	for ( long columnNumber=0; columnNumber<numberOfColumns; columnNumber++ )
    {
		result = AG_ReadFloatColumn( dataRefNum, fileFormat, columnNumber, &floatColumn );
    }

	The "floatColumn" variable has a relatively simple structure. 
    Integer, double or sequence column data is converted to a float array
    before being returned as part of the floatColumn variable. 

    The detailed description of AxoGraph file structure is supplied in
    the header file "AxoGraph_ReadWrite.h".

---------------------------------------------------------------------------------

	Notes on cross-platform porting
    -------------------------------

	All the Mac OS specific code is located in "fileUtils.cpp".
	This code creates, opens and closes files. It works under OS 9 and OS X. 
    To port this example program to Windows, or other platforms, the
	file manipulation code in "fileUtils.cpp" will need to be replaced.

    Computers that use little-endian math (Intel) will need to byte-swap
    numeric data values as they are read in. This will require modifications 
    to the code in "AxoGraph_ReadWrite.cpp"

    Note that where necessary, 'long' data types have been used 
	in preference to 'int', to minimize the possibility for size conflict 
	on different platforms, and with different compilers.
    
	This code is in the public domain. 

---------------------------------------------------------------------------------- */

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
			printf( "\n\nError: Could not find a file named '%s'.", fileName );
			printf( "\nCopy an AxoGraph file with this name into the same folder as the application.");
			continue;
		}
		
		printf( "\n\nOpened file: %s", fileName );
		
		// check the AxoGraph header, and get the number of columns to be read 	
		int fileFormat = 0;
		int result = AG_GetFileFormat( dataRefNum, &fileFormat );
		if ( result ) 
		{ 
			printf( "\nError from AG_GetFileFormat - ");
			if ( result == kAG_FormatErr )
				printf( "file is not in AxoGraph format" );
			else if ( result == kAG_VersionErr )
				printf( "file is of a more recent version than supported by this code" );
			else
				printf( "error number %d", result );
			
			CloseFile( dataRefNum );
			return result;
		}
		
		int32_t numberOfColumns = 0;
		result = AG_GetNumberOfColumns( dataRefNum, fileFormat, &numberOfColumns );
		if ( result ) 
		{ 
			printf( "Error from AG_GetNumberOfColumns - error number %d", result );
			CloseFile( dataRefNum );
			return result;
		}
		
		// Sanity check
		if ( numberOfColumns <= 0 )  	// negative columns 
		{
			printf ( "File format error: number of columns is set negative in AxoGraph data file" );
			CloseFile( dataRefNum );
			return kAG_FormatErr;
		}
		
		//  Create an array of column data structures to receive the data columns
		
		ColumnData* columnArray = new ColumnData[numberOfColumns];
		
		//	AG_ReadFloatColumn reads column data into a float column structure.
		
		for ( int columnNumber=0; columnNumber<numberOfColumns; columnNumber++ )
		{	
			ColumnData column;
			result = AG_ReadFloatColumn( dataRefNum, fileFormat, columnNumber, &column );
			
			if ( result ) 
			{ 
				printf( "Error from AG_ReadFloatColumn - error number %d", result );
				CloseFile( dataRefNum );
				return result;
			}
			
			printf( "\nColumn #%d: %s", columnNumber, column.title );
			
			columnArray[columnNumber] = column;
		}

		// Close the import file
		CloseFile( dataRefNum );
		
		
		// To make things interesting, multiply all x- and y-columns by two 
		for ( long columnNumber=0;  columnNumber<numberOfColumns; columnNumber++ )
		{
			float *array = columnArray[columnNumber].floatArray;
			for ( long i=0; i<columnArray[columnNumber].points; i++ )
			{
				array[i] = array[i] * 2;
			}
		}
		
		// =====================================================================================================================
		//
		// Create a new file and write out the scaled data
		//
		// =====================================================================================================================
		
		const char *outputFileName;
		
		// Select file name for this pass
		if ( pass == 1 ) outputFileName = "AxoGraph Graph Simple";
		if ( pass == 2 ) outputFileName = "AxoGraph Digitized Simple";
		if ( pass == 3 ) outputFileName = "AxoGraph X Simple";
		
		// create the new file 
		dataRefNum = NewFile( outputFileName );
		
		if ( dataRefNum == 0 ) 
		{
			printf( "\n\nError: Could not create a new file named '%s'.", outputFileName );
			printf( "\nMake sure that you have write permission for the current directory.");
			continue;
		}
		
		// Write the processed data out to the new AxoGraph graph file 
		fileFormat = kAxoGraph_Graph_Format;
		result = AG_WriteHeader( dataRefNum, fileFormat, numberOfColumns );
		
		if ( result ) { 
			printf( "Error from AG_WriteHeader - error number %d", result );
			CloseFile( dataRefNum );	
			return result;
		}
		
		// Write each column to the new file 
		for ( long columnNumber=0;  columnNumber<numberOfColumns; columnNumber++ )
		{
			result = AG_WriteColumn( dataRefNum, fileFormat, columnNumber, &columnArray[columnNumber] );
			
			if ( result ) { 
				printf( "Error from AG_WriteColumn - error number %d", result );
				CloseFile( dataRefNum );	
				return result;
			}
		}

		// Close the file
		CloseFile( dataRefNum );	

		// Free the array
		delete columnArray;
	}
	
	return 0;
}




