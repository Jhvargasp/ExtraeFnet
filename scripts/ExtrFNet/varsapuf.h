#ifndef _VARSAPUF_H_
#define _VARSAPUF_H_

/*
Archivo:             varsapuf.h
Aplicacion:          Credito Empresarial y Consumo
Sistema:             C406
Modulo:              TUXEDO
Version:             1.0
Fecha Creanción:  	 Enero 2006.
Fecha Modificacion:  Agosto 2007.
Funcion:             variables.
Desarrollado por:    Andres Ventura Gonzalez.
*/

#include <stdio.h>
#include <unistd.h>

char  sOperacion[80], sMaqDestino[80];
char  sUsuarioDestino[80], sTipoDestino[80];
char  sAplDestino[80], sAplOrigen[80];
char  sAtributo[80],  Timer[16];
char  s[256], sError[4], sSubError[4];
char  spassword[30];
char  sbasedato[20];
int		sACCESOBD;
int 	bande=0;

#endif
