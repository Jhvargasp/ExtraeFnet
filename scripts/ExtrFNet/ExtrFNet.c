 /*                                                                       *
 * Programa: ExtrFet.c                                                    *
 *                                                                        *
 * Objetivo: Gerarar un programa para extraer las imágenes de FileNet que *
 *           son califadas para manejo  y conversion a OnDemand.		      *
 *																		                                    *
 * Fecha de Creación : Abr/2005								                            *
 * Autor  : Andrés Ventura G.                                             *
 **************************************************************************
 * Modifico  : Andrés Ventura G.                                          *
 * Fecha Mod.: Jun/2012                                                   *
 * Mod.: Actualizacion de SIRH1 -> SIRH2 por el parametro                 *
 ***************************************************************>>avg<<***/
#ifdef _WIN32
	#include <sys\types.h>
#else
	#include <sys/types.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "TIFFMultipages.h"
#include "ExtrFNet.h"


void	ParteImagenes();
static int MueveHasta( char *, char, char *, int, int);
void	initialize();
void	get_profile_string( char *, char*, char* );
void	Inicia_Borrado_ImgCargadas (int );
void	Inicia_Proceso ();
void	Inicia_Reproceso (int, char *);
short bisiesto (int );
short valida_fecha(int, int, int);
char	anio[4],mes[2],dia[2];

/* AVG JUN-2012 FIN	----------------------------------------------------------*/
void Act_SIRH (int , char *);
/* AVG JUN-2012 FIN	----------------------------------------------------------*/


main( int argc, char **argv )
{
	
	int		posini,posfin;
	char	temp1[50];
	
	if( argc < 2 ){
		printf( "Uso %s Seccion \n" , argv[0] ) ;
		exit( 1 ) ;
	}
	if (argc>=2) {
		sprintf( VarPaso, "%s", argv[1] ) ;
		Seccion=atoi(VarPaso);
	} else {
		Seccion=1;	
	}


	if (Seccion > 2 && Seccion < 5 ) { 
		/* Proceso de Borrado de Img Seccion= 3 (Emp), Seccion= 4 (Consumo) */
		sprintf(pSeccion,"INIFILES%d",Seccion-2);
		printf( " \n Argc: %d Seccion: %s pSeccion: %s \n ", argc, argv[1], pSeccion);

		initialize ();
	
		Inicia_Borrado_ImgCargadas ( 1 ) ;
		Inicia_Borrado_ImgCargadas ( 2 ) ;

	}      
	if (Seccion > 0 && Seccion < 3 ) 
	{  
			/* Proceso de Extracciones Seccion= 1 (Emp) Seccion = 2 (Consumo) */
			sprintf(pSeccion,"INIFILES%d",Seccion);

			printf( " \n Argc: %d Seccion: %s pSeccion: %s \n ", argc, argv[1], pSeccion);

			initialize ();

			Inicia_Proceso();	
			
			#ifdef _WIN32
				sprintf(pathArchivoIndices , "%s\\Indices.txt" , DirCompletas);
			#else
				sprintf(pathArchivoIndices , "%s/Indices.txt" , DirCompletas);
			#endif

			ArchivoIndices = fopen(pathArchivoIndices, "r" ) ;
			if( ! ArchivoIndices ){
  				printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
				/* //free( pathArchivoIndices ); */
				exit( 1 ) ;
			}

			ParteImagenes();
			fflush (ArchivoIndices );
			fclose( ArchivoIndices );

	}
	if (Seccion > 4 && Seccion < 7 ) 
	{  
				/* Proceso de Resproceso Seccion= 5 (Emp) Seccion = 6 (Consumo) */
				sprintf(pSeccion,"INIFILES%d",Seccion-4);
				printf( " \n Argc: %d Seccion: %s pSeccion: %s \n ", argc, argv[1], pSeccion);

				initialize ();

				strcpy( temp1, argv[2]);
				posini = 0;
				posfin=MueveHasta( temp1, '/', argv[2], MAXLINE, 0) ;
				temp1[posfin] = '\0';
				sprintf(mes, temp1);
		
				strcpy( temp1, argv[2]);
				posini = posfin;
				posfin=MueveHasta( temp1, '/', argv[2], MAXLINE, posfin+1) ;
				temp1[posfin- posini-1] = '\0';
				sprintf(dia, temp1);
		
				strcpy( temp1, argv[2]);
				strcat(temp1,"/");
				posini = posfin;
				posfin=MueveHasta( temp1, '/', temp1, MAXLINE, posfin+1) ;
				temp1[posfin- posini-1] = '\0';
				sprintf(anio, temp1);
		
				/*if (valida_fecha(atoi(dia), atoi(mes), atoi(anio))) 
				{
					printf ("Fecha válida\n"); */

					Inicia_Reproceso(Seccion, argv[2]);	
				
					#ifdef _WIN32
						sprintf(pathArchivoIndices , "%s\\Indices.txt" , DirCompletas);
					#else
						sprintf(pathArchivoIndices , "%s/Indices.txt" , DirCompletas);
					#endif

					ArchivoIndices = fopen(pathArchivoIndices, "r" ) ;
					if( ! ArchivoIndices )
					{
	  					printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
							exit( 1 ) ;
					}
	
					ParteImagenes();
					fflush (ArchivoIndices );
					fclose( ArchivoIndices );
					
					/*}
					else
					{
					printf ("Fecha NO válida, Formato MM/DD/YYYY\n");
					}*/
	}
	
	/* AVG Ini Jun-2012	*/
	if (Seccion == 7 && argc == 4 ) 
	{
					/* Actualización de campo UOCC SIRH1 -> SIRH 2 */
					sprintf(pSeccion,"INIFILES%d",Seccion-6);
					printf( " \n Argc: %d Seccion: %s pSeccion: %s \n ", argc, argv[1], pSeccion);
					initialize ();
					sprintf( VarPaso, "%s", argv[2] ) ;
					SIRH1=atoi(VarPaso);
					sprintf( VarPaso, "%s", argv[3] ) ;
					/*SIRH2=atoi(VarPaso);*/
					sprintf( SIRH2, "%s", VarPaso ) ;
					printf( " \n Argc: %d Seccion: %s Arg1: %s Arg2: %s \n ", argc, argv[1], argv[2], argv[3]);
					Act_SIRH(SIRH1, SIRH2);
		}
		/* AVG Fin Jun-2012	*/
}

void ParteImagenes( )
{
	int				i;

	char	cadena[MAXLINE] ;
	char	respaldo[MAXLINE] ;
	char    temp[50];
	long    LongArchImag;
	static char     fsep      = ':';

	i=0;
	while( fgets( cadena, 128, ArchivoIndices ) ){
		strcpy( respaldo, cadena ) ;			
		LongArchImag=MueveHasta( temp, fsep, cadena, MAXLINE, 0) ;
		temp[LongArchImag] = '\0' ;
		ParticionaImg(temp, DirCompletas, DirPartidas);
		i++;
	}
}

short bisiesto (int a)
{
return (a%4==0) && ( (a%100!=0) || (a%400==0) );
}

short valida_fecha(int dd, int mm, int aa)
{
	short dias_mes[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ( (dd<=0) || (mm<=0) ) /* Si el día, o mes es negativo o cero */
	return 0; /* la fecha no es válida */

	if ( (mm!=2) || (!bisiesto(aa)) )
		return (dd<=dias_mes[mm-1]);
	else
		return (dd<=dias_mes[1]+1);
}

static int MueveHasta ( char *temp, char fs, char *line1, int limite, int inicio )
{
	int n=inicio;
	if (*(line1 + n) == fs )
	return ( 0 ) ;
	if (*(line1 +n) == '\n' || n >= limite )
	return (-1) ;
	while( *(line1 +  n) != fs && *(line1 + n) != '\n' && n < limite )
	{
	*(temp + (n - inicio)) =  *(line1 + n);
	n++;
	}
	return ( n );
}
