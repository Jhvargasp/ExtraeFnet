/**************************************************************************
 *                                                                        *
 * Programa: UtlExtrFet.c                                                 *
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
 **************************************************************************
 * Modifico  : Andrés Ventura G.                                          *
 * Fecha Mod.: Sept/2015                                                  *
 * Mod.: Se incluye el manejo de SubFolio al amparo del folio Padre       *
 ***************************************************************>>avg<<***/

#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
	#include <windows.h>
	#include <process.h>
	#include <sys\types.h>
	#include <sys\stat.h>
#else
	#include <sys/io.h>
	#include <sys/types.h>
	#include <sys/stat.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <malloc.h>

#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "ExtrFNet.h"
#include "tiff.h"
#include "tiffio.h"

/* Sapuf */
#include "libsapuf.h"
#include "varsapuf.h"

#include <DOC.i>
#include <CKS.i>
#include <CSM.i>
#include <IMS.i>
#include <DTI.i>
#include <FP.i>
#include <SQI.i>
#include <FFI.i>

#define BUFSIZE     (80*1024)
/*#define MIN(a,b)    ((a) < (b) ? (a) : (b)) */


char	CalificaOnDemand[MAX_VAR_NAME_SIZE],Status[MAX_VAR_NAME_SIZE],XFile[MAX_VAR_NAME_SIZE];
char	FolioS403[MAX_VAR_NAME_SIZE],Producto[MAX_VAR_NAME_SIZE],DocumentID[MAX_VAR_NAME_SIZE];
char	NumCliente[MAX_VAR_NAME_SIZE],Contrato[MAX_VAR_NAME_SIZE],Linea[MAX_VAR_NAME_SIZE];
char	FechaOperacion[MAX_VAR_NAME_SIZE],Folio[MAX_VAR_NAME_SIZE], SubFolio[MAX_VAR_NAME_SIZE];
char	Instrumento[MAX_VAR_NAME_SIZE],TipoDoc[MAX_VAR_NAME_SIZE],UOC[MAX_VAR_NAME_SIZE];
int		ImagProc,FechaFileNET;
char	curpath[MAXPATH];
char	pathArchivoLog[MAXPATH];
char	pathArchivoIndices[MAXPATH];
char	FechaFNetOp[10];
char	HOME[MAX_VAR_NAME_SIZE];

ASE_session_number_typ  sqih = 0;
ASE_session_number_typ	imsh = 0;

typedef  struct  var_typ
{	
	char				varname [MAX_VAR_NAME_SIZE + 1];
	char				*data_p;
	long				size;
	SQI_data_type_typ	type;
	short				ind;
	unsigned short		pos;
} var_typ;

static FILE *ArchivoLog ;
static FILE *ArchivoIndices;
static FILE *ArchivoIndTemp;

void     initialize();
void     quit();
void	 interrupt_handler(int);
extern   void exit();

void	logon( int );
void  logoff();
void	get_profile_string( char *, char*, char* );
void	PreparaAmbiente( int );
void	CargaDatos( int );
void	CargaDatos_ImgBorrar( int );
void	CargaFiltro(char *, char *);
void	ProcesaIndices( int );
void	ProcesaBIndices( );

int     ExtraeImagenes(char *);
int     Particiona(char *);
void	FechaOpera( int );
void	Inicia_Proceso ();
void	Inicia_Reproceso (int, char *);

static char *GetFecha( void );
static int MueveHasta( char *, char, char *, int, int);
error_typ create_cursor();
error_typ do_fetch();
error_typ do_fetch2();
error_typ exec();
error_typ execsql();
error_typ bindvalue();
error_typ bindresult();

bool    aborted = FALSE;

char 	X_domain[32];
char 	X_Org[32];
char 	fecha[9];


#if defined(__mpexl)
char *interrupt_str = "";
#else
char *interrupt_str = " or type ^C";
#endif

/* AVG JUN-2012 FIN	----------------------------------------------------------*/
void 	Act_SIRH (int, char *);
void  CargaDatosSIRH( int  );
error_typ do_fetch3();
/* AVG JUN-2012 FIN	----------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
void
interrupt_handler (int iVar)
{
    /* Note: NEVER do long jumps or exits from signal handlers which are 
       potentially in the middle of a WAL subroutine. */
    aborted = TRUE;
    signal (SIGINT, interrupt_handler);
}

/*----------------------------------------------------------------------------*/
void Inicia_Proceso ( ) 
{

	char vartemp[11];

	initialize ();
	strcpy(X_domain, DOMAINIO);
	strcpy(X_Org, ORGANIZATION);

	PreparaAmbiente (1); 

	sprintf ( vartemp, "%s", GetFecha());

	#ifdef _WIN32
		sprintf(pathArchivoLog , "%s\\C406%s.log" , DirBitacoras,vartemp);
	#else
		sprintf(pathArchivoLog , "%s/C406%s.log" , DirBitacoras,vartemp);
	#endif

	ArchivoLog = fopen(pathArchivoLog, "w" ) ;
	if( ! ArchivoLog ){
  		printf ("No se creo ArchivoLog %s \n" , pathArchivoLog); 
		free( pathArchivoLog ) ;
		exit( 1 ) ;
	}

	#ifdef _WIN32
		sprintf(pathArchivoIndices , "%s\\IndTemp.txt" , DirCompletas);
	#else
		sprintf(pathArchivoIndices , "%s/IndTemp.txt" , DirCompletas);
	#endif
	
	ArchivoIndTemp = fopen(pathArchivoIndices, "w" ) ;
	if( ! ArchivoIndTemp ){
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( pathArchivoIndices ) ;
		exit( 1 ) ;
	}

	fprintf(ArchivoLog, "Superficie a procesar \n") ;
	sprintf ( vartemp, "%s", GetFecha());
	fprintf(ArchivoLog, "Inicia Proceso : %s \n", vartemp) ;

  logon(1);

	fprintf(ArchivoLog, "\n\t========== OUTPUT BINDING TEST  FIELDS=========\n") ;
	CargaFiltro (CalificaOnDemand,"'CalificaOnDemand'");
	CargaFiltro (Status,"'Status'");
	CargaFiltro (XFile,"'XfolioS'");
	CargaFiltro (UOC,"'UOC'");
	CargaFiltro (TipoDoc,"'TipoDoc'");
	CargaFiltro (Folio,"'Folio'");
	CargaFiltro (NumCliente,"'NumCliente'");
	CargaFiltro (Contrato,"'Contrato'");
	CargaFiltro (Linea,"'Linea'");
	CargaFiltro (Producto,"'Producto'");
	CargaFiltro (Instrumento,"'Instrumento'");
	CargaFiltro (FolioS403,"'FolioS403'");
	CargaFiltro (FechaOperacion,"'FechaOperacion'");
	CargaFiltro (SubFolio,"'XfolioP'");

	CargaDatos( 1 );

	fprintf(ArchivoLog, "\t-------------------------------------------------------\n");
	
	fclose( ArchivoIndTemp );
	PreparaAmbiente (3); 

	ArchivoIndTemp = fopen(pathArchivoIndices, "r" ) ;
	if( ! ArchivoIndTemp ){
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( ArchivoIndTemp ) ;
		exit( 1 ) ;
	}

	#ifdef _WIN32
		sprintf(pathArchivoIndices , "%s\Indices.txt" , DirCompletas);
	#else
		sprintf(pathArchivoIndices , "%s/Indices.txt" , DirCompletas);
	#endif
		
	ArchivoIndices = fopen(pathArchivoIndices, "w" );
	if( ! ArchivoIndices ){
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( pathArchivoIndices ) ;
		exit( 1 ) ;
	}

	FechaOpera(1);
	ProcesaIndices(1);
	fflush (ArchivoIndTemp );
	fclose( ArchivoIndTemp );
	fclose( ArchivoIndices );

	/*sprintf(pathArchivoIndices , "%s\\Indices.txt" , DirCompletas);
	ArchivoIndices = fopen(pathArchivoIndices, "r" ) ;
	if( ! ArchivoIndices ){
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( pathArchivoIndices ) ;
		exit( 1 ) ;
	}

	ParteImagenes();*/
	
	fflush (ArchivoIndices );
	fclose( ArchivoIndices );
	fprintf(ArchivoLog, "\t-------------------------------------------------------\n");
	fflush (ArchivoLog );
	fclose( ArchivoLog );
	logoff();

}

/*----------------------------------------------------------------------------*/
void Inicia_Reproceso (int opc, char *Fecha)
{

	char vartemp[11];

	initialize ();
	strcpy(X_domain, DOMAINIO);
	strcpy(X_Org, ORGANIZATION);

	PreparaAmbiente (1); 

	sprintf ( vartemp, "%s", GetFecha());

	#ifdef _WIN32
		sprintf(pathArchivoLog , "%s\\C406%s.log" , DirBitacoras,vartemp);
	#else
		sprintf(pathArchivoLog , "%s/C406%s.log" , DirBitacoras,vartemp);
	#endif

	ArchivoLog = fopen(pathArchivoLog, "w" ) ;
	if( ! ArchivoLog ){
  		printf ("No se creo ArchivoLog %s \n" , pathArchivoLog); 
		free( pathArchivoLog ) ;
		exit( 1 ) ;
	}

	#ifdef _WIN32
		sprintf(pathArchivoIndices , "%s\\IndTemp.txt" , DirCompletas);
	#else
		sprintf(pathArchivoIndices , "%s/IndTemp.txt" , DirCompletas);
	#endif
	
	ArchivoIndTemp = fopen(pathArchivoIndices, "w" ) ;
	if( ! ArchivoIndTemp ){
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( pathArchivoIndices ) ;
		exit( 1 ) ;
	}

	fprintf(ArchivoLog, "Superficie a procesar \n") ;
	sprintf ( vartemp, "%s", GetFecha());
	fprintf(ArchivoLog, "Inicia Proceso : %s \n", vartemp) ;

   	logon(1);

	fprintf(ArchivoLog, "\n\t========== OUTPUT BINDING TEST  FIELDS=========\n") ;
	CargaFiltro (CalificaOnDemand,"'CalificaOnDemand'");
	CargaFiltro (Status,"'Status'");
	CargaFiltro (XFile,"'XfolioS'");
	CargaFiltro (UOC,"'UOC'");
	CargaFiltro (TipoDoc,"'TipoDoc'");
	CargaFiltro (Folio,"'Folio'");
	CargaFiltro (NumCliente,"'NumCliente'");
	CargaFiltro (Contrato,"'Contrato'");
	CargaFiltro (Linea,"'Linea'");
	CargaFiltro (Producto,"'Producto'");
	CargaFiltro (Instrumento,"'Instrumento'");
	CargaFiltro (FolioS403,"'FolioS403'");
	CargaFiltro (FechaOperacion,"'FechaOperacion'");
	CargaFiltro (SubFolio,"'XfolioP'");

	strcpy (FechaFNetOp, Fecha);
	FechaOpera(2);
	CargaDatos(2);

	fprintf(ArchivoLog, "\t-------------------------------------------------------\n");
	
	fclose( ArchivoIndTemp );
	PreparaAmbiente (5); 

	ArchivoIndTemp = fopen(pathArchivoIndices, "r" ) ;
	if( ! ArchivoIndTemp ) {
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( ArchivoIndTemp ) ;
		exit( 1 ) ;
	}

	#ifdef _WIN32
		sprintf(pathArchivoIndices , "%s\\Indices.txt" , DirCompletas);
	#else
		sprintf(pathArchivoIndices , "%s/Indices.txt" , DirCompletas);
	#endif
		
	ArchivoIndices = fopen(pathArchivoIndices, "w" ) ;
	if( ! ArchivoIndices ){
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( pathArchivoIndices ) ;
		exit( 1 ) ;
	}

	ProcesaIndices(2);

	fflush (ArchivoIndTemp );
	fclose( ArchivoIndTemp );
	fclose( ArchivoIndices );

	fflush (ArchivoIndices );
	fclose( ArchivoIndices );
	fprintf(ArchivoLog, "\t-------------------------------------------------------\n");
	fflush (ArchivoLog );
	fclose( ArchivoLog );
	logoff();

}

/* AVG JUN-2012 Ini	----------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void Act_SIRH (int SIRH1, char *SIRH2) 
{

	char vartemp[11];
	char	arg2[1920];
	SQI_cursor_handle_typ	ch;
	SQI_sqlca_typ		sqlca;
	var_typ				var1;
	var_typ				var2;
	initialize ();
	strcpy(X_domain, DOMAINIO);
	strcpy(X_Org, ORGANIZATION);

	sprintf ( vartemp, "%s", GetFecha());

	#ifdef _WIN32
		sprintf(pathArchivoLog , "%s\\C406-1%s.log" , DirBitacoras,vartemp);
	#else
		sprintf(pathArchivoLog , "%s/C406-1%s.log" , DirBitacoras,vartemp);
	#endif

	ArchivoLog = fopen(pathArchivoLog, "w" ) ;
	if( ! ArchivoLog )
	{
  		printf ("No se creo ArchivoLog %s \n" , pathArchivoLog); 
			free( pathArchivoLog );
			exit( 1 );
	}

	fprintf(ArchivoLog, "Superficie a procesar \n") ;
	sprintf ( vartemp, "%s", GetFecha());
	fprintf(ArchivoLog, "Inicia Proceso : %s \n", vartemp) ;
	fprintf(ArchivoLog, "Archivo Log: : %s \n", pathArchivoLog) ;

   	logon(1);

	fprintf(ArchivoLog, "\n\t========== OUTPUT BINDING TEST  FIELDS=========\n") ;
	CargaFiltro (CalificaOnDemand,"'CalificaOnDemand'");
	CargaFiltro (Status,"'Status'");
	CargaFiltro (XFile,"'XfolioS'");
	CargaFiltro (UOC,"'UOC'");
	CargaFiltro (TipoDoc,"'TipoDoc'");
	CargaFiltro (Folio,"'Folio'");
	CargaFiltro (NumCliente,"'NumCliente'");
	CargaFiltro (Contrato,"'Contrato'");
	CargaFiltro (Linea,"'Linea'");
	CargaFiltro (Producto,"'Producto'");
	CargaFiltro (Instrumento,"'Instrumento'");
	CargaFiltro (FolioS403,"'FolioS403'");
	CargaFiltro (FechaOperacion,"'FechaOperacion'");
	CargaFiltro (SubFolio,"'XfolioP'");

	CargaDatosSIRH(SIRH1 );

	fprintf(ArchivoLog, "\t-------------------------------------------------------\n");
	
	if ( BASEDATOS==1 ) 
	{
					/* Query SQL */
					sprintf( arg2, "Update f_sw.doctaba Set %s=%s", UOC,SIRH2); 
					sprintf( VarPaso, " Where %s=%d", UOC, SIRH1);
					strcat(arg2, VarPaso);
	} else {			
					/* Query Oracle */
					sprintf( arg2, "UPDATE F_SW.DOCTABA A SET A.%s=%s", UOC, SIRH2);
					sprintf( VarPaso, " WHERE A.%s=%d", UOC, SIRH1);
					strcat(arg2, VarPaso);
					strcat(arg2, ";");
	}

			fprintf(ArchivoLog, "Query Update :%s \n", arg2);
			create_cursor( arg2, &ch, &sqlca);

				memset( &var1, 0, sizeof(var_typ) );
				memset( &var2, 0, sizeof(var_typ) );
				
				strcpy( var1.varname, UOC );
				var1.data_p	= (char *)malloc(30 * sizeof(char));
				/*var1.data_p = SIRH2;*/
				sprintf( var1.data_p, SIRH2);			
				var1.type	= SQI_long;
				var1.size	= 10;
				var1.pos = 1;
				
				fprintf(ArchivoLog, "\n\t ---> Var1.varname: %s  ... <---\n", var1.varname);
				fprintf(ArchivoLog, "\n\t ---> var1.data_p: %d  ... <---\n", var1.data_p);
	
				printf( "\n\t ---> Query %s  ... <---\n",arg2 );

				exec( &ch, TRUE, 1, TRUE, &sqlca);
				
				fprintf(ArchivoLog, "\n\t ---> Actualizacion Exitosa ... <---\n");
				
				logoff();
				fprintf(ArchivoLog, "\t------------------ Termina proceso --------------------\n");
				fprintf(ArchivoLog, "\t-------------------------------------------------------\n");
	
				fflush (ArchivoLog );
				fclose( ArchivoLog );
}

void CargaDatosSIRH( int SIRH1 ) 
{

	char				arg1[1920];
	SQI_cursor_handle_typ	ch;
	SQI_sqlca_typ		sqlca;
	var_typ				var1;
	var_typ				var2;
	var_typ				var3;
	var_typ				var4;
	var_typ				var5;
	var_typ				var6;
	var_typ				var7;
	var_typ				var8;
	var_typ				var9;
	var_typ				var10;
	var_typ				var11;
	var_typ				var12;

	/* 
	 * Create a cursor for selecting rows from the table.
	 */ 
	fprintf(ArchivoLog, "\n\t========== SELECIONANDO IMAGENES  =========\n") ;
	fprintf(ArchivoLog, "\t ---> 1. Create a cursor for selecting rows from the table...<-- \n") ;

	/*Print #2, DocumentID; ":"; NumCliente; ":"; TipoDoc; ":"; XFolio; ":"; _
    Contrato; ":"; Linea; ":"; Producto; "/"; Instrumento; ":"; FolioS403; ":"; FolioUOC; ":"; SubFolio*/

	if ( BASEDATOS==1 ) {
		/* Creación del Select con los campos utilizados (SQL) */
		sprintf( arg1, "Select a.f_docnumber, " );	
		sprintf( VarPaso, "isnull(a.%s,0),",NumCliente);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",TipoDoc);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",XFile);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Contrato);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Linea);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Producto);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Instrumento);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",FolioS403);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",UOC);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0)",Folio);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0)",SubFolio);
		strcat(arg1, VarPaso);
	} else {
		/* Creación del Select con los campos utilizados (Oracle) */
		sprintf( arg1, "Select a.f_docnumber, " );	
		sprintf( VarPaso, "NVL(a.%s,0),",NumCliente);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",TipoDoc);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",XFile);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Contrato);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Linea);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Producto);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Instrumento);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",FolioS403);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",UOC);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Folio);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0)",SubFolio);
		strcat(arg1, VarPaso);
	}
	strcat(arg1," From f_sw.doctaba a, f_sw.document_class b " );
	/* Armado de Where*/
	strcat(arg1," Where a.f_docclassnumber=b.f_docclassnumber ");
	strcat(arg1," And f_docclassname='ExpedientesDC'");
	
	/* UOC */
	sprintf( VarPaso, " And a.%s='%d'",UOC,SIRH1);
	strcat(arg1, VarPaso);
	fprintf(ArchivoLog, "Query :%s \n", arg1);
	
	create_cursor( arg1, &ch, &sqlca );

	memset( &var1, 0, sizeof(var_typ) );
	memset( &var2, 0, sizeof(var_typ) );
	memset( &var3, 0, sizeof(var_typ) );
	memset( &var4, 0, sizeof(var_typ) );
	memset( &var5, 0, sizeof(var_typ) );
	memset( &var6, 0, sizeof(var_typ) );
	memset( &var7, 0, sizeof(var_typ) );
	memset( &var8, 0, sizeof(var_typ) );
	memset( &var9, 0, sizeof(var_typ) );
	memset( &var10, 0, sizeof(var_typ) );
	memset( &var11, 0, sizeof(var_typ) );
	memset( &var12, 0, sizeof(var_typ) );

	/*#define SQI_boolean        1       / * TRUE or FALSE */
	/*#define SQI_byte           2       / * signed two's complement 8 bit quantity */
	/*#define SQI_unsigned_byte  3       / * unsigned 8 bit quantity */
	/*#define SQI_short          4       / * signed two's complement 16 bit quantity */
	/*#define SQI_unsigned_short 5       / * unsigned 16 bit quantity */
	/*#define SQI_long           6       / * signed two's complement 32 bit quantity */
	/*#define SQI_unsigned_long  7       / * unsigned 32 bit quantity */
	/*#define SQI_FPnum          8       / * FileNet floating point number */
	/*#define SQI_ASCII          9       / * ASCII string data ( null terminating ) */
	/*#define SQI_date           10      / * FileNet encoded date */
	/*#define SQI_time           11      / * FileNet encoded date and time */
	/*#define SQI_menu           12      / * integer values encoding string */
	/*#define SQI_text_object    13      / * lots of characters, with length */
	/*#define SQI_binary_object  14      / * lots of bytes (BLOB), with length */

	/* ******* Variables que se asignan de acuerdo al orden de los campos del Select ******* */

	strcpy( var1.varname, "var1" );
	var1.data_p	= (char *)malloc(30 * sizeof(char));
	var1.pos 	= 1;
	var1.type  	= SQI_ASCII;
	var1.size	= 30 * sizeof(char);

	strcpy( var2.varname, "var2" );
	var2.data_p	= (char *)malloc(30 * sizeof(char));
	var2.pos 	= 2;
	var2.type  	= SQI_ASCII;
	var2.size	= 30 * sizeof(char);

	strcpy( var3.varname, "var3" );
	var3.data_p	= (char *)malloc(30 * sizeof(char));
	var3.pos 	= 3;
	var3.type  	= SQI_ASCII;
	var3.size	= 30 * sizeof(char);

	strcpy( var4.varname, "var4" );
	var4.data_p	= (char *)malloc(30 * sizeof(char));
	var4.pos 	= 4;
	var4.type  	= SQI_ASCII;
	var4.size	= 30 * sizeof(char);

	strcpy( var5.varname, "var5" );
	var5.data_p	= (char *)malloc(30 * sizeof(char));
	var5.pos 	= 5;
	var5.type  	= SQI_ASCII;
	var5.size	= 30 * sizeof(char);

	strcpy( var6.varname, "var6" );
	var6.data_p	= (char *)malloc(30 * sizeof(char));
	var6.pos 	= 6;
	var6.type  	= SQI_ASCII;
	var6.size	= 30 * sizeof(char);

	strcpy( var7.varname, "var7" );
	var7.data_p	= (char *)malloc(30 * sizeof(char));
	var7.pos 	= 7;
	var7.type  	= SQI_ASCII;
	var7.size	= 30 * sizeof(char);

	strcpy( var8.varname, "var8" );
	var8.data_p	= (char *)malloc(30 * sizeof(char));
	var8.pos 	= 8;
	var8.type  	= SQI_ASCII;
	var8.size	= 30 * sizeof(char);

	strcpy( var9.varname, "var9" );
	var9.data_p	= (char *)malloc(30 * sizeof(char));
	var9.pos 	= 9;
	var9.type  	= SQI_ASCII;
	var9.size	= 30 * sizeof(char);

	strcpy( var10.varname, "var10" );
	var10.data_p	= (char *)malloc(30 * sizeof(char));
	var10.pos 	= 10;
	var10.type  	= SQI_ASCII;
	var10.size	= 30 * sizeof(char);

	strcpy( var11.varname, "var11" );
	var11.data_p	= (char *)malloc(30 * sizeof(char));
	var11.pos 	= 11;
	var11.type  	= SQI_ASCII;
	var11.size	= 30 * sizeof(char);
	
	strcpy( var12.varname, "var12" );
	var12.data_p	= (char *)malloc(30 * sizeof(char));
	var12.pos 	= 12;
	var12.type  	= SQI_ASCII;
	var12.size	= 30 * sizeof(char);

	memset( &sqlca, 0, sizeof(SQI_sqlca_typ) );

	bindresult( &ch, &var1, &sqlca );	
	bindresult( &ch, &var2, &sqlca );
	bindresult( &ch, &var3, &sqlca );
	bindresult( &ch, &var4, &sqlca );
	bindresult( &ch, &var5, &sqlca );
	bindresult( &ch, &var6, &sqlca );
	bindresult( &ch, &var7, &sqlca );
	bindresult( &ch, &var8, &sqlca );
	bindresult( &ch, &var9, &sqlca );
	bindresult( &ch, &var10, &sqlca );
	bindresult( &ch, &var11, &sqlca );
	bindresult( &ch, &var12, &sqlca );

	printf("\t ---> 2. Exececuting cursor for selecting rows from the table...<---\n");

	exec( &ch, TRUE, 1, FALSE, &sqlca );

	printf( "\n\t ---> 3. We will fetch all the rows and display...<---\n" );

	do_fetch3( &var1, &var2, &var3, &var4, &var5, &var6, &var7, &var8, &var9, &var10, &var11, &var12, &ch);

	SQI_drop_cursor( sqih, &ch, &sqlca );

	fprintf(ArchivoLog, "\t ---> Imagenes a procesar: %d \n ",ImagProc);

}

/*			--------------------
 *			   do_fetch3
 *			--------------------
 *
 */
error_typ
do_fetch3 ( var1, var2, var3, var4, var5, var6, var7, var8, var9, var10, var11, var12, ch)
var_typ		*var1, *var2, *var3, *var4, *var5, *var6, *var7, *var8, *var9, *var10, *var11, *var12;
SQI_cursor_handle_typ	*ch;
{
	short		count = 0;
	error_typ	err = 0;
	SQI_sqlca_typ	sqlca;	
	ImagProc =0;
	unsigned long x;
	fprintf(ArchivoLog,"\t---> Procesando Registros <---\n");
	printf( "\n\t ----> Fetch the data of query...<----\n\n" );

	while ( !err )
	{
		if ( err = SQI_fetch( sqih, *ch, &sqlca) )
   		{ 
   	  		if ( err_number(err) == SQI_END_OF_FETCH ) 
    		{ 
				return(err);
     		}
	   		if (err != SQI_ERR_OUTPUT_WARNING) 
   	  		{ 
				return(err);
   	  		}
			printf( "SQI_fetch: error <%d,%d,%d>\n",
            		err_category(err), err_function(err), err_number(err));
			return ( err );
   		}
		
		/*Print #2, DocumentID; ":"; NumCliente; ":"; TipoDoc; ":"; XFolio; ":"; _
		Contrato; ":"; Linea; ":"; Producto; "/"; Instrumento; ":"; FolioS403; ":"; UOC ":"; Folio; ":"; SubFolio*/
		
			sprintf( VarPaso, "%s:%s:%s:%s:%s:%s:%s/%s:%s:%s:%s:%s\n",
				var1->data_p,var2->data_p,var3->data_p,var4->data_p,var5->data_p,
				var6->data_p,var7->data_p,var8->data_p,var9->data_p,var10->data_p,var11->data_p, var12->data_p);
	
		fprintf(ArchivoLog,VarPaso);
		ImagProc++;

	} /* while */
	return (0);
}

/* AVG JUN-2012 FIN	----------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
void Inicia_Borrado_ImgCargadas (int opc ) 
{

	char vartemp[11];

	initialize ();
	strcpy(X_domain, DOMAINIO);
	strcpy(X_Org, ORGANIZATION);

	sprintf ( vartemp, "%s", GetFecha());

	if (opc == 1 ) {
		#ifdef _WIN32
			sprintf(pathArchivoLog , "%s\\HBC406%s.log" , DirBitacoras,vartemp);
		#else
			sprintf(pathArchivoLog , "%s/HBC406%s.log" , DirBitacoras,vartemp);
		#endif
	} else {
		#ifdef _WIN32
			sprintf(pathArchivoLog , "%s\\BC406%s.log" , DirBitacoras,vartemp);
		#else
			sprintf(pathArchivoLog , "%s/BC406%s.log" , DirBitacoras,vartemp);
		#endif
	}
	ArchivoLog = fopen(pathArchivoLog, "w" ) ;
	if( ! ArchivoLog ){
  		printf ("No se creo ArchivoLog %s \n" , pathArchivoLog); 
		free( pathArchivoLog ) ;
		exit( 1 ) ;
	}
	if (opc == 1 ) {
		#ifdef _WIN32
			sprintf(pathArchivoIndices , "%s\\HBIndTemp.txt" , DirCompletas);
		#else
			sprintf(pathArchivoIndices , "%s/HBIndTemp.txt" , DirCompletas);
		#endif
	} else {
		#ifdef _WIN32
			sprintf(pathArchivoIndices , "%s\\BIndTemp.txt" , DirCompletas);
		#else
			sprintf(pathArchivoIndices , "%s/BIndTemp.txt" , DirCompletas);
		#endif
	}
	ArchivoIndTemp = fopen(pathArchivoIndices, "w" ) ;
	if( ! ArchivoIndTemp ){
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( pathArchivoIndices ) ;
		exit( 1 ) ;
	}

	fprintf(ArchivoLog, "Superficie a procesar \n") ;
	sprintf ( vartemp, "%s", GetFecha());
	fprintf(ArchivoLog, "Inicia Proceso : %s \n", vartemp) ;

   	logon(1);

	fprintf(ArchivoLog, "\n\t========== OUTPUT BINDING TEST  FIELDS=========\n") ;
	CargaFiltro (CalificaOnDemand,"'CalificaOnDemand'");
	CargaFiltro (Status,"'Status'");
	CargaFiltro (XFile,"'XfolioS'");
	CargaFiltro (UOC,"'UOC'");
	CargaFiltro (TipoDoc,"'TipoDoc'");
	CargaFiltro (Folio,"'Folio'");
	CargaFiltro (NumCliente,"'NumCliente'");
	CargaFiltro (Contrato,"'Contrato'");
	CargaFiltro (Linea,"'Linea'");
	CargaFiltro (Producto,"'Producto'");
	CargaFiltro (Instrumento,"'Instrumento'");
	CargaFiltro (FolioS403,"'FolioS403'");
	CargaFiltro (FechaOperacion,"'FechaOperacion'");
	CargaFiltro (SubFolio,"'XfolioP'");

	CargaDatos_ImgBorrar( opc );

	fprintf(ArchivoLog, "\t-------------------------------------------------------\n");
	
	fclose( ArchivoIndTemp );
	

	ArchivoIndTemp = fopen(pathArchivoIndices, "r" ) ;
	if( ! ArchivoIndTemp ){
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( ArchivoIndTemp ) ;
		exit( 1 ) ;
	}

	if (opc == 1 ) {
		#ifdef _WIN32
			sprintf(pathArchivoIndices , "%s\\HBIndices.txt" , DirCompletas);
		#else
			sprintf(pathArchivoIndices , "%s/HBIndices.txt" , DirCompletas);
		#endif
	} else {
		#ifdef _WIN32
			sprintf(pathArchivoIndices , "%s\\BIndices.txt" , DirCompletas);
		#else
			sprintf(pathArchivoIndices , "%s/BIndices.txt" , DirCompletas);
		#endif	
	}
	ArchivoIndices = fopen(pathArchivoIndices, "w" ) ;
	if( ! ArchivoIndices ){
  		printf ("No se creo ArchivoIndices %s \n" , pathArchivoIndices); 
		free( pathArchivoIndices ) ;
		exit( 1 ) ;
	}

	FechaOpera(1);
	ProcesaBIndices();

	fflush (ArchivoIndTemp );
	fclose( ArchivoIndTemp );
	fclose( ArchivoIndices );

	
	fflush (ArchivoIndices );
	fclose( ArchivoIndices );
	fprintf(ArchivoLog, "\t-------------------------------------------------------\n");
	fflush (ArchivoLog );
	fclose( ArchivoLog );
	logoff();

}


/*----------------------------------------------------------------------------*/
int
ExtraeImagenes (char *ArcImg)
{   error_typ                   err;
    char                        fname[80];
    ASE_page_range_typ          page_range;
    ASE_migrate_status_typ      status;
    ASE_page_number_typ         pages_avail;
    ASE_request_id_typ          request_id;
    ASE_page_number_typ         pages_in_doc;
    ASE_service_name_typ        cache_name;
    bool                        migrated;
    ASE_session_number_typ      csmh;
    long                        checksum;
    bool                        checksum_exists;
    CSM_object_desc_typ         csm_obj;
    CSM_object_attr_typ         csm_attr;
    CSM_object_handle_typ       csm_oh;
    char*                       buf_p;
    unsigned                    offset;
    unsigned                    len;
    unsigned                    bytes_read;
    long                        fid;
    long                        accumulator;
    short                       retries;
    /*//unsigned short              num_matches;
    //bool                        last_match;
    //DOC_doc_location_desc_typ   doclocs[4];*/
    unsigned short              Bande;
	unsigned long   num_val;

    buf_p = (char*) malloc (BUFSIZE);
    /*if (!buf_p)
        quit (0, "Can't get memory for buffer");
    printf ("First few documents on this system are:\n\n");
    if (err = DOC_find_documents (imsh, ASE_LOCAL_SSN, 100000, GEQ, 4, 
                                  &num_matches, doclocs, &last_match))
        quit (0, "Can't get list of documents on system");
    for (i = 0;  i < num_matches;  i++) {
        printf ("  %u", doclocs[i].doc_id);
    }
    printf ("\n\n");*/
	Bande = 0;

    /*//while (TRUE) {

        //if (aborted)
        //    quit (0, "Program terminated by operator");

        /  * Initiate the retrieval from optical disk */
		num_val=atol(ArcImg);
        page_range.doc_id = num_val; /* promptu ("Document id of document to retrieve"); */
		num_val=atol("1");
        page_range.first_page = num_val; /* promptu ("Page to retrieve"); */
        page_range.last_page = page_range.first_page;
        if (err = DOC_migrate_from_optical_disk (imsh, &page_range, NULL, 
            DOC_EXACT_ASCENDING, ASE_NOTIFY_ASYNCHRONOUS, &status, &pages_avail,
            &request_id, &pages_in_doc, &cache_name)) {
			fprintf(ArchivoLog,"Imagen Id %s error num  %08x\n", ArcImg, err);            
            Bande= 0;			
        }
		else {
			Bande = 1;
		}
 if (Bande ==1) {	
	if   (status == ASE_INTERVENTION_REQ) {
		printf("\nIntervention required by operator. This might due to ");
		printf("an out of osar disk, a disabled osar, slot, or optical drive.\n\n");
	}
	else {
        retries = 0;
        do {
            retries++; 

            /* Wait for retrieval to complete.  Note that a non-zero request id
               is returned if and only if the migrate is in progress (has not
               completed yet) and we must wait for a result.  Note that we 
               could replace the statement "if (request_id)" with:
               "if (status != ASE_ALL_MIGRATED)" */

            if (request_id) {
                do {
                    err = DOC_is_migrated (imsh, request_id, 5, &migrated);
                    if (err) {
                        fprintf(ArchivoLog,"Error %08x calling DOC_is_migrated File %s\n", err,ArcImg);
						return (0); 
					}
                    if (err || aborted) {
                        if (err = DOC_cancel_migrate (imsh, request_id)) {
                        	fprintf(ArchivoLog,"Can't cancel migrate file %s\n", ArcImg);
							return (0); 
						}
                    }
                } while (!migrated);
            }
            else {
                if (status != ASE_ALL_MIGRATED) {
                    /* This will never happen */
					fprintf(ArchivoLog,"Unexpected migrate status file %s Status %u\n", ArcImg, status);
					return (0);
				}
            }

            /* Get the handle for the cache service */
            if (err = IMS_get_svc_handle (imsh, &cache_name, IMS_csm_svc_typ, 
                &csmh)) {
				fprintf(ArchivoLog,"Can't handle for cache file %s\n", ArcImg);
				return (0);
			}

            /* Beginning of copy all pages loop */
            if (err = DOC_get_ssn_of_docs (imsh, &csm_obj.ssn))
                quit (err, "Can't get ssn from doc service");
            csm_obj.id = page_range.doc_id;
            csm_obj.page = page_range.first_page;
			#ifdef _WIN32
				sprintf (fname, "%s\\%s_1.tif", DirCompletas, ArcImg);
			#else
				sprintf (fname, "%s/%s_1.tif", DirCompletas, ArcImg);
			#endif

			#ifdef _WIN32
				if ((fid = open (fname, (_O_BINARY | _O_CREAT | _O_WRONLY | _O_TRUNC), 0666)) < 0) {
					fprintf(ArchivoLog,"Can't create output file %s\n", fname);
					return (0);
				}
			#else
				if ((fid = creat (fname, 0666)) < 0) {
					fprintf(ArchivoLog,"Can't create output file %s\n", fname);
					return (0);
				}
			#endif

            /* Open the object in the cache.  Note that objects can be 
               delete from the cache at the same time as this process is
               adding objects to the cache, so in rare cases the object 
               will not be in the cache at this point, and we may need to 
               retry the migrate. */

            if (err = CSM_open_csum_object (csmh, &csm_obj, CSM_OBJECT_READ,
                &csm_oh, &csm_attr, &checksum_exists, &checksum))
                continue;  /* this stmt goes to end of do..while */

            /* Beginning of loop to copy this page. */

            offset = 0;
            accumulator = 0;
            while (offset < csm_attr.cur_length) {
                len = MIN (BUFSIZE, csm_attr.cur_length - offset);
                if (err = CSM_read_object (csmh, csm_oh, offset, len,
                    buf_p, BUFSIZE, &bytes_read)) {
                    /* Note that we can exit here with the object open,
                       because IMS_logoff (called by quit) will logoff 
                       handles created via IMS_get_svc_handle, and 
                   CSM_logoff will close the open object. */
                    /*quit (err, "Can't read cache object");*/
					fprintf(ArchivoLog,"Can't read cache object file %s\n", fname);
					return (0);

				}
                if (write (fid, buf_p, len) < (int) len) {					
					fprintf(ArchivoLog,"Can't write output file %s\n", fname);
					return (0);
				}

                /* Compute the checksum if one is saved with the document.
                   Note that this step is optional (and time consuming) */

                if (checksum_exists)
                    accumulator = CKS_compute_csum ((long*) buf_p, len, 
                                                    accumulator);
                offset += len;
            }

            /* Close the object with update=FALSE (last parameter).  If
               update were TRUE, then last read time on object would be
               updated in database.  Using FALSE, however, provides better
               performance (note: do NOT use FALSE when creating objects).  */

            if (err = CSM_close_object (csmh, &csm_oh, FALSE)) {
				fprintf(ArchivoLog,"Can't close cache object file %s\n", fname);
				return (0);
			} 
            /* Compare the computed checksum with stored value.
               Note that this step is optional and is done as a safety
               check to insure that the hardware is operating correctly.
               It is not expected that this check will ever show an error */

            if (checksum_exists)
                if (accumulator != checksum) {
                    /* If a checksum error occurs, flag a warning and continue,
                       since most of the image is probably readable. */
                    printf (
                        "Warning: doc %u, page %u:\n\
computed checksum (%08x) != stored checksum(%u)\n",
                        csm_obj.id, csm_obj.page, accumulator, checksum);
                }

            /* Retry entire loop for rare case when object is not in the
               cache due because an aging algorithm deleted it */

        } while (retries < 5 && err == CSM_no_such_object);

        if (err) {
			fprintf(ArchivoLog,"Can't open object file '%s'\n", fname);
			return (0);
		}
        else {
			fprintf(ArchivoLog,"\tPage(s) retrieved and put into file '%s'\n", fname);
			return (1);
		}
	}
 } /* If Bande == 1 */
    /*} / * while (TRUE) */
 return (0);	
}

/*----------------------------------------------------------------------------*/
int
BorraImagenes (char *ArcImg)
{   error_typ                   err;
    ASE_page_range_typ          page_range;
    ASE_service_name_typ        cache_name;
    ASE_session_number_typ      csmh;
    CSM_object_desc_typ         csm_obj;
    char*                       buf_p;
    short                       retries;
	unsigned long				num_val;
	unsigned short              Bande;
	ASE_migrate_status_typ      status;
    ASE_page_number_typ         pages_in_doc;
    ASE_page_number_typ         pages_avail;
    ASE_request_id_typ          request_id;
	bool                        migrated;
	ASE_doc_id_typ				doc_id;

    buf_p = (char*) malloc (BUFSIZE);

	num_val=atol(ArcImg);
    page_range.doc_id = num_val; /*promptu ("Document id of document to retrieve");*/
	num_val=atol("1");
    page_range.first_page = num_val; /* promptu ("Page to retrieve");*/
    page_range.last_page = page_range.first_page;

	doc_id = page_range.doc_id;


    if (err = DOC_migrate_from_optical_disk (imsh, &page_range, NULL, 
        DOC_EXACT_ASCENDING, ASE_NOTIFY_ASYNCHRONOUS, &status, &pages_avail,
        &request_id, &pages_in_doc, &cache_name)) {
		fprintf(ArchivoLog,"Imagen Id %s error num  %08x\n", ArcImg, err);            
        Bande= 0;			
    }
	else {
		Bande = 1;
	}
 if (Bande ==1) {	
	if (status == ASE_INTERVENTION_REQ) {
		printf("\nIntervention required by operator. This might due to ");
		printf("an out of osar disk, a disabled osar, slot, or optical drive.\n\n");
	}
	else {
        retries = 0;
        do {
            retries++; 


            if (request_id) {
                do {
                    err = DOC_is_migrated (imsh, request_id, 5, &migrated);
                    if (err) {
                        fprintf(ArchivoLog,"Error %08x calling DOC_is_migrated File %s\n", err,ArcImg);
						return (0); 
					}
                    if (err || aborted) {
                        if (err = DOC_cancel_migrate (imsh, request_id)) {
                        	fprintf(ArchivoLog,"Can't cancel migrate file %s\n", ArcImg);
							return (0); 
						}
                    }
                } while (!migrated);
            }
            else {
                if (status != ASE_ALL_MIGRATED) {
                    /* This will never happen */
					fprintf(ArchivoLog,"Unexpected migrate status file %s Status %u\n", ArcImg, status);
					return (0);
				}
            }

	        /* Get the handle for the cache service */
            if (err = IMS_get_svc_handle (imsh, &cache_name, IMS_csm_svc_typ, 
                &csmh)) {
				fprintf(ArchivoLog,"Can't handle for cache file %s\n", ArcImg);
				return (0);
			}
			
            /* Beginning of copy all pages loop */
            if (err = DOC_get_ssn_of_docs (imsh, &csm_obj.ssn))
                quit (err, "Can't get ssn from doc service");
            csm_obj.id = page_range.doc_id;
            csm_obj.page = page_range.first_page;
			
          		
			csm_obj.page = CSM_ALL_PAGES;


            if (err = CSM_delete_object(csmh, &csm_obj)) {
				fprintf(ArchivoLog,"Can't delete Object file %s\n", ArcImg);
				return (0);
			} else {
				retries = 5;
			}

            /* Retry entire loop for rare case when object is not in the
               cache due because an aging algorithm deleted it */

        } while (retries < 5 && err == CSM_no_such_object );

        if (err) {
			fprintf(ArchivoLog,"Can't open object file '%s'\n", ArcImg);
			return (0);
		}
        else {
			fprintf(ArchivoLog,"\t Borrado de Archivo de Chace file '%s'\n", ArcImg);
			return (1);
		}
	} /* If status == ASE_INTERVENTION_REQ */
 } /* If Bande == 1 */
 return (0);	
}


/*----------------------------------------------------------------------------*/
void
quit (err, msg)
error_typ   err;
char*       msg;
{
    printf ("\n%s", msg);
    if (err)
        printf (", err=%08x", err);
    printf ("\n");
    if (imsh)
        if (err = IMS_logoff (imsh))
            printf ("Error %08x logging off of IMS\n", err);
    exit (1);
}
/*----------------------------------------------------------------------------*/

/*............................................................................*/
void
initialize ()
{

	char	PathArchConfig[MAXPATH] ;

	if (getenv("HOME") == (char *) NULL)
		#ifdef _WIN32
			strcpy(HOME, "\\opt\\c406\\000"); /* Valor Fijo */
		#else
			strcpy(HOME, "//opt//c406//000"); /* Valor Fijo */
		#endif
	else
	{
		strcpy(HOME, getenv("HOME"));
	}
	
	#ifdef _WIN32
		sprintf( PathArchConfig, "%s%s", HOME, "\\FileNET\\conf\\WExtrFNet.ini");
	#else
		sprintf( PathArchConfig, "%s%s", HOME, "//FileNET//conf//ExtrFNet.ini");
	#endif


	get_profile_string(PathArchConfig, "INICIO", "ImgDominio");
	sprintf(DOMAINIO , "%s" , VarPaso);

	get_profile_string(PathArchConfig, "INICIO", "ImgOrg");
	sprintf(ORGANIZATION , "%s" , VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgCompletas");
	sprintf(DirCompletas , "%s%s", HOME, VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgPartidas");
	sprintf(DirPartidas , "%s%s", HOME, VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgFormatOnD");
	sprintf(DirFormatoOnD , "%s%s", HOME, VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgReportes");
	sprintf(DirReportes , "%s%s", HOME, VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgBitacoras");
	sprintf(DirBitacoras , "%s%s", HOME, VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgTemp");
	sprintf(DirTemporal , "%s%s", HOME, VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "Ejecutables");
	sprintf(DirProgramas , "%s%s", HOME, VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgSigno");
	sprintf(Signo , "%s" , VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgSIRH");
	SIRH=atoi(VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgVigencia");
	Vigencia = atoi(VarPaso);

	get_profile_string(PathArchConfig, pSeccion, "ImgVigencia2");
	Vigencia2 = atoi(VarPaso);
	
	get_profile_string(PathArchConfig, pSeccion, "FolioXFile");
	sprintf(FolioXFile , "%s" , VarPaso);

}
/* stamp 0G^HXCR5Si[p@W:T4KtE\?V@P7OaD[>U;R2IaD`=[MN1JdBa<S6_9H^H^;RITcZ]@WOc<KhZ\?kKP<JaD]PV:P9I`GZ=W7N1HbTmBW6M */

void get_profile_string(char *PathArch, char *Seg, char *VarB) {
	
	FILE	*aini ;

	char	cadena[256] ;
	char	cade[256];
	char	cadepaso[256];
	int		y,paso,Bande ;

	if ((aini = fopen(PathArch, "r")) == NULL) {
		printf( "Cannot open input\n" );
		exit( 1 ) ;
	}	

	fflush(stdin);
	for (y=0;y<256;y++) {
		cadepaso[y]='\x0'; 
		VarPaso[y]='\x0'; 
	}

	paso = strlen(Seg);
	Bande =0;
	do { 
		fgets(cadena, 256, aini);		
		sprintf( cade, "%s" , cadena );
		for (y=0;y<256;y++) {cadepaso[y]='\x0';}
		for (y=0;y<paso;y++) {cadepaso[y]=cade[y+1];}
		if (strcmp(cadepaso, Seg) == 0) {
			for (y=0;y<=strlen(cade);y++) {cadepaso[y]=VarB[y];}
			do {
				fgets(cadena, 256, aini);		
				sprintf( cade, "%s" , cadena );	
				if (strncmp(cadepaso, cade,strlen(cadepaso))== 0) {
					paso = strlen(VarB);
					if ( Bande == 0 ) {
						for (y=0;y<256;y++) {cadepaso[y]='\x0';}
						for (y=0;y<strlen(cade);y++) {
							cadepaso[y]=cade[y+paso+1];
							VarPaso[y]=cade[y+paso+1];
						}
						Bande = 1;
						VarPaso[strlen(VarPaso)-1]='\x0';
					}
				}
			} while ( !feof(aini) && (Bande==0) );
		}
	} while ( !feof(aini) && (Bande==0) );
	fclose (aini) ;
}

void PreparaAmbiente(int opc) {
	char	cade[MAXLINE];

#ifdef _WIN32
	sprintf( cade, " del %s%s " , DirTemporal,"\\*.tif" );	
	system ( cade );
	
	switch (opc) {
	case 1:
		sprintf( cade, " del %s%s " , DirCompletas,"\\*.txt" );	
		system ( cade );
		sprintf( cade, " del %s%s " , DirCompletas,"\\*.tif" );	
		system ( cade );

		sprintf( cade, " del %s%s " , DirPartidas,"\\*.txt" );	
		system ( cade );
		sprintf( cade, " del %s%s " , DirPartidas,"\\*.tif" );	
		system ( cade );

		sprintf( cade, " del %s%s " , DirBitacoras,"\\*.deb" );	
		system ( cade );		
		break;


	case 2:
		sprintf( cade, " del %s%s " , DirCompletas,"\\*.txt" );	
		system ( cade );
		sprintf( cade, " del %s%s " , DirCompletas,"\\*.tif" );	
		system ( cade );

		sprintf( cade, " del %s%s " , DirPartidas,"\\*.txt" );	
		system ( cade );
		sprintf( cade, " del %s%s " , DirPartidas,"\\*.tif" );	
		system ( cade );

		break;

	case 3:
		sprintf( cade, " del %s%s " , DirCompletas,"\\*.tif" );	
		system ( cade );

		sprintf( cade, " del %s%s " , DirFormatoOnD,"\\*.cif" );	
		system ( cade );
		sprintf( cade, " del %s%s " , DirFormatoOnD,"\\*.ind" );	
		system ( cade );
		sprintf( cade, " del %s%s " , DirFormatoOnD,"\\*.out" );	
		system ( cade );		
		sprintf( cade, " del %s%s " , DirFormatoOnD,"\\*.ctr" );	
		system ( cade );		
		break;

	case 4:
		sprintf( cade, " del %s%s " , DirProgramas,"\\*.tif" );	
		system ( cade );
		break;

	case 5:
		sprintf( cade, " del %s%s " , DirCompletas,"\\*.tif" );	
		system ( cade );
		break;
	}
#else
	sprintf( cade, " rm %s%s " , DirTemporal,"/*.tif" );	
	system ( cade );
	
	switch (opc) {
	case 1:
		sprintf( cade, " rm %s%s " , DirCompletas,"/*.txt" );	
		system ( cade );
		sprintf( cade, " rm %s%s " , DirCompletas,"/*.tif" );	
		system ( cade );

		sprintf( cade, " rm %s%s " , DirPartidas,"/*.txt" );	
		system ( cade );
		sprintf( cade, " rm %s%s " , DirPartidas,"/*.tif" );	
		system ( cade );

		sprintf( cade, " rm %s%s " , DirBitacoras,"/*.deb" );	
		system ( cade );		
		break;


	case 2:
		sprintf( cade, " rm %s%s " , DirCompletas,"/*.txt" );	
		system ( cade );
		sprintf( cade, " rm %s%s " , DirCompletas,"/*.tif" );	
		system ( cade );

		sprintf( cade, " rm %s%s " , DirPartidas,"/*.txt" );	
		system ( cade );
		sprintf( cade, " rm %s%s " , DirPartidas,"/*.tif" );	
		system ( cade );

		break;

	case 3:
		sprintf( cade, " rm %s%s " , DirCompletas,"/*.tif" );	
		system ( cade );

		sprintf( cade, " find %s -name \'%s\' -mtime +7 -exec rm {} \\;", DirFormatoOnD,"*.cif" );	
		system ( cade );
		sprintf( cade, " find %s -name \'%s\' -mtime +7 -exec rm {} \\;", DirFormatoOnD,"*.ind" );	
		system ( cade );
		sprintf( cade, " find %s -name \'%s\' -mtime +7 -exec rm {} \\;", DirFormatoOnD,"*.out" );	
		system ( cade );		
		sprintf( cade, " find %s -name \'%s\' -mtime +7 -exec rm {} \\;", DirFormatoOnD,"*.ctr" );	
		system ( cade );		
		break;

	case 4:
		sprintf( cade, " rm %s%s " , DirProgramas,"/*.tif" );	
		system ( cade );
		break;

	case 5:
		sprintf( cade, " rm %s%s " , DirCompletas,"/*.tif" );	
		system ( cade );
		break;

	}
#endif
}

/*			--------------------
 *			    logon
 *			--------------------
 */
void
logon (int opc)
{
	error_typ		err;
	ASE_service_name_typ    ase_object;
	SQI_sqlca_typ		sqlca;
	ASE_service_name_typ    ims_name;
	/*char	spassword[MAXLINE];*/
	printf( "\n\t Valores de Inicio \n" );
	ValoresInicio();
	sleep(1); /* Pausa si no es la primera vez */
	sprintf ( spassword, "%s", ConectaSapuf("7"));

	strcpy(ase_object.object,"SQLServer");
	strcpy(ase_object.domain, X_domain);
 	strcpy(ase_object.organization, X_Org);

 	printf( "\n\t Primer Intento \n" );
	if ( err = IMS_logon (USER, spassword, TERMINAL, &ase_object,
						 IMS_version, &imsh) )
	{
		printf( "\n\t Segundo Intento \n");
		if ( err = IMS_logon (USER2, PASSWORD2, TERMINAL, &ase_object,
						 IMS_version, &imsh) )
		{
			quit (err, "Can't log on to IMS");
		}
		else
		{
			sprintf ( spassword, "%s", PASSWORD2);
			printf( "\n\t IMS_logon: successful to %s\n", X_domain);

			if ( err = SQI_full_use_logon (&ase_object, &sqih, &sqlca) )
			{
				quit (err, "Can't log on to SQI");
			}
			else
			{
				printf( "\n\t SQI_full_use_logon: successful\n" );
			}
		}

		signal (SIGINT, interrupt_handler);
		ims_name.object[0] = '\0';
		strcpy (ims_name.domain, X_domain);
		strcpy (ims_name.organization, X_Org);
		if (err = IMS_logon (USER2, PASSWORD2, TERMINAL, &ims_name, IMS_version, &imsh)) 
		{
			quit (err, "Can't log on to IMS");
		}
		printf( "\n\t IMS_logon: successful to %s\n", X_domain);
		return;
	}
	else
	{
		printf( "\n\t IMS_logon: successful to %s\n", X_domain);

		if ( err = SQI_full_use_logon (&ase_object, &sqih, &sqlca) )
		{
			quit (err, "Can't log on to SQI");
		}
		else
		{
			printf( "\n\t SQI_full_use_logon: successful\n" );
		}
	}

	signal (SIGINT, interrupt_handler);
	ims_name.object[0] = '\0';

    strcpy (ims_name.domain, X_domain);
    strcpy (ims_name.organization, X_Org);
    if (err = IMS_logon (USER, spassword, TERMINAL, &ims_name, IMS_version, &imsh)) 
	{
        quit (err, "Can't log on to IMS");
	}
}


/*			--------------------
 *			   logoff
 *			--------------------
 *	Logoff the current session.
 */
void
logoff ()
{
	error_typ	err = 0;
	SQI_sqlca_typ	sqlca;
 
	if ( err = SQI_logoff( &sqih, &sqlca ) )
	{
		quit (err, "SQI logoff failed");
	}
	else 
	{
		printf( "\n\tSQI logoff: successful!\n");
	}
	if ( err = IMS_logoff( imsh ) )
	{
		quit (err, "IMS logoff failed");
	}
	else 
	{
		printf( "\n\tIMS logoff: successful!\n");
	}

}

void CargaFiltro(char *Campo, char *Filtro){

	char				arg1[1920];
	SQI_cursor_handle_typ	ch;
	SQI_sqlca_typ		sqlca;
	var_typ				var1;
	char				varpaso[MAX_VAR_NAME_SIZE];

	/* 
	 * Create a cursor for selecting rows from the table.
	 */ 
	
	fprintf(ArchivoLog, "\t --> 1. Cargando Filtros...%s<-- \n", Filtro) ;

	if ( BASEDATOS==1 ) {
		/* Query SQL */
		sprintf( arg1, "Select f_columnname from f_sw.user_index " );
		strcat(arg1," Where f_indexname=");
	} else {
		/* Query SQL */
		sprintf( arg1, "SELECT UPPER(F_COLUMNNAME) FROM F_SW.USER_INDEX " );		
		strcat(arg1," WHERE F_INDEXNAME=");
	}
	strcat(arg1,  Filtro);

	create_cursor( arg1, &ch, &sqlca );

	memset( &var1, 0, sizeof(var_typ) );

	strcpy( var1.varname, "var1" );
	var1.data_p	= (char *)malloc(30 * sizeof(char));
	var1.pos 	= 1;
	var1.type  	= SQI_ASCII;
	var1.size	= 30 * sizeof(char);
 
	memset( &sqlca, 0, sizeof(SQI_sqlca_typ) );
	bindresult( &ch, &var1, &sqlca );

	exec( &ch, TRUE, 1, FALSE, &sqlca );
	
	do_fetch2( &var1, &ch, &varpaso );

	sprintf( Campo, varpaso);

	SQI_drop_cursor( sqih, &ch, &sqlca );

}

void FechaOpera( int opc ){

	char					arg1[1920];
	SQI_cursor_handle_typ	ch;
	SQI_sqlca_typ			sqlca;
	var_typ					var1,var2;
	char					varpaso[MAX_VAR_NAME_SIZE];

	/* 
	 * Create a cursor for selecting rows from the table.
	 */ 
	if (opc==1) {
		if ( BASEDATOS==1 ) {
			/* Query SQL */
			sprintf( arg1, "Select X=CONVERT(char(10),getdate(),101) " );
		} else {
			/* Query Oracle */
			sprintf( arg1, "Select TO_CHAR(SYSDATE,'MM/DD/YYYY') \"X\" from DUAL " );	
		}

		create_cursor( arg1, &ch, &sqlca );

		memset( &var1, 0, sizeof(var_typ) );

		strcpy( var1.varname, "var1" );
		var1.data_p	= (char *)malloc(30 * sizeof(char));
		var1.pos 	= 1;
		var1.type  	= SQI_ASCII;
		var1.size	= 30 * sizeof(char);
 
		memset( &sqlca, 0, sizeof(SQI_sqlca_typ) );
		bindresult( &ch, &var1, &sqlca );

		exec( &ch, TRUE, 1, FALSE, &sqlca );
		
		do_fetch2( &var1, &ch, &varpaso );

		sprintf(FechaFNetOp, varpaso);
	}
	fprintf(ArchivoLog, "\t ---> Fecha Operación %s...<---\n", FechaFNetOp  );

	if ( BASEDATOS==1 ) {
		/* Query SQL */
		sprintf( arg1, "Select X=CONVERT(int,CONVERT(datetime,'%s'))-CONVERT(int,CONVERT(Datetime,'01/01/1970'))",FechaFNetOp );
	} else {
		/* Query Oracle */
		sprintf( arg1, "SELECT TO_DATE('%s','mm/dd/yyyy')-TO_DATE('1/1/1970','mm/dd/yyyy') \"X\" from DUAL", FechaFNetOp);
	}
	
	create_cursor( arg1, &ch, &sqlca );

	memset( &var2, 0, sizeof(var_typ) );

	strcpy( var2.varname, "var2" );
	var2.data_p	= (char *)malloc(30 * sizeof(char));
	var2.pos 	= 1;
	var2.type  	= SQI_ASCII;
	var2.size	= 30 * sizeof(char);

	memset( &sqlca, 0, sizeof(SQI_sqlca_typ) );
	bindresult( &ch, &var2, &sqlca );

	exec( &ch, TRUE, 1, FALSE, &sqlca );
	
	do_fetch2( &var2, &ch, &varpaso );	
	
	FechaFileNET = atoi(varpaso);

	fprintf(ArchivoLog, "\t ---> Fecha Operación FileNET %d...<---\n", FechaFileNET  );

	SQI_drop_cursor( sqih, &ch, &sqlca );
}

void CargaDatos( int opc ) {

	char				arg1[1920];
	SQI_cursor_handle_typ	ch;
	SQI_sqlca_typ		sqlca;
	var_typ				var1;
	var_typ				var2;
	var_typ				var3;
	var_typ				var4;
	var_typ				var5;
	var_typ				var6;
	var_typ				var7;
	var_typ				var8;
	var_typ				var9;
	var_typ				var10;
	var_typ				var11;
	var_typ				var12;

	/* 
	 * Create a cursor for selecting rows from the table.
	 */ 
	fprintf(ArchivoLog, "\n\t========== SELECIONANDO IMAGENES  =========\n") ;
	fprintf(ArchivoLog, "\t ---> 1. Create a cursor for selecting rows from the table...<-- \n") ;

	/*Print #2, DocumentID; ":"; NumCliente; ":"; TipoDoc; ":"; XFolio; ":"; _
    Contrato; ":"; Linea; ":"; Producto; "/"; Instrumento; ":"; FolioS403; ":"; FolioUOC; ":"; SubFolio*/

	if ( BASEDATOS==1 ) {
		/* Creación del Select con los campos utilizados (SQL) */
		sprintf( arg1, "Select a.f_docnumber, " );	
		sprintf( VarPaso, "isnull(a.%s,0),",NumCliente);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",TipoDoc);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",XFile);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Contrato);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Linea);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Producto);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Instrumento);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",FolioS403);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",UOC);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0)",Folio);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0)",SubFolio);
		strcat(arg1, VarPaso);
	} else {
		/* Creación del Select con los campos utilizados (Oracle) */
		sprintf( arg1, "Select a.f_docnumber, " );	
		sprintf( VarPaso, "NVL(a.%s,0),",NumCliente);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",TipoDoc);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",XFile);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Contrato);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Linea);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Producto);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Instrumento);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",FolioS403);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",UOC);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Folio);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0)",SubFolio);
		strcat(arg1, VarPaso);
	}
	strcat(arg1," From f_sw.doctaba a, f_sw.document_class b " );
	/* Armado de Where*/
	strcat(arg1," Where a.f_docclassnumber=b.f_docclassnumber ");
	strcat(arg1," And f_docclassname='ExpedientesDC' ");
	/* CalificaOndemand */
	sprintf( VarPaso, " And a.%s='1'",CalificaOnDemand);
	strcat(arg1, VarPaso);

	switch ( opc) {
		case 1:
			/* Status */
			sprintf( VarPaso, " And a.%s<'3'",Status);
			strcat(arg1, VarPaso);
		break;
		case 2:
			/* Status */
			sprintf( VarPaso, " And a.%s='3'",Status);
			strcat(arg1, VarPaso);
			/* FechaOperacion */
			sprintf( VarPaso, " And a.%s='%d'", FechaOperacion, FechaFileNET);
			strcat(arg1, VarPaso);
		break;
	}

	/* UOC */
	sprintf( VarPaso, " And a.%s %s '%d'",UOC,Signo,SIRH);
	strcat(arg1, VarPaso);
	/* XfolioS */
	sprintf( VarPaso, " And a.%s Is Not Null ",XFile);
	strcat(arg1, VarPaso);

	create_cursor( arg1, &ch, &sqlca );

	memset( &var1, 0, sizeof(var_typ) );
	memset( &var2, 0, sizeof(var_typ) );
	memset( &var3, 0, sizeof(var_typ) );
	memset( &var4, 0, sizeof(var_typ) );
	memset( &var5, 0, sizeof(var_typ) );
	memset( &var6, 0, sizeof(var_typ) );
	memset( &var7, 0, sizeof(var_typ) );
	memset( &var8, 0, sizeof(var_typ) );
	memset( &var9, 0, sizeof(var_typ) );
	memset( &var10, 0, sizeof(var_typ) );
	memset( &var11, 0, sizeof(var_typ) );
	memset( &var12, 0, sizeof(var_typ) );

	/*#define SQI_boolean        1       / * TRUE or FALSE */
	/*#define SQI_byte           2       / * signed two's complement 8 bit quantity */
	/*#define SQI_unsigned_byte  3       / * unsigned 8 bit quantity */
	/*#define SQI_short          4       / * signed two's complement 16 bit quantity */
	/*#define SQI_unsigned_short 5       / * unsigned 16 bit quantity */
	/*#define SQI_long           6       / * signed two's complement 32 bit quantity */
	/*#define SQI_unsigned_long  7       / * unsigned 32 bit quantity */
	/*#define SQI_FPnum          8       / * FileNet floating point number */
	/*#define SQI_ASCII          9       / * ASCII string data ( null terminating ) */
	/*#define SQI_date           10      / * FileNet encoded date */
	/*#define SQI_time           11      / * FileNet encoded date and time */
	/*#define SQI_menu           12      / * integer values encoding string */
	/*#define SQI_text_object    13      / * lots of characters, with length */
	/*#define SQI_binary_object  14      / * lots of bytes (BLOB), with length */

	/* ******* Variables que se asignan de acuerdo al orden de los campos del Select ******* */

	strcpy( var1.varname, "var1" );
	var1.data_p	= (char *)malloc(30 * sizeof(char));
	var1.pos 	= 1;
	var1.type  	= SQI_ASCII;
	var1.size	= 30 * sizeof(char);

	strcpy( var2.varname, "var2" );
	var2.data_p	= (char *)malloc(30 * sizeof(char));
	var2.pos 	= 2;
	var2.type  	= SQI_ASCII;
	var2.size	= 30 * sizeof(char);

	strcpy( var3.varname, "var3" );
	var3.data_p	= (char *)malloc(30 * sizeof(char));
	var3.pos 	= 3;
	var3.type  	= SQI_ASCII;
	var3.size	= 30 * sizeof(char);

	strcpy( var4.varname, "var4" );
	var4.data_p	= (char *)malloc(30 * sizeof(char));
	var4.pos 	= 4;
	var4.type  	= SQI_ASCII;
	var4.size	= 30 * sizeof(char);

	strcpy( var5.varname, "var5" );
	var5.data_p	= (char *)malloc(30 * sizeof(char));
	var5.pos 	= 5;
	var5.type  	= SQI_ASCII;
	var5.size	= 30 * sizeof(char);

	strcpy( var6.varname, "var6" );
	var6.data_p	= (char *)malloc(30 * sizeof(char));
	var6.pos 	= 6;
	var6.type  	= SQI_ASCII;
	var6.size	= 30 * sizeof(char);

	strcpy( var7.varname, "var7" );
	var7.data_p	= (char *)malloc(30 * sizeof(char));
	var7.pos 	= 7;
	var7.type  	= SQI_ASCII;
	var7.size	= 30 * sizeof(char);

	strcpy( var8.varname, "var8" );
	var8.data_p	= (char *)malloc(30 * sizeof(char));
	var8.pos 	= 8;
	var8.type  	= SQI_ASCII;
	var8.size	= 30 * sizeof(char);

	strcpy( var9.varname, "var9" );
	var9.data_p	= (char *)malloc(30 * sizeof(char));
	var9.pos 	= 9;
	var9.type  	= SQI_ASCII;
	var9.size	= 30 * sizeof(char);

	strcpy( var10.varname, "var10" );
	var10.data_p	= (char *)malloc(30 * sizeof(char));
	var10.pos 	= 10;
	var10.type  	= SQI_ASCII;
	var10.size	= 30 * sizeof(char);

	strcpy( var11.varname, "var11" );
	var11.data_p	= (char *)malloc(30 * sizeof(char));
	var11.pos 	= 11;
	var11.type  	= SQI_ASCII;
	var11.size	= 30 * sizeof(char);
	
	strcpy( var12.varname, "var12" );
	var12.data_p	= (char *)malloc(30 * sizeof(char));
	var12.pos 	= 12;
	var12.type  	= SQI_ASCII;
	var12.size	= 30 * sizeof(char);

	memset( &sqlca, 0, sizeof(SQI_sqlca_typ) );

	bindresult( &ch, &var1, &sqlca );	
	bindresult( &ch, &var2, &sqlca );
	bindresult( &ch, &var3, &sqlca );
	bindresult( &ch, &var4, &sqlca );
	bindresult( &ch, &var5, &sqlca );
	bindresult( &ch, &var6, &sqlca );
	bindresult( &ch, &var7, &sqlca );
	bindresult( &ch, &var8, &sqlca );
	bindresult( &ch, &var9, &sqlca );
	bindresult( &ch, &var10, &sqlca );
	bindresult( &ch, &var11, &sqlca );
	bindresult( &ch, &var12, &sqlca );

	printf("\t ---> 2. Exececuting cursor for selecting rows from the table...<---\n");

	exec( &ch, TRUE, 1, FALSE, &sqlca );

	printf( "\n\t ---> 3. We will fetch all the rows and display...<---\n" );

	do_fetch( &var1, &var2, &var3, &var4, &var5, &var6, &var7, &var8, &var9, &var10, &var11, &var12, &ch, &FolioXFile );

	SQI_drop_cursor( sqih, &ch, &sqlca );

	fprintf(ArchivoLog, "\t ---> Imagenes a procesar: %d \n ",ImagProc);

}


void CargaDatos_ImgBorrar( int opc ) {

	char				arg1[1920];
	SQI_cursor_handle_typ	ch;
	SQI_sqlca_typ		sqlca;
	var_typ				var1;
	var_typ				var2;
	var_typ				var3;
	var_typ				var4;
	var_typ				var5;
	var_typ				var6;
	var_typ				var7;
	var_typ				var8;
	var_typ				var9;
	var_typ				var10;
	var_typ				var11;
	var_typ				var12;

	/* 
	 * Create a cursor for selecting rows from the table.
	 */ 
	fprintf(ArchivoLog, "\n\t========== SELECIONANDO IMAGENES  =========\n") ;
	fprintf(ArchivoLog, "\t ---> 1. Create a cursor for selecting rows from the table...<-- \n") ;

	/*Print #2, DocumentID; ":"; NumCliente; ":"; TipoDoc; ":"; XFolio; ":"; _
    Contrato; ":"; Linea; ":"; Producto; "/"; Instrumento; ":"; FolioS403; ":"; FolioUOC; "-"; SubFolio*/

	if ( BASEDATOS==1 ) {
		/* Creación del Select con los campos utilizados (SQL) */
		sprintf( arg1, "Select a.f_docnumber, " );	
		sprintf( VarPaso, "isnull(a.%s,0),",NumCliente);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",TipoDoc);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",XFile);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Contrato);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Linea);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Producto);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",Instrumento);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",FolioS403);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0),",UOC);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0)",Folio);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "isnull(a.%s,0)",SubFolio);
		strcat(arg1, VarPaso);
	} else {
		/* Creación del Select con los campos utilizados (Oracle) */
		sprintf( arg1, "Select a.f_docnumber, " );	
		sprintf( VarPaso, "NVL(a.%s,0),",NumCliente);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",TipoDoc);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",XFile);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Contrato);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Linea);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Producto);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Instrumento);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",FolioS403);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",UOC);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0),",Folio);
		strcat(arg1, VarPaso);
		sprintf( VarPaso, "NVL(a.%s,0)",SubFolio);
		strcat(arg1, VarPaso);
	}
	strcat(arg1," From f_sw.doctaba a, f_sw.document_class b " );
	/* Armado de Where*/
	strcat(arg1," Where a.f_docclassnumber=b.f_docclassnumber ");
	strcat(arg1," And f_docclassname='ExpedientesDC' ");
	if (opc == 2 ) { /* Selecciona Img Procesadas y Cargadas en Ondemand */
		/* CalificaOndemand */
		sprintf( VarPaso, " And a.%s='1'",CalificaOnDemand);
		strcat(arg1, VarPaso);
		/* Status */
		sprintf( VarPaso, " And a.%s='3'",Status);
		strcat(arg1, VarPaso);
		/* UOC */
		sprintf( VarPaso, " And a.%s %s '%d'",UOC,Signo,SIRH);
		strcat(arg1, VarPaso);
		/* FechaOperacion */
		sprintf( VarPaso, " And ROUND(MONTHS_BETWEEN(SYSDATE,a.%s+TO_DATE('1/1/1970','mm/dd/yyyy')),0)>%d", FechaOperacion, Vigencia2);
		strcat(arg1, VarPaso);
	} else { /* Selecciona Img  No Procesadas y ni Cargadas en Ondemand con big 6 meses */
		/* Status */
		sprintf( VarPaso, " And a.%s<>'3'",Status);
		strcat(arg1, VarPaso);
		/* UOC */
		sprintf( VarPaso, " And a.%s %s '%d'",UOC,Signo,SIRH);
		strcat(arg1, VarPaso);
		/* FechaOperacion */
		sprintf( VarPaso, " And ROUND(MONTHS_BETWEEN(SYSDATE,a.F_ENTRYDATE+TO_DATE('1/1/1970','mm/dd/yyyy')),0)>%d", Vigencia);
		strcat(arg1, VarPaso);
	}

	create_cursor( arg1, &ch, &sqlca );

	memset( &var1, 0, sizeof(var_typ) );
	memset( &var2, 0, sizeof(var_typ) );
	memset( &var3, 0, sizeof(var_typ) );
	memset( &var4, 0, sizeof(var_typ) );
	memset( &var5, 0, sizeof(var_typ) );
	memset( &var6, 0, sizeof(var_typ) );
	memset( &var7, 0, sizeof(var_typ) );
	memset( &var8, 0, sizeof(var_typ) );
	memset( &var9, 0, sizeof(var_typ) );
	memset( &var10, 0, sizeof(var_typ) );
	memset( &var11, 0, sizeof(var_typ) );
	memset( &var12, 0, sizeof(var_typ) );
	

	/*#define SQI_boolean        1       / * TRUE or FALSE */
	/*#define SQI_byte           2       / * signed two's complement 8 bit quantity */
	/*#define SQI_unsigned_byte  3       / * unsigned 8 bit quantity */
	/*#define SQI_short          4       / * signed two's complement 16 bit quantity */
	/*#define SQI_unsigned_short 5       / * unsigned 16 bit quantity */
	/*#define SQI_long           6       / * signed two's complement 32 bit quantity */
	/*#define SQI_unsigned_long  7       / * unsigned 32 bit quantity */
	/*#define SQI_FPnum          8       / * FileNet floating point number */
	/*#define SQI_ASCII          9       / * ASCII string data ( null terminating ) */
	/*#define SQI_date           10      / * FileNet encoded date */
	/*#define SQI_time           11      / * FileNet encoded date and time */
	/*#define SQI_menu           12      / * integer values encoding string */
	/*#define SQI_text_object    13      / * lots of characters, with length */
	/*#define SQI_binary_object  14      / * lots of bytes (BLOB), with length */

	/* ******* Variables que se asignan de acuerdo al orden de los campos del Select ******* */

	strcpy( var1.varname, "var1" );
	var1.data_p	= (char *)malloc(30 * sizeof(char));
	var1.pos 	= 1;
	var1.type  	= SQI_ASCII;
	var1.size	= 30 * sizeof(char);

	strcpy( var2.varname, "var2" );
	var2.data_p	= (char *)malloc(30 * sizeof(char));
	var2.pos 	= 2;
	var2.type  	= SQI_ASCII;
	var2.size	= 30 * sizeof(char);

	strcpy( var3.varname, "var3" );
	var3.data_p	= (char *)malloc(30 * sizeof(char));
	var3.pos 	= 3;
	var3.type  	= SQI_ASCII;
	var3.size	= 30 * sizeof(char);

	strcpy( var4.varname, "var4" );
	var4.data_p	= (char *)malloc(30 * sizeof(char));
	var4.pos 	= 4;
	var4.type  	= SQI_ASCII;
	var4.size	= 30 * sizeof(char);

	strcpy( var5.varname, "var5" );
	var5.data_p	= (char *)malloc(30 * sizeof(char));
	var5.pos 	= 5;
	var5.type  	= SQI_ASCII;
	var5.size	= 30 * sizeof(char);

	strcpy( var6.varname, "var6" );
	var6.data_p	= (char *)malloc(30 * sizeof(char));
	var6.pos 	= 6;
	var6.type  	= SQI_ASCII;
	var6.size	= 30 * sizeof(char);

	strcpy( var7.varname, "var7" );
	var7.data_p	= (char *)malloc(30 * sizeof(char));
	var7.pos 	= 7;
	var7.type  	= SQI_ASCII;
	var7.size	= 30 * sizeof(char);

	strcpy( var8.varname, "var8" );
	var8.data_p	= (char *)malloc(30 * sizeof(char));
	var8.pos 	= 8;
	var8.type  	= SQI_ASCII;
	var8.size	= 30 * sizeof(char);

	strcpy( var9.varname, "var9" );
	var9.data_p	= (char *)malloc(30 * sizeof(char));
	var9.pos 	= 9;
	var9.type  	= SQI_ASCII;
	var9.size	= 30 * sizeof(char);

	strcpy( var10.varname, "var10" );
	var10.data_p	= (char *)malloc(30 * sizeof(char));
	var10.pos 	= 10;
	var10.type  	= SQI_ASCII;
	var10.size	= 30 * sizeof(char);

	strcpy( var11.varname, "var11" );
	var11.data_p	= (char *)malloc(30 * sizeof(char));
	var11.pos 	= 11;
	var11.type  	= SQI_ASCII;
	var11.size	= 30 * sizeof(char);
	
	strcpy( var12.varname, "var12" );
	var12.data_p	= (char *)malloc(30 * sizeof(char));
	var12.pos 	= 12;
	var12.type  	= SQI_ASCII;
	var12.size	= 30 * sizeof(char);

	memset( &sqlca, 0, sizeof(SQI_sqlca_typ) );

	bindresult( &ch, &var1, &sqlca );	
	bindresult( &ch, &var2, &sqlca );
	bindresult( &ch, &var3, &sqlca );
	bindresult( &ch, &var4, &sqlca );
	bindresult( &ch, &var5, &sqlca );
	bindresult( &ch, &var6, &sqlca );
	bindresult( &ch, &var7, &sqlca );
	bindresult( &ch, &var8, &sqlca );
	bindresult( &ch, &var9, &sqlca );
	bindresult( &ch, &var10, &sqlca );
	bindresult( &ch, &var11, &sqlca );
	bindresult( &ch, &var12, &sqlca );

	printf("\t ---> 2. Exececuting cursor for selecting rows from the table...<---\n");

	exec( &ch, TRUE, 1, FALSE, &sqlca );

	printf( "\n\t ---> 3. We will fetch all the rows and display...<---\n" );

	do_fetch( &var1, &var2, &var3, &var4, &var5, &var6, &var7, &var8, &var9, &var10, &var11, &var12, &ch, &FolioXFile);

	SQI_drop_cursor( sqih, &ch, &sqlca );

	fprintf(ArchivoLog, "\t ---> Imagenes a procesar: %d \n ",ImagProc);

}


/*			--------------------
 *			  bindvalue
 *			--------------------
 *
 *     bindvalue binds an input host variable to a statement.
 *
 */
error_typ
bindvalue ( ch, id, var_p )
SQI_cursor_handle_typ	*ch;
char			id[];
var_typ			*var_p;
{
	error_typ	err=0;
	SQI_sqlca_typ	sqlca;

	if ( strlen ( id ) > MAX_SUB_NAME_SIZE  )
	{
		printf ( "maximum length of identifer is %d\n", MAX_SUB_NAME_SIZE );
		return(err);
	}
	else if ( !strlen ( id ) )
	{
		printf ( "iendtifier expected\n" );
		return(err);
	}

	printf( "\tNow binding var [%s] to identifier [%s]", var_p->varname, id );

	err = SQI_bind_value ( sqih, *ch, id, var_p->type, var_p->size, 
							var_p->data_p, &sqlca );
	if ( err )
	{
		printf( "SQI_bind_value: error <%d,%d,%d>\n",
           		err_category(err), err_function(err), err_number(err));
		return ( err );
	}
	else
	{
        printf( "... bound successfully!\n" );
	return (err);
	}
}

/*			--------------------
 *			  bindresult
 *			--------------------
 *
 *     bindresult bind an output host variable to a statement.
 *
 */
error_typ
bindresult ( ch, var_p, sqlca_p, var_list )
SQI_cursor_handle_typ	*ch;
var_typ			*var_p;
SQI_sqlca_typ		*sqlca_p;
var_typ 		*var_list;
{
	error_typ     err;

	printf( "\tBindresult [var=%s]; cursor = [%u]", var_p->varname, *ch );

	if ( err = SQI_bind_result( sqih, *ch, var_p->pos, var_p->data_p, 
				    var_p->type, var_p->size, 
				    &var_p->ind, sqlca_p ) )

	{
		printf( "SQI_bind_result: error <%d,%d,%d>\n",
           		err_category(err), err_function(err), err_number(err));
		return ( err );
	}
	else
	{
        printf( "... bound successfully!\n" );
	}
	return (0);
}

/*			--------------------
 *			   do_fetch
 *			--------------------
 *
 */
error_typ
do_fetch ( var1, var2, var3, var4, var5, var6, var7, var8, var9, var10, var11, var12, ch, FolioXFile )
var_typ		*var1, *var2, *var3, *var4, *var5, *var6, *var7, *var8, *var9, *var10, *var11, *var12;
SQI_cursor_handle_typ	*ch;
{
	short		count = 0;
	error_typ	err = 0;
	SQI_sqlca_typ	sqlca;	
	ImagProc =0;
	unsigned long x;
	fprintf(ArchivoLog,"\t---> Procesando Registros <---\n");
	printf( "\n\t ----> Fetch the data of query...<----\n\n" );

	while ( !err )
	{
		if ( err = SQI_fetch( sqih, *ch, &sqlca) )
   		{ 
   	  		if ( err_number(err) == SQI_END_OF_FETCH ) 
    		{ 
				return(err);
     		}
	   		if (err != SQI_ERR_OUTPUT_WARNING) 
   	  		{ 
				return(err);
   	  		}
			printf( "SQI_fetch: error <%d,%d,%d>\n",
            		err_category(err), err_function(err), err_number(err));
			return ( err );
   		}
		
		/*Print #2, DocumentID; ":"; NumCliente; ":"; TipoDoc; ":"; XFolio; ":"; _
		Contrato; ":"; Linea; ":"; Producto; "/"; Instrumento; ":"; FolioS403; ":"; FolioUOC; "-"; SubFolio*/
		

		/*printf ( "\t output row[%d]: %s(%s, type(%d), bytes(%u) )\n", 
			 ++count, var1->varname, var1->data_p, var1->type, 
			 var1->size );
		printf ( "\t output row[%d]: %s(%s, type(%d), bytes(%u) )\n", 
			 ++count, var2->varname, var2->data_p, var2->type, 
			 var2->size );*/		
			 
		sprintf(VarPaso , "%s" , var4->data_p);
		fprintf(ArchivoLog, "\t Valor del Folio XFile : %s \n", VarPaso);
		if (atoi(VarPaso)>0) 
		{ 
			sprintf(VarPaso , "%s" , var12->data_p);
			fprintf(ArchivoLog, "\t Valor del SubFolio : %s \n", VarPaso);
			if (atoi(VarPaso)>0) 
			{
				sprintf( VarPaso, "%s:%s:%s:%s:%s:%s:%s/%s:%s:%s%s-%s\n",
				var1->data_p,var2->data_p,var3->data_p,var4->data_p,var5->data_p,
				var6->data_p,var7->data_p,var8->data_p,var9->data_p,var10->data_p,var11->data_p,var12->data_p);
			} else {
				sprintf( VarPaso, "%s:%s:%s:%s:%s:%s:%s/%s:%s:%s%s\n",
				var1->data_p,var2->data_p,var3->data_p,var4->data_p,var5->data_p,
				var6->data_p,var7->data_p,var8->data_p,var9->data_p,var10->data_p,var11->data_p);
			}
		} else {
			sprintf(VarPaso , "%s" , var12->data_p);
			fprintf(ArchivoLog, "\t Valor del SubFolio : %s \n", VarPaso);
			if (atoi(VarPaso)>0) 
			{
				fprintf(ArchivoLog, "\t Se Remplaza Valor del Folio XFile por  : %s \n", FolioXFile);
				sprintf( VarPaso, "%s:%s:%s:%s:%s:%s:%s/%s:%s:%s%s-%s\n",
				var1->data_p,var2->data_p,var3->data_p,FolioXFile,var5->data_p,
				var6->data_p,var7->data_p,var8->data_p,var9->data_p,var10->data_p,var11->data_p,var12->data_p);
			} else {
				fprintf(ArchivoLog, "\t Se Remplaza Valor del Folio XFile por  : %s \n", FolioXFile);
				sprintf( VarPaso, "%s:%s:%s:%s:%s:%s:%s/%s:%s:%s%s\n",
				var1->data_p,var2->data_p,var3->data_p,FolioXFile,var5->data_p,
				var6->data_p,var7->data_p,var8->data_p,var9->data_p,var10->data_p,var11->data_p);
			}
		}	
		fprintf(ArchivoIndTemp,VarPaso);
		ImagProc++;

	} /* while */
	return (0);
}

/*			--------------------
 *			   do_fetch2
 *			--------------------
 *
 */
error_typ
do_fetch2 ( var1, ch, varpaso )
var_typ			*var1;
SQI_cursor_handle_typ	*ch;
{
	short		count = 0;
	error_typ	err = 0;
	SQI_sqlca_typ	sqlca;

	while ( !err )
	{
		if ( err = SQI_fetch( sqih, *ch, &sqlca) )
   		{ 
   	  		if ( err_number(err) == SQI_END_OF_FETCH ) 
    		{ 
				return(err);
     		}
	   		if (err != SQI_ERR_OUTPUT_WARNING) 
   	  		{ 
				return(err);
   	  		}
			fprintf(ArchivoLog, "SQI_fetch: error <%d,%d,%d>\n",
            		err_category(err), err_function(err), err_number(err));
			return ( err );
   		}
	
		/*//printf ( "\t output row[%d]: %s(%s, type(%d), bytes(%u) )\n", 
		//	 ++count, var1->varname, var1->data_p, var1->type, 
		//	 var1->size );*/
		sprintf( varpaso, var1->data_p);
		fprintf(ArchivoLog, "\t ---> 2. Valor encontrado : %s \n", varpaso);

	} /* while */
	return (0);
}

/*			-------------------
 *			  create
 *			--------------------
 *
 *     Create a cursor on the current or a specific session with a SQL
 *     statement.
 */
error_typ
create_cursor ( sqlstmt, ch_p, sqlca_p )
char			*sqlstmt;
SQI_cursor_handle_typ	*ch_p;
SQI_sqlca_typ		*sqlca_p;
{
	error_typ     err;
	int           len;
	short         no_col = 0;

	/* drop trailing semi-colon */
	len = strlen ( sqlstmt );
	if (sqlstmt[len-1] == ';')
		sqlstmt[--len] = '\0';      
 
	err = SQI_create_cursor ( sqih, sqlstmt, ch_p, &no_col, sqlca_p );
	if ( err )
	{ 
		printf( "SQI_CreateCursor: error <%d,%d,%d>\n",
           		err_category(err), err_function(err), err_number(err));
		return ( err );
	}
	printf( "\n\tSQI_CreateCursor:\n\t\t[%s] \n\t\t Successful, cursor handle = [%u]\n",
                sqlstmt, *ch_p );
	return (0);
}
 
/*			-------------------
 *			  exec
 *			-------------------
 *
 *     Executes a statement with cursor.
 *
 */
error_typ
exec ( ch, commit, max_rows, drop_cursor, sqlca_p )
SQI_cursor_handle_typ	*ch;
unsigned short		max_rows;
bool			commit;
bool			drop_cursor;
SQI_sqlca_typ		*sqlca_p;
{
	error_typ         err;

	if ( err = SQI_exec_cursor ( sqih, ch, commit, drop_cursor,
				     max_rows, sqlca_p ) )
	{
		printf( "SQI_exec_cursor: error <%d,%d,%d>\n",
           		err_category(err), err_function(err), err_number(err));
    /*fprintf(ArchivoLog,"SQI_exec_cursor: error <%d,%d,%d>\n",
           		err_category(err), err_function(err), err_number(err));*/     		
		return ( err );
	}
	else
	{
        printf ( "\tSQI_exec_cursor: executed with %ld rows affected\n",
                  sqlca_p->sqlerrd [3] );
       /*fprintf(ArchivoLog,"\tSQI_exec_cursor: executed with %ld rows affected\n",
                  sqlca_p->sqlerrd [3]);*/
	}
	return (0);
}

/*			--------------------
 *			  execsql
 *			--------------------
 *
 *     Execute a sql statement that does not have input or output binds.
 */
error_typ
execsql ( sqlstmt, commit )
char	*sqlstmt;
bool	commit;
{
	error_typ	err;
	SQI_sqlca_typ	sqlca;
 
	printf( "\tSQI_exec_sql: <%s>\n", sqlstmt );

	if ( err = SQI_exec_sql( sqih, sqlstmt, commit, &sqlca) )
	{ 
		printf( "SQI_exec_sql: error <%d,%d,%d>\n",
           		err_category(err), err_function(err), err_number(err));
    /*fprintf(ArchivoLog, "SQI_exec_sql: error <%d,%d,%d>\n",
           		err_category(err), err_function(err), err_number(err));*/      		
		return ( err );
	}
	else
	{ 
		printf ( "\t\tstatement executed with %ld rows affected\n",
        		 sqlca.sqlerrd [3] );
    /*fprintf(ArchivoLog, "\t\tstatement executed with %ld rows affected\n",
        		 sqlca.sqlerrd [3] );*/
	}
	return (0);
}

void ProcesaIndices( int opc )
{
	int				i;
	char			arg1[1920];
	SQI_cursor_handle_typ	ch;
	SQI_sqlca_typ		sqlca;
	var_typ				var1;
	var_typ				var2;


	char	cadena[MAXLINE] ;
	char	respaldo[MAXLINE] ;
	char    temp[50];
	long    LongArchImag;
	static char     fsep      = ':';
	i=0;
	while( fgets( cadena, 128, ArchivoIndTemp ) ){
		strcpy( respaldo, cadena ) ;
			
		LongArchImag=MueveHasta( temp, fsep, cadena, MAXLINE, 0) ;
		temp[LongArchImag] = '\0' ;

		if (ExtraeImagenes (temp) ==1) {
		
			fprintf(ArchivoIndices,respaldo);

			if ( opc == 1) {

				if ( BASEDATOS==1 ) {
					/* Query SQL */
					sprintf( arg1, "Update f_sw.doctaba Set %s='3', %s=%d", Status,FechaOperacion,FechaFileNET); 
					sprintf( VarPaso, " Where f_docnumber='%s'", temp);
					strcat(arg1, VarPaso);
				} else {			
					/* Query Oracle */
					sprintf( arg1, "UPDATE F_SW.DOCTABA A SET A.%s=3, A.%s=%d", Status,FechaOperacion,FechaFileNET); 	
					/*//sprintf( arg1, "UPDATE F_SW.DOCTABA A SET A.%s='3' ", Status); 	
					//sprintf( arg1, "UPDATE F_SW.DOCTABA A SET A.%s=%d", FechaOperacion,FechaFileNET);*/
					sprintf( VarPaso, " WHERE A.F_DOCNUMBER=%s", temp);
					strcat(arg1, VarPaso);
					strcat(arg1, ";");
				}

				sprintf(VarPaso,"%d",FechaFileNET);

				create_cursor( arg1, &ch, &sqlca );

				memset( &var1, 0, sizeof(var_typ) );
				memset( &var2, 0, sizeof(var_typ) );
				
				/*var1.data_p	= (char *)malloc(30 * sizeof(char));
				sprintf( var1.data_p, "3");
				var1.type	= SQI_ASCII;
				var1.size	= strlen(var1.data_p);
				var1.pos = 1;*/

				strcpy( var1.varname, Status );
				var1.data_p	= (char *)malloc(30 * sizeof(char));  /*(int *)malloc(4 * sizeof(int));*/
				/*var1.data_p = 3;*/
				sprintf( var1.data_p, "3");			
				var1.type	= SQI_long;
				var1.size	= 10;
				var1.pos = 1;
				
				strcpy( var2.varname, FechaOperacion);
				var2.data_p	=  (char *)malloc(30 * sizeof(char));  /*(int *)malloc(4 * sizeof(int));			
				var2.data_p = FechaFileNET;*/
				sprintf( var2.data_p, VarPaso);
				var2.type	= SQI_long;
				var2.size	= 10;
				var2.pos = 2;

				printf( "\n\t ---> Actualizando registro %d  ... <---\n",i );
				printf( "\n\t ---> Query %s  ... <---\n",arg1 );
				/*bindvalue( &ch, "3", &var1 );
				bindvalue( &ch, VarPaso, &var2 );*/

				exec( &ch, TRUE, 1, TRUE, &sqlca );
			}
			i++;
		} /* end if */

	} /* end While */
	fprintf(ArchivoLog, "\t ---> Imagenes procesadas : %d \n ", i);
}

void ProcesaBIndices( )
{
	int				i;
	char			arg1[1920];
	char	cadena[MAXLINE] ;
	char	respaldo[MAXLINE] ;
	char    temp[50];
	long    LongArchImag;
	SQI_cursor_handle_typ	ch;
	SQI_sqlca_typ		sqlca;

	static char     fsep      = ':';
	i=0;
	while( fgets( cadena, 128, ArchivoIndTemp ) ){
		strcpy( respaldo, cadena ) ;
			
		LongArchImag=MueveHasta( temp, fsep, cadena, MAXLINE, 0) ;
		temp[LongArchImag] = '\0' ;

		if (BorraImagenes (temp) ==1) {
		
			if ( BASEDATOS==1 ) {
				/* Query SQL */
				sprintf( arg1, "DELETE f_sw.DOCTABA A "); 
				sprintf( VarPaso, " Where A.F_DOCNUMBER='%s'", temp);
				strcat(arg1, VarPaso);
			} else {			
				/* Query Oracle */
				sprintf( arg1, "DELETE f_sw.DOCTABA A"); 	
				sprintf( VarPaso, " WHERE A.F_DOCNUMBER=%s", temp);
				strcat(arg1, VarPaso);
				strcat(arg1, ";");
			}

			sprintf(VarPaso,"%d",FechaFileNET);

			create_cursor( arg1, &ch, &sqlca );

			printf( "\n\t ---> Borrando Registro registro %d Id: %s ... <---\n", i, temp );
			printf( "\n\t ---> Query %s  ... <---\n",arg1 );

			exec( &ch, TRUE, 1, TRUE, &sqlca );

			i++;
		} /* end if */

	} /* end While */
	fprintf(ArchivoLog, "\t ---> Imagenes procesadas : %d \n ", i);
}


int Particiona(char *ArcImg)
{   
	error_typ                   err;
    char                        fname[80];
	char                        fname1[80];
    bool                        checksum_exists;
    unsigned                    offset;
    unsigned                    len;
    long                        fid;
	long                        fid1;
    long                        accumulator;
	unsigned                    accumulatorpage;
    long						in_len;
	struct stat					stat;
	FFI_buf_typ					ffi_buf;
    FFI_page_type_typ			page_type;
	char*                       buf_p;
	unsigned short              Bande;	
	int							i;

    buf_p = (char*) malloc (BUFSIZE);

	Bande = 0;
	i=1;
	sprintf (fname, "%s\\%s_%d.tif", DirCompletas, ArcImg, i);

#ifdef _WIN32
	if ((fid = open (fname, (_O_BINARY), 0666)) < 0) {
		fprintf(ArchivoLog,"Can't open output file %s\n", fname);
		return (0);
	}
#else
	if ((fid = open (fname, 0)) < 0) {
		fprintf(ArchivoLog,"Can't Open output file %s\n", fname);
		return (0);
	}
#endif

	if (fstat (fid, &stat))
	{
		fprintf(ArchivoLog,"fstat on file failed file %s\n", fname);
		return (0);
	}
    offset = 0;
    accumulator = 0;
	accumulatorpage=0;
	offset = lseek(fid, 0, SEEK_SET);  /* Move to the beginning. */

	if (stat.st_size <= 950000) {
		Bande = 0;
		sprintf (fname1, "%s\\%s_%d.tif", DirPartidas, ArcImg, i);
	} else {
		Bande = 1;
		/*//sprintf (fname1, "%s\\%s_%d.tif", DirPartidas, ArcImg, i);*/
		sprintf (fname1, "%s.tif", ArcImg);
	} /* End if */

#ifdef _WIN32
	if ((fid1 = open (fname1, (_O_BINARY | _O_CREAT | _O_WRONLY | _O_TRUNC), 0666)) < 0) {
		fprintf(ArchivoLog,"Can't create output file %s\n", fname1);		
	}
#else
    if ((fid1 = creat (fname1, 0666)) < 0) {
		fprintf(ArchivoLog,"Can't create output file %s\n", fname1);
		return (0);
	}
#endif
	while (offset < stat.st_size) { 
		len = MIN (stat.st_size - offset, BUFSIZE);
		if ((in_len = read (fid, buf_p, len)) != len) {
				fprintf(ArchivoLog,"Read from file failed %s\n", fname);
				return (0);
		}
		if (offset == 0) {
            ffi_buf.buf_p = buf_p;
            ffi_buf.buf_size = BUFSIZE;
            ffi_buf.data_size = len; 
            if (err = FFI_get_page_type (&ffi_buf, &page_type)) {
				fprintf(ArchivoLog,"Not a valid page format %s\n", fname1);
				return (0);
			}
                
		}
		if (write (fid1, buf_p, len) < (int) len) {					
			fprintf(ArchivoLog,"Can't write output file %s\n", fname);
			return (0);
		}

	    /* Compute the checksum if one is saved with the document.
		   Note that this step is optional (and time consuming) */
		if (checksum_exists) {
			accumulator = CKS_compute_csum ((long*) buf_p, len, accumulator);		
		}
		accumulatorpage += len;
		offset += len;

	}  /* End while */
	close (fid);
	close (fid1);

    if (err) {
		fprintf(ArchivoLog,"Can't open object file '%s'\n", fname);
		return (0);
	}
    else {
		fprintf(ArchivoLog,"\tPage(s) retrieved and put into file '%s'\n", fname);		
		if (Bande ==1) {
			/*if	(ParticionaImg(fname1) != 1) {
				fprintf(ArchivoLog,"No se pudo particionar Imagen '%s'\n", fname);
				return (0);						
			}*/
			/*//ParticionaImg(fname1);*/
		} /* Endif Bande==1 */			
		return (0);
	} /* Endif err */
	return (0);
}

static char *GetFecha()
{
	struct tm *tms ;
	time_t rest_t , *arg_t = ( time_t *) malloc( sizeof( time_t ) ) ;
	/*char fecha[9] ;*/
	
	rest_t = time( arg_t ) ;
	tms = localtime( arg_t ) ;
	/*//sprintf( fecha , "%02d/%02d/%02d" , tms->tm_mon + 1 , tms->tm_mday, tms->tm_year ) ;*/
	/*sprintf( fecha , "%02d%02d%02d" , tms->tm_mon + 1 , tms->tm_mday, tms->tm_year ) ;*/
	sprintf( fecha , "%02d%02d" , tms->tm_mon + 1 , tms->tm_mday) ;
	fecha[5] = '\0' ;
	return( fecha ) ;
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
