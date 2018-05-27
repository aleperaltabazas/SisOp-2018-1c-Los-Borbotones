/*
 * planificador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <shared-library.h>

//Estructuras
typedef struct algoritmo {
	enum {
		FIFO, SJF, HRRN,
	} tipo;
	bool desalojo;
} algoritmo;

typedef struct ESI {
	int id;
	pthread_t hilo;
} ESI;

//Variables locales

int ESI_id;
t_list * ESIs;
t_list * ESIs_bloqueados;
t_list * ESIs_en_ejecucion;
t_list * ESIs_listos;
t_list * ESIs_finalizados;

ESI* executing_ESI;

algoritmo algoritmo_planificacion;

pthread_spinlock_t sem_ESIs;
pthread_spinlock_t sem_id;

//Hilos

pthread_t hiloDeConsola;

//Funciones de consola

void* consola(void);
void listarOpciones(void);
void pausarOContinuar(void);
void bloquear(float codigo);
void desbloquear(float codigo);
void listar(void);
void kill(float codigo);
void status(float codigo);
void deadlock(void);
float recibirCodigo(void);
void interpretarYEjecutarCodigo(float comando);

//Funciones de servidor

int manjear_cliente(int listening_socket, int socket_cliente, char* mensaje);
void identificar_cliente(char* mensaje, int socket_cliente);

//Funciones de hilos

void* atender_ESI(void* a_parsear);

//Funciones

void iniciar(void);
void planificar(int sockfd);
void desalojar(void);
void ejecutar(int sockfd);
void copiar_a(void* esi_copiado, ESI* esi_receptor);
void cerrar(void);
void cerrar_listas(void);

ESI* cabeza(t_list* lista);
ESI* shortest(t_list* lista);
ESI* highest_RR(t_list* lista);

#endif /* PLANIFICADOR_H_ */
