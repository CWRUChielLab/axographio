
// If we're running on a mac and are willing to link to the carbon framework,
// we can use Carbon's file APIs so the file type will be set correctly.  
#if defined(__APPLE__) && !defined(NO_CARBON)
#include <Carbon/Carbon.h>

#include "fileUtils.h"

// Mac-specific file access functions
// On other platforms, replace the following with equivalent functions

/* GetApplicationDirectory returns the volume reference number
and directory ID for the demo application's directory. */

OSStatus GetApplicationDirectory( short *vRefNum, long *dirID ) 
{
	ProcessSerialNumber PSN;
	ProcessInfoRec pinfo;
	FSSpec pspec;
	OSStatus err;
	
	/* valid parameters */
	if ( vRefNum == NULL || dirID == NULL ) return paramErr;
	
	/* set up process serial number */
	PSN.highLongOfPSN = 0;
	PSN.lowLongOfPSN = kCurrentProcess;
	
	/* set up info block */
	pinfo.processInfoLength = sizeof( pinfo );
	pinfo.processName = NULL;
	pinfo.processAppSpec = &pspec;
	
	/* grab the vrefnum and directory */
	err = GetProcessInformation( &PSN, &pinfo );
	if ( err == noErr ) 
	{
		*vRefNum = pspec.vRefNum;
		*dirID = pspec.parID;
	}
	
	return err;
}



int OpenFile( const char *fileName )
{
	short dataRefNum = 0;
	short vRefNum;
	long dirID;
	OSErr result;
	FSSpec spec;
	
	// get the application's directory ID
	result = GetApplicationDirectory( &vRefNum, &dirID );	
	
	if ( result != noErr ) 
	{ 
		printf( "Error from GetApplicationDirectory - result = %d", result );
		return 0;
	}
	
	// Make an FSSpec for the AxoGraph file
	Str255 macFileName;
	CopyCStringToPascal( fileName, macFileName);
	
	result = FSMakeFSSpec( vRefNum, dirID, macFileName, &spec );
	
	if ( result != noErr ) { 
		printf( "Error from FSMakeFSSpec - result = %d", result );
		return 0;
	}
	
	// open the selected file 
	result = FSpOpenDF( &spec, fsRdPerm, &dataRefNum );
	
	if ( result != noErr ) { 
		printf( "Error from FSpOpenDF - result = %d", result );
		return 0;
	}
	
	return dataRefNum;
}


void CloseFile( int dataRefNum )
{
	FSClose( dataRefNum );
}



int NewFile( const char *fileName )
{
	short dataRefNum = 0;
	short vRefNum;
	long dirID;
	OSErr result;
	FSSpec spec;
	
	// check if the file name ends in 'x'
	// if it does, create an AxoGraph X file
	const char *character = fileName;
	char previous = 0;
	
	while ( *character )
	{
		previous = *character;
		character++;
	}
	
	bool AxoGraphXFormat = ( previous == 'x' );
	
	// get the application's directory ID
	result = GetApplicationDirectory( &vRefNum, &dirID );	
	
	if ( result != noErr ) { 
		printf( "Error from GetApplicationDirectory - result = %d", result );
		return 0;
	}
	
	// Make an FSSpec for the AxoGraph file
	Str255 macFileName;
	CopyCStringToPascal( fileName, macFileName);
	
	result = FSMakeFSSpec( vRefNum, dirID, macFileName, &spec );
	
	if ( result == fnfErr ) { 
		
		// create the new file 
		if ( AxoGraphXFormat )
			result = FSpCreate( &spec, kAGX_Creator, kAGX_DocType, smSystemScript );
		else
			result = FSpCreate( &spec, kAG_Creator, kAG_DocType, smSystemScript );
		
		if ( result != noErr ) { 
			printf( "Error from FSpCreate - result = %d", result );
			return 0;
		}
	}
	else
	{
		if ( result != noErr ) { 
			printf( "Error from FSMakeFSSpec - result = %d", result );
			return 0;
		}
	}
	
	result = FSpOpenDF( &spec, fsRdWrPerm, &dataRefNum );
	
	if ( result != noErr ) { 
		printf( "Error from FSpOpenDF - result = %d", result );
		return 0;
	}
	
	return dataRefNum;
}


int SetFilePosition( int dataRefNum, int posn )
{
	return SetFPos( dataRefNum, fsFromStart, posn );		// Position the mark 
}


int ReadFromFile( int dataRefNum, long *count, void *dataToRead )
{
	return FSRead( dataRefNum, count, dataToRead );
}

int WriteToFile( int dataRefNum, long *count, void *dataToWrite )
{
	return FSWrite( dataRefNum, count, dataToWrite );
}




// If we're not running on a mac or can't link to Carbon, we can
// usually depend on the standard C file I/O.  If you don't have
// this on your platform, you will need to write your own version
// of these functions.  
#else 


#include "fileUtils.h"
#include <cstdio>

AGDataRef OpenFile( const char *fileName )
{
	return fopen(fileName, "rb");
}

void CloseFile( AGDataRef dataRefNum )
{
	fclose((FILE*)(dataRefNum));
}

AGDataRef NewFile( const char *fileName )
{
	return fopen(fileName, "wb+");
}

int SetFilePosition( AGDataRef dataRefNum, int posn )
{
	return fseek((FILE*)(dataRefNum), posn, SEEK_SET);
}

int ReadFromFile( AGDataRef dataRefNum, long *count, void *dataToRead )
{
	long goal = *count;

	(*count) = (long)fread(dataToRead, 1, *count, (FILE*)(dataRefNum)); 
	return *count != goal;
}

int WriteToFile( AGDataRef dataRefNum, long *count, void *dataToWrite )
{
	long goal = *count;

	(*count) = (long)fwrite(dataToWrite, 1, *count, (FILE*)(dataRefNum));
	return *count != goal;
}

#endif
