/*
 * coordinador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <shared-library.h>

//Variables globales

pthread_t hilo_ESI;
pthread_t hilo_instancia;
pthread_t hilo_planificador;

//Funciones

void responder(void);
void *atender_ESI(void);
void *atender_Instaica(void);
void *atender_Planificador(void);
void iniciar_log(void);
void loggear(char* mensaje);

#endif /* COORDINADOR_H_ */
