/*
Archivo:             libsapuf.h
Aplicacion:          Credito Empresarial y Consumo
Sistema:             C406
Modulo:              TUXEDO
Version:             1.0
Fecha Creanción:  	 Enero 2006.
Fecha Modificacion:  Agosto 2007.
Funcion:             acceso al Sapuf.
Desarrollado por:    Andres Ventura Gonzalez.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include "SAPUF_Enlace.h"

#include "varsapuf.h"

static char *ConectaSapuf(char *);
long RutinaSapuf(void);
void ValoresInicio(void);

char xmensaje[500];

void ValoresInicio() {

	int 	i=0;
	char *vartemp;

	for (i = 0;i<=79; i++)  {
		sOperacion[i]='\0';
		sMaqDestino[i]='\0';
		sUsuarioDestino[i]='\0';
		sTipoDestino[i]='\0';
		sAplDestino[i]='\0';
		sAplOrigen[i]='\0';
		sAtributo[i]='\0';
		s[i]='\0';
	}
	for (i = 0;i<=29; i++) {
		spassword[i]='\0';		
	}
	for (i = 0;i<=19; i++) {
		sbasedato[i]='\0';		
	}

	vartemp=getenv("OPERACION");
	strcpy(sOperacion,vartemp);
	/*strcpy(sMaqDestino,"ucadmty2");*/
	vartemp=getenv( "MAQDEST" );
	strcpy(sMaqDestino,vartemp);
	/*strcpy(sUsuarioDestino,"c406_090");*/
	vartemp=getenv( "USUDEST" );
	strcpy(sUsuarioDestino,vartemp);
	/*strcpy(sTipoDestino,"AP");*/
	vartemp=getenv("TIPDEST");
	strcpy(sTipoDestino,vartemp);
	/*strcpy(sAplDestino,"Filenet");*/
	vartemp=getenv( "APLDEST" );
	strcpy(sAplDestino,vartemp);
	/*strcpy(sAplOrigen,"c406_090");*/
	vartemp=getenv( "APLORIG" );
	strcpy(sAplOrigen,vartemp);
	/*strcpy(Timer,"9999");*/
	vartemp=getenv( "TIMER" );
	strcpy(Timer,vartemp);
	/*strcpy(sAtributo,"0");*/ /* Solicitud del primer password */
	vartemp= getenv( "ATRIBUTO" );
	strcpy(sAtributo,vartemp);
	vartemp= getenv( "BASEDATO" );
	strcpy(sbasedato,vartemp);

}

long RutinaSapuf() {
	long  res;
	int Intentos=0, maxIntentos=3;
	/*
	** La instrucción do-while permite reintentar la conexión al SAPUF CENTRAL
	** si hubiese problemas de comunicación o puertos ocupados del lado de Tandem
	*/
	sleep(1); /* Pausa si no es la primera vez */
	do {
		if(Intentos)	{
			RecuperarMsg(s, sError, sSubError); /* Cierra el socket local */
			sleep(5); /* Pausa si no es la primera vez */
		}
		Intentos++;

		sprintf( xmensaje, "SolicitarPassword");

		res = SolicitarPassword( sOperacion, sMaqDestino, sUsuarioDestino,
		sTipoDestino, sAplDestino, sAplOrigen, sAtributo, Timer);
	} while(res!=1 && Intentos<=maxIntentos); /* Reintenta si el puerto esta ocupado */

	/* sprintf(xmensaje, "Saliendo del do - while valor ");*/

	return (res);

}

static char *ConectaSapuf(char *Atributo) {
	long  res=0;
	int   tam;

	if (atoi(Atributo)==7) 
	{
		strcpy(sAtributo,"0");
		bande=1;
	} 
	else 
	{
		strcpy(sAtributo,Atributo);
	}
	
	/*ValoresInicio();*/
	/* ------------------------------------------------------------------
			 preparando la conexion a SYBASE
	-------------------------------------------------------------------*/
	
	/*sprintf( xmensaje, "Valores de Recibidos por parametro (Atributo)  %s", sAtributo);*/
	/*sprintf( xmensaje, "Valor de ACCESOBD : %d ",sACCESOBD);
  sprintf( xmensaje, "Servidor BD : %s ",sMaqDestino);
	sprintf( xmensaje, "Engine : %s ",sAplDestino);
	sprintf( xmensaje, "Usuario : %s ",sUsuarioDestino);
	sprintf( xmensaje, "Apl Origen : %s ",sAplOrigen);
	sprintf( xmensaje, "Tipo Destino : %s ",sTipoDestino);*/

	res=RutinaSapuf();

	if( res != 1 ) {
	  tam = RecuperarMsg(s, sError, sSubError);
		sprintf(xmensaje, "E:%s SubC: %s DErr: %s", sError, sSubError, s);
	  strncpy(spassword, s,29);
	  bande=0;
	}
	else
	{
		tam = RecuperarMsg(s, sError, sSubError);
		strncpy(spassword, s,29);
		/*printf(xmensaje, "Atributo :%s ,Pw:%s",Atributo, spassword);*/
		tam =atoi(sAtributo);
		return (spassword);
  }	/* Endif ( res != 1 ) Atributo="0" */
} /* Fin ConectaSapuf   */
