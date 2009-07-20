
#ifndef FILEUTILS_H
#define FILEUTILS_H

#define kAG_Creator  'AxG2'
#define kAG_DocType  'AxGr'

#define kAGX_Creator 'AxGX'
#define kAGX_DocType  'axgx'

// On the mac, Carbon uses ints to keep track of open files
// If you can't/don't want to link to Carbon (e.g. for a web service that's
// not running as root), uncomment the following line:
// #define NO_CARBON
#if defined(__APPLE__) && !defined(NO_CARBON)
typedef int AGDataRef;
#else
// on other systems, we'll use the FILE* defined in the C standard library
// (we cast it to a void* here to avoid loading stdlib)
typedef void* AGDataRef;
#endif

AGDataRef OpenFile( const char *fileName );
void CloseFile( AGDataRef dataRefNum );
AGDataRef NewFile( const char *fileName );

int SetFilePosition( AGDataRef dataRefNum, int posn );
int ReadFromFile( AGDataRef dataRefNum, long *count, void *dataToRead );
int WriteToFile( AGDataRef dataRefNum, long *count, void *dataToWrite );

#endif
