#ifndef _TIFFMULTIPAGES_H_
#define _TIFFMULTIPAGES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "TIFFManipulate.h"

int ParticionaImg(char *, char *, char *);

long FileSize(FILE *);

#define BUFSIZE     (80*1024)
#define	TRUE	1
#define	FALSE	0

static	int ignore = FALSE;		/* if true, ignore read errors */



int ParticionaImg(char *Image, char *DirCompletas, char *DirPartidas) {
	uint32 diroff = 0, p_diroff = 0;
	char mode[10];
	char fname[80];
	char fname1[80];
	char fname2[80];
	char* mp = mode;
	uint16 numPages = 0;
	uint16 oddPage = 1;
	int verbose = TRUE;
	int veryVerbose = FALSE;
	int warnings = FALSE;
	char* outFile;
	char* oddFile;
	char* tempFile;
	uint32	npixels, w, l;


	int i,j;
	long tam,tamtemp;

	TIFFManipulate odd;
	TIFFManipulate out;
	TIFFManipulate arctemp;

	FILE *file;
	char	cade[256];
	
	i= 1;
	j=0;
	tam=0;	
	tamtemp=0;

	*mp++ = 'w';
	*mp = '\0';

	sprintf( cade, " cd %s", DirCompletas);	
	system ( cade );
	#ifdef _WIN32
		sprintf (fname1,"%s\\%s_%d.tif",DirPartidas,Image,i);
	#else
		sprintf (fname1,"%s/%s_%d.tif",DirPartidas,Image,i);
	#endif	
	outFile =  fname1;		
	
	

	#ifdef _WIN32
		sprintf (fname,"%s\\%s_%d.tif",DirCompletas,Image,i);
	#else
		sprintf (fname,"%s/%s_%d.tif",DirCompletas,Image,i);
	#endif	
	oddFile = fname;

	#ifdef _WIN32
		sprintf (fname2,"%s\\Filetemp.tif",DirCompletas);
	#else
		sprintf (fname2,"%s/Filetemp.tif",DirCompletas);
	#endif	
	tempFile = fname2;


	mode[0] = 'a';
		
	if (diroff != 0 && !SetSubDirectory(diroff, &odd)) {
		PrintError(GetFileName(&odd),
			"Error, setting subdirectory at %#x", diroff);
		return (1);
	}

	TIFFManipulateIni(&out);
	if (!Open(&out, outFile, mode))		
		return (-2);

	sprintf( cade, " cd %s", DirPartidas);	
	system ( cade );

	TIFFManipulateIni(&odd);
	if (!Open(&odd,oddFile, "r"))
		return -3;

	do {

		tamtemp=0;

		#ifdef _WIN32
			sprintf( cade, " del %s\\Filetemp.tif", DirCompletas);	
			system ( cade );
		#else
			sprintf( cade, " rm %s/Filetemp.tif", DirCompletas);	
			system ( cade );
		#endif

		TIFFManipulateIni(&arctemp);
		if (!Open(&arctemp, tempFile, mode))		
			return (-2);

		if (veryVerbose) {
			printf("\n");
			PrintDirectory( stdout, TIFFPRINT_NONE, &odd );
		}

		if (!AppendDirectory( &odd, &arctemp )) {
			PrintError("tiffmesh", "Error copying file %s", oddFile);
			return (1);
		}

		l = TifLength(); 
		w = TifWidth(); 
		npixels = l*w;

		file = fopen(fname2, "r" ) ;
		if( ! file ){
  			printf ("No se creo ArchivoLog %s \n" , fname2); 
			free( file ) ;
			exit( 1 ) ;
		}
		tamtemp = FileSize(file);
		fflush (file );
		fclose( file );

		Close(&arctemp);

		if (tamtemp <= 9900000 ) {

			if ((tam+tamtemp) > 9900000 ) {
				Close(&out);
				i++;
				sprintf( cade, " cd %s", DirPartidas);	
				system ( cade );
				#ifdef _WIN32
					sprintf (fname1,"%s\\%s_%d.tif",DirPartidas,Image,i);
				#else
					sprintf (fname1,"%s/%s_%d.tif",DirPartidas,Image,i);
				#endif	
				outFile =  fname1;
				TIFFManipulateIni(&out);
				if (!Open(&out, outFile, mode))
					return (-2);
			} /*End if (tam > 980000 )*/

			if (veryVerbose) {
				printf("\n");
				PrintDirectory( stdout, TIFFPRINT_NONE, &odd );
			}

			if (!AppendDirectory( &odd, &out )) {
				PrintError("tiffmesh", "Error copying file %s", oddFile);
				return (1);
			}

			l = TifLength(); 
			w = TifWidth(); 
			npixels = l*w;

			if (verbose) {
				printf("Appended page %i from %s.\n", CurrentDirectory(&odd)+1, oddFile);
			}
		
			file = fopen(fname1, "r" ) ;
			if( ! file ){
  				printf ("No se creo ArchivoLog %s \n" , fname1); 
				free( file ) ;
				exit( 1 ) ;
			}
			tam = FileSize(file);
			fflush (file );
			fclose( file );

			j=1;

		} /*End if (tamtemp <= 980000 )*/

		oddPage++;

	} while (ReadDirectory(&odd) ); /*//&& p_diroff == 0 );*/

	Close(&odd);
	Close(&out);

	if (j==1) {
		return (0);
	} else {
		return (1);
	}

}


long FileSize(FILE *file)
{
	long pos_actual, tam;
	pos_actual = ftell(file);
	fseek(file, 0L, SEEK_END);
	tam = ftell(file);
	fseek(file, pos_actual, SEEK_SET); /* vuelvo a poner el puntero donde estaba antes */
	return tam;
}



#endif 
/* _TIFFMULTIPAGES_H_ */

