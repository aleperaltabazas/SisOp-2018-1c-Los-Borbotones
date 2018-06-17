/*
 * coordinador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <shared-library.h>
//#include "shared-library.h"

//Variables globales

char* PUERTO_COORDINADOR;
algoritmo_distribucion ALGORITMO_DISTRIBUCION;
int CANTIDAD_ENTRADAS;
int TAMANIO_ENTRADAS;
float RETARDO;

int seguir_ejecucion = 1;

//Hilos

pthread_t hilo_ESI;
pthread_t hilo_instancia;
pthread_t hilo_planificador;

//Funciones de servidor

int manejar_cliente(int listening_socket, int socketCliente, char* mensaje);
	/*
	 * Descripción: determina qu� hacer cuando recibe una nueva conexi�n a trav�s del
	 * 		socket cliente.
	 * Argumentos:
	 * 		int listening_socket: socket del servidor local.
	 * 		int socket_cliente: socket del cliente.
	 * 		char* mensaje: mensaje para enviar como identificaci�na a los nuevos clientes.
	 */

void identificar_cliente(char* mensaje, int socket_cliente);
	/*
	 * Descripción: identifica a un cliente y crea un hilo para el mismo, o llama a salir_con_error().
	 * 		en caso de ser un cliente desconocido.
	 * Argumentos:
	 * 		char* mensaje: el mensaje enviado por el cliente.
	 * 		int socket_cliente: socket del cliente.
	 */

//Funciones de hilos

void *atender_ESI(void* un_socket);
	/*
	 * Descripción: atiende los mensajes enviados por un proceso ESI y le asigna los recursos
	 * 		o no si se encuentran disponibles.
	 * Argumentos:
	 * 		void* un_socket: descriptor del socket que luego se castea a int.
	 */

void *atender_Instancia(void* un_socket);
	/*
	 * Descripción: atiende los mensajes enviados por un proces de instancia y le indica
	 * 		los resultados de una ejecución de ESI.
	 * Argumentos:
	 * 		void* un_socket: descriptor del socket que luego se castea a int.
	 */

void *atender_Planificador(void* un_socket);
	/*
	 * Descripción: atiende los mensajes enviados por el planificador.
	 * Argumentos:
	 * 		void* un_socket: descriptor del socket que luego se castea a int.
	 */

//Funciones

int chequear_solicitud(int socket_cliente);
	/*
	 * Descripción: revisa una solicitud de ejecución de un proceso ESI y le indica si puede
	 * 		o no ejecutar.
	 * Argumentos:
	 * 		int socket_cliente: socket del proceso ESI.
	 */

void iniciar(void);
	/*
	 * Descripción: carga las configuraciones iniciales del proceso.
	 * Argumentos:
	 * 		void
	 */

void cargar_configuracion(void);
	/*
	 * Descripción: abre el archivo de configuración y carga los valores en variables globales.
	 * Argumentos:
	 * 		void
	 */

algoritmo_distribucion dame_algoritmo(char* algoritmo_src);
	/*
	 * Descripción: devuelve el algoritmo correspondiente a una cadena.
	 * Argumentos:
	 * 		char* algoritmo_src: cadena a tomar el algoritmo.
	 */

float dame_retardo(int retardo);
	/*
	 * Descripción: devuelve en segundos un retardo.
	 * Argumentos:
	 * 		int retardo: la forma entera (en microsegundos) del retardo.
	 */

#endif /* COORDINADOR_H_ */
