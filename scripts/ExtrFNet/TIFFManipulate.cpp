#include <string.h>
#include <stdarg.h>

#include "TIFFManipulate.h"

uint32		tiffWidth, tiffLength;

/*!
Initializes the object setting some default values.

*/
TIFFManipulate::TIFFManipulate()
{
	fileOpened = false;
	printErrors = true;
	printWarnings = true;

	SetPlanarConfig( (uint16) -1 );
	SetFillOrder( 0 );
	SetRowsPerStrip( (uint32) -1 );
	SetOutTiled( -1 );
	SetTileWidth( (uint32) -1 );
	SetTileLength( (uint32) -1 );
	SetCompressionOptions(NULL);
}

/*!
\param name file name

\param mode mode in which file is to be opened.  See TIFFManipulate::Open for a description
of possible parameters.

Initialize the object and open a TIFF file.
*/

TIFFManipulate::TIFFManipulate( const char* name, const char* mode )
{
	TIFFManipulate();

	Open( name, mode );
}


/*
	Destructor
*/
TIFFManipulate::~TIFFManipulate()
{
	if (fileOpened) {
		Close();
	}
}

/*!
Saves the current error handlers and then sets the error handlers to two
statically defined functions of this class.

\note This function is used internally and should not be called directly.

*/
void
TIFFManipulate::setHandlers()
{
	pointerToObject = this;
	oldErrorHandler = TIFFSetErrorHandler( TIFFManipulate::ErrorHandlerWrapper );
	oldWarningHandler = TIFFSetWarningHandler( TIFFManipulate::WarningHandlerWrapper );
}


/*!
Restores tifflib error handlers to their saved values.

\note This function is used internally and should not be called directly.

*/
void
TIFFManipulate::restoreHandlers()
{
	TIFFSetErrorHandler( oldErrorHandler );
	TIFFSetWarningHandler( oldWarningHandler );
}


/*!
Error handler wrapper which routes tifflib error callbacks to the current object.

\note This function is used internally and should not be called directly.

*/
void
TIFFManipulate::ErrorHandlerWrapper( const char* module, const char* fmt, va_list ap )
{
    // explicitly cast global variable <pointerToObject> to a pointer to TIFFManipulate
    // warning: <pointerToObject> MUST point to an appropriate object!
    TIFFManipulate* mySelf = pointerToObject;

    // call member
    mySelf->ErrorHandler(module, fmt, ap);
}


/*!
Warning handler wrapper which routes tifflib warning callbacks to the current object.

\note This function is used internally and should not be called directly.

*/
void
TIFFManipulate::WarningHandlerWrapper( const char* module, const char* fmt, va_list ap )
{
    // explicitly cast global variable <pointerToObject> to a pointer to TIFFManipulate
    // warning: <pointerToObject> MUST point to an appropriate object!
    TIFFManipulate* mySelf = pointerToObject;

    // call member
    mySelf->WarningHandler(module, fmt, ap);
}


/*!
Error handler for the class which receives error callbacks from
::ErrorHandlerWrapper.  Its default behavior is to call PrintError the error
information.

\note This function is used internally and should not be called directly.

*/
void
TIFFManipulate::ErrorHandler( const char* module, const char* fmt, va_list ap )
{
	PrintError( module, fmt, ap );
}


/*!
Warning handler for the class which receives warning callbacks from 
::WarningHandlerWrapper.  Its default behavior is to call PrintWarning with the
warning information.

\note This function is used internally and should not be called directly.

*/
void
TIFFManipulate::WarningHandler( const char* module, const char* fmt, va_list ap )
{
	PrintWarning( module, fmt, ap );
}



void
TIFFManipulate::PrintError( const char* module, const char* fmt, ... )
{
	va_list ap;
	va_start(ap,fmt); /* point to first element after fmt*/

	if (printErrors) {
		if (module != NULL)
			fprintf(stderr, "%s: ", module);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, ".\n");
	}
}


void
TIFFManipulate::PrintWarning( const char* module, const char* fmt, ... )
{
	va_list ap;
	va_start(ap,fmt); /* point to first element after fmt*/

	if (printWarnings) {
		if (module != NULL)
			fprintf(stderr, "%s: ", module);
		fprintf(stderr, "Warning, ");
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, ".\n");
	}
}


/*
*
*	Calls which mimic libtiff calls
*
*/

/*!

\note See TIFFOpen()

*/
int
TIFFManipulate::Open( const char* name, const char* mode )
{
	if (fileOpened) {
		Close();
	}

	setHandlers();
	tiffHandle = TIFFOpen( name, mode );
	restoreHandlers();

	if (tiffHandle) {
		fileOpened = 1;
	} else {
		fileOpened = 0;
	}

	return fileOpened;
}


/*!

\note See TIFFClose()

*/
void
TIFFManipulate::Close()
{
	setHandlers();
	TIFFClose( tiffHandle );
	restoreHandlers();

	fileOpened = 0;
}


/*!

\note See TIFFReadDirectory()

*/
int
TIFFManipulate::ReadDirectory()
{
	setHandlers();
	int result = TIFFReadDirectory( tiffHandle );
	restoreHandlers();

	return result;
}


/*!

\note See TIFFCurrentDirectory()

*/
tdir_t
TIFFManipulate::CurrentDirectory()
{
	setHandlers();
	tdir_t result = TIFFCurrentDirectory( tiffHandle );
	restoreHandlers();

	return result;
}


/*!

\note See TIFFPrintDirectory()

*/
void
TIFFManipulate::PrintDirectory(FILE* fd, long flags )
{
	setHandlers();
	TIFFPrintDirectory( tiffHandle, fd, flags );
	restoreHandlers();
}


/*!

\note See TIFFSetDirectory()

*/
int
TIFFManipulate::SetDirectory( tdir_t dirNum )
{
	setHandlers();
	int result = TIFFSetDirectory( tiffHandle, dirNum );
	restoreHandlers();

	return result;
}

int
TIFFManipulate::LastDirectory()
{
	setHandlers();
	int result = TIFFLastDirectory( tiffHandle );
	restoreHandlers();
	
	return result;
}

int
TIFFManipulate::SetSubDirectory( uint32 dirOff )
{
	return TIFFSetSubDirectory( tiffHandle, dirOff );
}


int
TIFFManipulate::WriteDirectory()
{
	return TIFFWriteDirectory( tiffHandle );
}


TIFF*
TIFFManipulate::GetTIFFHandle()
{
	return tiffHandle;
}


const char*
TIFFManipulate::GetFileName()
{
	setHandlers();

	const char* filename = TIFFFileName( tiffHandle );

	restoreHandlers();

	return filename;
}


int
TIFFManipulate::GetTagOrientation()
{
	uint16* orientation;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_ORIENTATION, orientation );
	restoreHandlers();

	if (result) {
		return *orientation;
	}
	return -1;
}


double
TIFFManipulate::GetTagImageLength()
{
	uint32* length;
	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_IMAGELENGTH, length );
	restoreHandlers();

	if (result) {
		return *length;
	}

	return -1;
}


double
TIFFManipulate::GetTagImageWidth()
{
	uint32* width;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_IMAGEWIDTH, width );
	restoreHandlers();

	if (result) {
		return *width;
	}

	return -1;
}

uint32 TIFFManipulate::TifWidth()
{
	return tiffWidth;

}

uint32 TIFFManipulate::TifLength()
{
	return tiffLength;

}


double
TIFFManipulate::GetTagImageDepth()
{
	uint32* depth;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_IMAGEDEPTH, depth );
	restoreHandlers();

	if (result) {
		return *depth;
	}

	return -1;
}


double
TIFFManipulate::GetTagResolutionUnit()
{
	uint16* unit;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_RESOLUTIONUNIT, unit );
	restoreHandlers();

	if (result) {
		return *unit;
	}

	return -1;
}


double
TIFFManipulate::GetTagCompression()
{
	uint16* var;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_COMPRESSION, var );
	restoreHandlers();

	if (result) {
		return *var;
	}

	return -1;
}


double
TIFFManipulate::GetTagGroup3Options()
{
	uint32* var;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_GROUP3OPTIONS, var );
	restoreHandlers();

	if (result) {
		return *var;
	}

	return -1;
}


double
TIFFManipulate::GetTagGroup4Options()
{
	uint32* var;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_GROUP4OPTIONS, var );
	restoreHandlers();

	if (result) {
		return *var;
	}

	return -1;
}


char*
TIFFManipulate::GetTagSoftware()
{
	char** var;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_SOFTWARE, var );
	restoreHandlers();

	if (result) {
		return *var;
	}

	*var = "undefined";
	return *var;
}


float
TIFFManipulate::GetTagXResolution()
{
	float* var;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_XRESOLUTION, var );
	restoreHandlers();

	if (result) {
		return *var;
	}

	return -1;
}


float
TIFFManipulate::GetTagYResolution()
{
	float* var;

	setHandlers();
	int result = TIFFGetField( tiffHandle, TIFFTAG_YRESOLUTION, var );
	restoreHandlers();

	if (result) {
		return *var;
	}

	return -1;
}


extern "C" {
	int
	tiffcp_interface( TIFF*, TIFF*, char*, uint16, uint16, uint32, int, uint32, uint32 );
}

extern "C" {
	uint32 GettiffWidth();
}

extern "C" {
	uint32 GettiffLength();
}

int
TIFFManipulate::AppendDirectory( TIFFManipulate* src )
{
	if (0) {	// Check that file opened in append mode
		return 0;
	}

	setHandlers();
	
	int result =
		tiffcp_interface(
			src->GetTIFFHandle(),
			tiffHandle,
			tiffcpCompressionOptions,
			tiffcpConfig,
			tiffcpFillOrder,
			tiffcpRowsPerStrip,
			tiffcpOutTiled,
			tiffcpTileWidth,
			tiffcpTileLength
		);

	tiffWidth = GettiffWidth();
	tiffLength = GettiffLength();

	restoreHandlers();
	
	if (!result) {
		return 0;
	}

	return 1;
}

void
TIFFManipulate::SetOutTiled( int i )
{
	tiffcpOutTiled = i;
}


void
TIFFManipulate::SetPlanarConfig( uint16 i )
{
	tiffcpConfig = i;
}


void
TIFFManipulate::SetCompressionOptions( char* opts )
{
	tiffcpCompressionOptions = opts;
}


void
TIFFManipulate::SetFillOrder( uint16 i )
{
	tiffcpFillOrder = i;
}


void
TIFFManipulate::SetRowsPerStrip( uint32 i )
{
	tiffcpRowsPerStrip = i;
}


void
TIFFManipulate::SetTileWidth( uint32 i )
{
	tiffcpTileWidth = i;
}


void
TIFFManipulate::SetTileLength( uint32 i )
{
	tiffcpTileLength = i;
}


tdir_t
TIFFManipulate::GetLastDirectoryNumber()
{
	tdir_t dirno_save = CurrentDirectory();
	while (!LastDirectory()) {
		ReadDirectory();
	}

	tdir_t lastDir = CurrentDirectory();

	SetDirectory( dirno_save );

	return lastDir;
}

