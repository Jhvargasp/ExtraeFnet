
/* Some of the defines in this file will need to be modified for
   the system which the sample programs are run on.  Comments
   below indicate the changes needed. */

/* If you install the system and leave the defaults in place, the items 
   below will work.  If you've changed the password for SysAdmin, however,
   then the password line below must be changed. */ 

#define     USER            "c406_090"  /* name of user logging on */
#define     PASSWORD        "c406_pwd"  /* password of user */
#define     USER2           "SysAdmin"  /* name of user logging on */
#define     PASSWORD2       "SysAdmin"  /* password of user */

/* #define     USER            "Operator"  / * name of user logging on */
/* #define     PASSWORD        "Operator"  / * password of user */

/* For the terminal, either change it to the name you use for the
   terminal, or leave it as listed below. */

#define     TERMINAL        "station1"  /* terminal name */

/* The domain name must be set to a double quoted character string
   which is the domain name of your system.  This name is also   
   referred to as the "system" name in some documentation. */ 

/* #define     DOMAINIO          "desc406"  / * domain name (system) Oracle */
#define     BASEDATOS          2				 /* oracle */

/* #define     DOMAINIO          "pruebas"  / * domain name (system) SQL */
/* #define     BASEDATOS          1	/ * SQL */


/* The following defines do not change. */ 

/*#define     ORGANIZATION    "Banamex"*/
#define     SECURITY        "SecurityService"
#define     WFLSERVER       "WflServer"


#define MAXLINE 300
#ifndef MAXPATH
#define	MAXPATH	250
#endif
#define  MAX_VAR_NAME_SIZE   30
#define  MAX_SUB_NAME_SIZE   30

char	DOMAINIO[MAX_VAR_NAME_SIZE];
char	ORGANIZATION[MAX_VAR_NAME_SIZE];
char	pSeccion[MAXLINE];
char	DirCompletas[MAXLINE];
char	DirPartidas[MAXLINE];
char	DirFormatoOnD[MAXLINE];
char	DirReportes[MAXLINE];
char	DirBitacoras[MAXLINE];
char	DirTemporal[MAXLINE];
char	DirProgramas[MAXLINE];
int		Vigencia, Vigencia2;
char	VarPaso[MAXLINE];
char	pathArchivoIndices[MAXPATH];
static	FILE *ArchivoIndices;
char	Signo[3];
int		SIRH,SIRH1,Seccion; 
char	FolioXFile[15];
char 	SIRH2[10];

/* stamp 0G^VXCR5SiZp@W:T4KwE\?V@P3OaD[>U;P2IaD`=[MN1HdBa<S6_9H^H^;R9Tc[]@WOf<KhZ\?jMP8JaD]AV8P9I`CZ=U7N1HaT]BW6M */
