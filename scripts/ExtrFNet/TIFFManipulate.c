/*                                                                        *
 * Programa: TIFFManipulate.c                                             *
 *                                                                        *
 * Objetivo: Gerarar un programa para extraer las imágenes de FileNet que *
 *           son califadas para manejo  y conversion a OnDemand.		  *
 *																		  *
 * Fecha de Creación : Abr/2005								              *
 *																		  *	
 * Autor  : Andrés Ventura G.                                             *
 ***************************************************************>>avg<<***/

#include <string.h>
#include <stdarg.h>

#include "TIFFManipulate.h"


uint32		tiffWidth, tiffLength;


#define	true	1
#define	false	0
#define	this	0



void TIFFManipulateIni(TIFFManipulate* src) {
	src->fileOpened = false;
	printErrors = true;
	printWarnings = true;
	
	SetPlanarConfig( (uint16) -1 , src );	
	SetFillOrder( 0, src );
	SetRowsPerStrip( (uint32) -1, src );
	SetOutTiled( -1, src );
	SetTileWidth( (uint32) -1, src );
	SetTileLength( (uint32) -1, src );
	SetCompressionOptions(NULL, src);
}


/*!
Saves the current error handlers and then sets the error handlers to two
statically defined functions of this class.

\note This function is used internally and should not be called directly.

*/
void
setHandlers(TIFFManipulate* src)
{
	pointerToObject = this;
	src->oldErrorHandler = TIFFSetErrorHandler( ErrorHandlerWrapper );
	src->oldWarningHandler = TIFFSetWarningHandler( WarningHandlerWrapper );
}


/*!
Restores tifflib error handlers to their saved values.

\note This function is used internally and should not be called directly.

*/
void restoreHandlers(TIFFManipulate* src)
{
	TIFFSetErrorHandler( src->oldErrorHandler );
	TIFFSetWarningHandler( src->oldWarningHandler );
}


/*!
Error handler wrapper which routes tifflib error callbacks to the current object.

\note This function is used internally and should not be called directly.

*/
void ErrorHandlerWrapper(char* module, char* fmt, va_list ap )
{
    /*// explicitly cast global variable <pointerToObject> to a pointer to TIFFManipulate
    // warning: <pointerToObject> MUST point to an appropriate object!
    //TIFFManipulate* mySelf = pointerToObject;

    // call member
    //mySelf->ErrorHandler(module, fmt, ap);*/
}


/*!
Warning handler wrapper which routes tifflib warning callbacks to the current object.

\note This function is used internally and should not be called directly.

*/
void WarningHandlerWrapper( char* module, char* fmt, va_list ap )
{
    /*// explicitly cast global variable <pointerToObject> to a pointer to TIFFManipulate
    // warning: <pointerToObject> MUST point to an appropriate object!
    //TIFFManipulate* mySelf = pointerToObject;

    // call member
    //mySelf->WarningHandler(module, fmt, ap);*/
}


/*!
Error handler for the class which receives error callbacks from
::ErrorHandlerWrapper.  Its default behavior is to call PrintError the error
information.

\note This function is used internally and should not be called directly.

*/
void ErrorHandler( const char* module, const char* fmt, va_list ap )
{
	PrintError( module, fmt, ap );
}


/*!
Warning handler for the class which receives warning callbacks from 
::WarningHandlerWrapper.  Its default behavior is to call PrintWarning with the
warning information.

\note This function is used internally and should not be called directly.

*/
void WarningHandler( const char* module, const char* fmt, va_list ap )
{
	PrintWarning( module, fmt, ap );
}


void PrintError( const char* module, const char* fmt, ... )
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


void PrintWarning( const char* module, const char* fmt, ... )
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
int Open(TIFFManipulate* src, const char* name, const char* mode )
{
	if (src->fileOpened) {
		Close(src);
	}

	setHandlers(src);
	src->tiffHandle = TIFFOpen( name, mode );
	restoreHandlers(src);

	if (src->tiffHandle) {
		src->fileOpened = 1;
	} else {
		src->fileOpened = 0;
	}

	return src->fileOpened;
}


/*!

\note See TIFFClose()

*/
void Close(TIFFManipulate* src)
{
	setHandlers(src);
	TIFFClose( src->tiffHandle );
	restoreHandlers(src);
	src->fileOpened = 0;
}


/*!

\note See TIFFReadDirectory()

*/
int ReadDirectory(TIFFManipulate* src)
{
	int result;
	setHandlers(src);
	result= TIFFReadDirectory( src->tiffHandle );
	restoreHandlers(src);

	return result;
}


/*!

\note See TIFFCurrentDirectory()

*/
tdir_t CurrentDirectory(TIFFManipulate* src)
{
	tdir_t result;
	setHandlers(src);	
	result= TIFFCurrentDirectory( src->tiffHandle );
	restoreHandlers(src);

	return result;
}


/*!

\note See TIFFPrintDirectory()

*/
void PrintDirectory(FILE* fd, long flags, TIFFManipulate* src )
{
	setHandlers(src);
	TIFFPrintDirectory( src->tiffHandle, fd, flags );
	restoreHandlers(src);
}


/*!

\note See TIFFSetDirectory()

*/
int SetDirectory( tdir_t dirNum, TIFFManipulate* src )
{
	int result;
	setHandlers(src);
	result = TIFFSetDirectory( src->tiffHandle, dirNum );
	restoreHandlers(src);

	return result;
}

int LastDirectory(TIFFManipulate* src)
{
	int result;
	setHandlers(src);
	result = TIFFLastDirectory( src->tiffHandle );
	restoreHandlers(src);
	
	return result;
}

int SetSubDirectory( uint32 dirOff, TIFFManipulate* src )
{
	return TIFFSetSubDirectory( src->tiffHandle, dirOff );
}


int WriteDirectory(TIFFManipulate* src)
{
	return TIFFWriteDirectory( src->tiffHandle );
}


TIFF* GetTIFFHandle(TIFFManipulate* src)
{
	return src->tiffHandle;
}


const char* GetFileName(TIFFManipulate* src)
{
	const char* filename;
	setHandlers(src);

	filename = TIFFFileName( src->tiffHandle );

	restoreHandlers(src);

	return filename;
}


int GetTagOrientation(TIFFManipulate* src)
{
	uint16* orientation;
	int result;

	orientation =0;

	setHandlers(src);
	result = TIFFGetField( src->tiffHandle, TIFFTAG_ORIENTATION, orientation );
	restoreHandlers(src);

	if (result) {
		return *orientation;
	}
	return -1;
}


double GetTagImageLength(TIFFManipulate* src)
{
	int result;
	uint32* length;
	
	length =0;

	setHandlers(src);
	result = TIFFGetField( src->tiffHandle, TIFFTAG_IMAGELENGTH, length );
	restoreHandlers(src);

	if (result) {
		return *length;
	}

	return -1;
}


double GetTagImageWidth(TIFFManipulate* src)
{
	uint32* width;
	int result;
	setHandlers(src);

	width =0;

	result = TIFFGetField( src->tiffHandle, TIFFTAG_IMAGEWIDTH, width );
	restoreHandlers(src);

	if (result) {
		return *width;
	}

	return -1;
}

uint32 TifWidth()
{
	return tiffWidth;

}

uint32 TifLength()
{
	return tiffLength;

}


double GetTagImageDepth(TIFFManipulate* src)
{
	uint32* depth;
	int result;

	depth=0;

	setHandlers(src);
	result = TIFFGetField(src->tiffHandle, TIFFTAG_IMAGEDEPTH, depth );
	restoreHandlers(src);

	if (result) {
		return *depth;
	}

	return -1;
}


double GetTagResolutionUnit(TIFFManipulate* src)
{
	uint16* unit;
	int result;
	
	unit=0;

	setHandlers(src);
	result = TIFFGetField( src->tiffHandle, TIFFTAG_RESOLUTIONUNIT, unit );
	restoreHandlers(src);

	if (result) {
		return *unit;
	}

	return -1;
}


double GetTagCompression(TIFFManipulate* src)
{
	uint16* var;
	int result;

	var =0;

	setHandlers(src);
	result = TIFFGetField( src->tiffHandle, TIFFTAG_COMPRESSION, var );
	restoreHandlers(src);

	if (result) {
		return *var;
	}

	return -1;
}


double GetTagGroup3Options(TIFFManipulate* src)
{
	uint32* var;
	int result;

	var =0;

	setHandlers(src);
	result = TIFFGetField( src->tiffHandle, TIFFTAG_GROUP3OPTIONS, var );
	restoreHandlers(src);

	if (result) {
		return *var;
	}

	return -1;
}


double GetTagGroup4Options(TIFFManipulate* src)
{
	uint32* var;
	int result;

	var =0;

	setHandlers(src);
	result = TIFFGetField( src->tiffHandle, TIFFTAG_GROUP4OPTIONS, var );
	restoreHandlers(src);

	if (result) {
		return *var;
	}

	return -1;
}


char* GetTagSoftware(TIFFManipulate* src)
{
	char** var;
	int result;

	var =0;

	setHandlers(src);
	result = TIFFGetField( src->tiffHandle, TIFFTAG_SOFTWARE, var );
	restoreHandlers(src);

	if (result) {
		return *var;
	}

	*var = "undefined";
	return *var;
}


float GetTagXResolution(TIFFManipulate* src)
{
	float* var;
	int result;

	var =0;

	setHandlers(src);
	result = TIFFGetField(src->tiffHandle, TIFFTAG_XRESOLUTION, var );
	restoreHandlers(src);

	if (result) {
		return *var;
	}

	return -1;
}


float GetTagYResolution(TIFFManipulate* src)
{
	float* var;
	int result;

	var =0;

	setHandlers(src);
	result = TIFFGetField( src->tiffHandle, TIFFTAG_YRESOLUTION, var );
	restoreHandlers(src);

	if (result) {
		return *var;
	}

	return -1;
}


int AppendDirectory(TIFFManipulate* src,TIFFManipulate* tget )
{
	int result;
/*
	if (0) {	// Check that file opened in append mode
		return 0;
	}
*/

	setHandlers(src);

	
	result =
		tiffcp_interface(
			src->tiffHandle,			
			tget->tiffHandle,
			src->tiffcpCompressionOptions,
			src->tiffcpConfig,
			src->tiffcpFillOrder,
			src->tiffcpRowsPerStrip,
			src->tiffcpOutTiled,
			src->tiffcpTileWidth,
			src->tiffcpTileLength
		);

	tiffWidth = GettiffWidth();
	tiffLength = GettiffLength();

	restoreHandlers(src);
	
	if (!result) {
		return 0;
	}

	return 1;
}

void
SetOutTiled( int i, TIFFManipulate* src )
{
	src->tiffcpOutTiled = i;
}


void
SetPlanarConfig( uint16 i, TIFFManipulate* src  )
{
	src->tiffcpConfig = i;
}


void
SetCompressionOptions( char* opts, TIFFManipulate* src )
{
	src->tiffcpCompressionOptions = opts;
}


void
SetFillOrder( uint16 i, TIFFManipulate* src )
{
	src->tiffcpFillOrder = i;
}


void
SetRowsPerStrip( uint32 i, TIFFManipulate* src )
{
	src->tiffcpRowsPerStrip = i;
}


void
SetTileWidth( uint32 i, TIFFManipulate* src )
{
	src->tiffcpTileWidth = i;
}


void
SetTileLength( uint32 i,TIFFManipulate* src )
{
	src->tiffcpTileLength = i;
}


tdir_t
GetLastDirectoryNumber( TIFFManipulate* src )
{
	tdir_t lastDir;
	tdir_t dirno_save = CurrentDirectory(src);
	while (!LastDirectory(src)) {
		ReadDirectory(src);
	}

	lastDir = CurrentDirectory(src);

	SetDirectory( dirno_save, src );

	return lastDir;
}

