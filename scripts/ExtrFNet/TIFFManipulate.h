#include <stdarg.h>
#include "tiffio.h"
#include "tiff.h"

#ifndef _TIFFMANIPULATE_H
#define _TIFFMANIPULATE_H


typedef struct {

	TIFF*	tiffHandle;
	int		fileOpened;
	char*	fileName;
	char*	fileMode;
	TIFFErrorHandler	oldErrorHandler;
	TIFFErrorHandler	oldWarningHandler;

	uint16	tiffcpConfig;
	uint16	tiffcpFillOrder;
	uint32	tiffcpRowsPerStrip;
	int		tiffcpOutTiled;
	uint32	tiffcpTileWidth;
	uint32	tiffcpTileLength;	
	char*	tiffcpCompressionOptions;

} TIFFManipulate;

	void	setHandlers(TIFFManipulate*) ;
	void	restoreHandlers(TIFFManipulate*);

	int			printErrors;
	int			printWarnings;




	/* static member functions */
	void	ErrorHandlerWrapper(  char*,  char*, va_list );
	void	WarningHandlerWrapper(  char*,  char*, va_list );

	void		ErrorHandler( const char*, const char*, va_list );
	void		WarningHandler( const char*, const char*, va_list );
	void		PrintError( const char*, const char*, ... );
	void		PrintWarning( const char*, const char*, ... );

	int			Open(TIFFManipulate*, const char*, const char* );
	void		Close(TIFFManipulate*);
	tdir_t		CurrentDirectory(TIFFManipulate*);
	int			ReadDirectory();
	void		PrintDirectory( FILE*, long, TIFFManipulate* );
	int			SetDirectory( tdir_t,TIFFManipulate* );
	int			SetSubDirectory( uint32, TIFFManipulate* );
	int			LastDirectory();
	int			WriteDirectory();
	TIFF*		GetTIFFHandle();
	const char*	GetFileName();
	int			GetTagOrientation();
	double		GetTagImageLength();
	double		GetTagImageWidth();
	double		GetTagImageDepth();
	double		GetTagResolutionUnit();
	double		GetTagCompression();
	double		GetTagGroup3Options();
	double		GetTagGroup4Options();
	char*		GetTagSoftware();
	float		GetTagXResolution();
	float		GetTagYResolution();
	uint32		GettiffLength();
	uint32		GettiffWidth();

	/* Custom Methods*/
	void	TIFFManipulateIni(TIFFManipulate* );
	int		AppendDirectory(TIFFManipulate*, TIFFManipulate* );
	void	SetOutTiled( int, TIFFManipulate* );
	void	SetPlanarConfig( uint16, TIFFManipulate* );
	void	SetFillOrder( uint16, TIFFManipulate* );
	void	SetRowsPerStrip( uint32, TIFFManipulate*  );
	void	SetTileWidth( uint32, TIFFManipulate*  );
	void	SetTileLength( uint32, TIFFManipulate* );
	void	SetCompressionOptions( char* , TIFFManipulate*);
	int		pointerToObject;
	
	tdir_t	GetLastDirectoryNumber(TIFFManipulate*);

	uint32 TifWidth();
	uint32 TifLength();


#endif


