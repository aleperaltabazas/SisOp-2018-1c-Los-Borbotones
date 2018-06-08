/*
 * shared-library.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef SHARED_LIBRARY_H_
#define SHARED_LIBRARY_H_

#include <stdio.h>
#include <stdint.h>
#include <openssl/md5.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <parsi/parser.h>

#define PUERTO_COORDINADOR "8000"
#define PUERTO_PLANIFICADOR "8001"
#define IP_COORDINADOR "127.0.0.1"
#define IP_PLANIFICADOR "127.0.0.2"
#define BACKLOG 5//Definimos cuantas conexiones pendientes al mismo tiempo tendremos
#define PACKAGE_SIZE 1024
#define LINE_MAX 40
#define ESTIMACION_INICIAL 3
#define MAX_MESSAGE_SIZE 300

//Variables super globales

t_log * logger;
char* mensajePlanificador =	"My name is Planificador.c and I'm the fastest planifier alive...";
char* mensajeESI = "A wild ESI has appeared!";
char* mensajeESILista = "Gotcha! Wild ESI was added to the list!";
char* mensajeInstancia = "It's ya boi, instancia!";
char* mensajeCoordinador = "Coordinador: taringuero profesional.";

//Estructuras

typedef struct aviso_ESI{
	int aviso;
	int id;
} __attribute__((packed)) aviso_ESI;

typedef struct package_permiso {
	uint32_t pedido;
} package_pedido;

typedef struct package_ESI{
	int aviso;
	char* clave;
	int long_clave;
	char* valor;
	int long_valor;
	int size;
} __attribute__((packed)) package_ESI;

//Funciones servidor

int levantar_servidor(char* puerto);
	/*
	 * Descripción: crea un socket escuchante y llama a bind() para atarlo al puerto y crear
	 * 		un servidor en dicho puerto.
	 * Argumentos:
	 * 		char* puerto: puerto en el que se encuentra el servidor.
	 */

//Funciones cliente

int conectar_a(char* ip, char* puerto, char* mensaje);
	/*
	 * Descripción: establece una conexión con un servidor y le envía un mensaje, y devuelve
	 * 		el socket servidor. En caso que falle la conexión con el servidor o el envío
	 * 		de mensajes, llama a salir_con_error().
	 * Argumentos:
	 * 		char* ip: ip del servidor.
	 * 		char* puerto: puerto por el que escucha el servidor.
	 * 		char* mensaje: mensaje a enviar al servidor.
	 */

void chequear_servidor(char* id, int server_socket);
	/*
	 * Descripción: revisa el mensaje devuelto por el servidor para saber de qué servidor se trata.
	 * 		En caso de que sea un servidor desconocido, llama a salir_con_error().
	 * Argumentos:
	 * 		char* id: identificación enviada por el servidor.
	 * 		int server_socket: socket del servidor.
	 */

void serializar_pedido(package_pedido pedido, char** message);
	/*
	 * Descripción: serializa un mensaje del tipo package_pedido.
	 * Argumentos:
	 * 		package_pedido pedido: mensaje a serializar.
	 * 		char** message: el recipiente del mensaje serializado.
	 */

void deserializar_pedido(package_pedido *pedido, char **package);
	/*
	 * Descripción: deserializa un mensaje del tipo package_pedido.
	 * Argumentos:
	 * 		package_pedido *pedido: el recipiente del mensaje a deserializar.
	 * 		char** message: buffer de memoria con el mensaje a deserializar.
	 */

void serializar_aviso(aviso_ESI aviso, char** message);
	/*
	 * Descripción: serializa un mensaje del tipo aviso_ESI.
	 * Argumentos:
	 * 		aviso_ESI aviso: mensaje a serializar.
	 * 		char** message: el recipiente del mensaje serializado.
	 */

void deserializar_aviso(aviso_ESI *aviso, char** package);
	/*
	 * Descripción: deserializa un mensaje del tipo aviso_ESI.
	 * Argumentos:
	 * 		aviso_ESI aviso: el recipiente del mensaje a deserializar.
	 * 		char** message: buffer de memoria con el mensaje a deserializar.
	 */

//Misc

void salir_con_error(char* mensaje, int socket);
	/*
	 * Descripción: llama a exit_gracefully() para terminar el proceso con error (1), loggea
	 * 		un mensaje de error y cierra el descriptor de archivo del socket.
	 * Argumentos:
	 * 		char* mensaje: el mensaje a loggear.
	 * 		socket: socket a cerrar.
	 */

void exit_gracefully(int return_val);
	/*
	 * Descripción: llama a exit() para terminar el proceso con un valor determinado.
	 * Argumentos:
	 * 		int return_val: el valor con el que se llama a exit().
	 */

void iniciar_log(char* nombre, char* mensajeInicial);
	/*
	 * Descripción: crea un log con un nombre pasado por parámetro y se loggea
	 * 		un mensaje inicial pasado por parámetro.
	 * Argumentos:
	 * 		char* nombre: nombre del logger.
	 * 		char* mensajeInicial: mensaje a loggear al principio.
	 */

void loggear(char* mensaje);
	/*
	 * Descripción: loggea un mensaje en la variable global logger.
	 * Argumentos:
	 * 		char* mensaje: el mensaje a loggear.
	 */

void terminar_conexion(int sockfd);
	/*
	 * Descripción: envía un mensaje de terminación a un proceso a través de un socket.
	 * Argumentos:
	 * 		int sockfd: el socket por el cual se envía el mensaje.
	 */

#endif /* SHARED_LIBRARY_H_ */
