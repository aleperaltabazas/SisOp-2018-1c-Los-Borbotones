/*
 * planificador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <shared-library.h>

//Variables locales
int ESI_id;
t_list * ESIs;
t_list * ESIs_bloqueados;
t_list * ESIs_en_ejecucion;
t_list * ESIs_listos;
t_list * ESIs_finalizados;

//Variables globales
FILE* archivo_de_parseo;

//Hilos
pthread_t hilo_ESI;
pthread_t hiloDeConsola;

//Funciones de consola
void * consolita(void *);
void listarOpciones(void);
void pausarOContinuar(void);
void bloquear(float codigo);
void desbloquear(float codigo);
void listar (void);
void kill (float codigo);
void status (float codigo);
void deadlock (void);
float recibirCodigo (void);
void interpretarYEjecutarCodigo (float comando);

//Funciones de servidor
int manjear_cliente(int listening_socket, int socket_cliente, char* mensaje);
void identificar_cliente(char* mensaje, int socket_cliente);

//Funciones de hilos
void* atender_ESI(void* a_parsear);

//Funciones
FILE* levantar_archivo(char* archivo);
void error_de_archivo(char* mensaje, int retorno);
void che_parsea(int socket_cliente);

#endif /* PLANIFICADOR_H_ */
