/*
 * planificador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
//#include <shared-library.h>
#include "shared-library.h"
#include <commons/log.h>
#include <commons/collections/list.h>

void iniciar_log();

void listarOpciones() {
	printf ("1 : Pausar o reactivar la planificación /n");
	printf ("2,<ESI ID> : Bloquea al ESI elegido /n");
	printf ("3,<ESI ID> : Desbloquea al ESI elegido /n");
	printf ("4,<Recurso> : Lista procesos esperando dicho recurso /n");
	printf ("5,<ESI ID> : Mata al ESI elegido /n");
	printf ("6,<ESI ID> : Brinda el estado del ESI elegido");
	printf ("7 : Lista los ESI en deadlock");
	printf ("Introduzca la opcion deseada /n");
}
float recibirCodigo () {
	float code;
	scanf("%f", code);
	return code;
}

void interpretarYEjecutarCodigo (float comando) {
	int i;
	int * opcionElegida = malloc(sizeof(int));
	float * codigoSubsiguiente = malloc(sizeof(float));
	opcionElegida = div (comando,1);

	switch (opcionElegida) {
	case 1: pausarOContinuar();
	break;
	case 2: codigoSubsiguiente = comando - opcionElegida;
			bloquear (codigoSubsiguiente);
	break;
	case 3: codigoSubsiguiente = comando - opcionElegida;
			desbloquear (codigoSubsiguiente);
	break;
	case 4: codigoSubsiguiente = comando - opcionElegida;
			listar ();
	break;
	case 5: codigoSubsiguiente = comando - opcionElegida;
			kill (codigoSubsiguiente);
	break;
	case 6: codigoSubsiguiente = comando - opcionElegida;
			status (codigoSubsiguiente);
	break;
	case 7: deadlock();
	break;
	default: printf ("Codigo incorrecto, recuerde que se introduce un float");
	break;
	}

#endif /* PLANIFICADOR_H_ */
