/*
 * planificador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <shared-library.h>
//#include "shared-library.h"

//Estructuras
typedef struct parsed{
	int socket_cliente;
	FILE* archivo_a_parsear;
} parsed;

//Variables globales
FILE* archivo_de_parseo;

//Hilos
pthread_t hilo_ESI;

//Funciones de servidor
int manjear_cliente(int listening_socket, int socket_cliente, char* mensaje);
void identificar_cliente(char* mensaje, int socket_cliente);

//Funciones de hilos
void* atender_ESI(void* a_parsear);

//Funciones
FILE* levantar_archivo(char* archivo);
void error_de_archivo(char* mensaje, int retorno);
void che_parsea(int socket_cliente, char* line);

#endif /* PLANIFICADOR_H_ */
