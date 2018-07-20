/*
 * shared-library.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef SHARED_LIBRARY_H_
#define SHARED_LIBRARY_H_

#include <openssl/md5.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <readline/readline.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <commons/string.h>
#include <signal.h>
#include <math.h>
#include "structs/estructuras.h"
#include "lists/lists.h"
#include "strings/strings.h"

//Variables super globales

#define PACKAGE_SIZE 1024
#define BACKLOG 20
#define MAX_LEN 40
#define WEED "\x1b[32m"

t_log * logger;
char* mensajePlanificador =
		"My name is Planificador.c and I'm the fastest planifier alive...";
char* mensajeESI = "A wild ESI has appeared!";
char* mensajeESILista = "Gotcha! Wild ESI was added to the list!";
char* mensajeInstancia = "It's ya boi, instancia!";
char* mensajeCoordinador = "Coordinador: taringuero profesional.";

package_int id_coordinador = { .packed = 0 };

package_int id_planificador = { .packed = 1 };

package_int id_ESI = { .packed = 2 };

package_int id_instancia = { .packed = 3 };

package_int packed_recv_error = { .packed = -20 };

aviso_con_ID aviso_recv_error = { .aviso = -20 };



char* string_recv_error = "RECV ERROR";

//Funciones servidor

int levantar_servidor(char* puerto, int tries);
/*
 * Descripción: crea un socket escuchante y llama a bind() para atarlo al puerto y crear
 * 		un servidor en dicho puerto. En caso que falle el bindeo, se intenta de nuevo. Si falla 5
 * 		veces, se cierra el proceso.
 * Argumentos:
 * 		char* puerto: puerto en el que se encuentra el servidor.
 * 		int tries: número de intentos realizados.
 */

//Funciones cliente
int conectar_a(char* ip, char* puerto, package_int id, int tries);
/*
 * Descripción: establece una conexión con un servidor y le envía un mensaje, y devuelve
 * 		el socket servidor. En caso que falle la conexión, se intenta de nuevo. Si falla 5 veces,
 * 		se cierra el proceso.
 * Argumentos:
 * 		char* ip: ip del servidor.
 * 		char* puerto: puerto por el que escucha el servidor.
 * 		package_int id: mensaje a enviar al servidor.
 * 		int tries: número de intentos realizados.
 */

void chequear_servidor(package_int id, int server_socket);
/*
 * Descripción: revisa el mensaje devuelto por el servidor para saber de qué servidor se trata.
 * 		En caso de que sea un servidor desconocido, llama a salir_con_error().
 * Argumentos:
 * 		package_int id: identificación enviada por el servidor.
 * 		int server_socket: socket del servidor.
 */

void serializar_packed(package_int pedido, char** message);
/*
 * Descripción: serializa un mensaje del tipo package_int.
 * Argumentos:
 * 		package_int pedido: mensaje a serializar.
 * 		char** message: el recipiente del mensaje serializado.
 */

void deserializar_packed(package_int *pedido, char **package);
/*
 * Descripción: deserializa un mensaje del tipo package_int.
 * Argumentos:
 * 		package_int *pedido: el recipiente del mensaje a deserializar.
 * 		char** message: buffer de memoria con el mensaje a deserializar.
 */

void serializar_aviso(aviso_con_ID aviso, char** message);
/*
 * Descripción: serializa un mensaje del tipo aviso_con_ID.
 * Argumentos:
 * 		aviso_con_ID aviso: mensaje a serializar.
 * 		char** message: el recipiente del mensaje serializado.
 */

void deserializar_aviso(aviso_con_ID *aviso, char** package);
/*
 * Descripción: deserializa un mensaje del tipo aviso_con_ID.
 * Argumentos:
 * 		aviso_con_ID aviso: el recipiente del mensaje a deserializar.
 * 		char** message: buffer de memoria con el mensaje a deserializar.
 */

void avisar_cierre(int sockfd);
/*
 * Descripción: informa a un proceso que éste va a cerrar.
 * Argumentos:
 * 		int sockfd: socket al cual se ennviará el mensaje de cierre.
 */

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

void terminar_conexion(int sockfd, bool retry);
/*
 * Descripción: envía un mensaje de terminación a un proceso a través de un socket. El flag
 * 		retry indica si, en caso que falle el envìo, se intenta de nuevo o no.
 * Argumentos:
 * 		int sockfd: el socket por el cual se envía el mensaje.
 * 		bool retry: flag de reintento de cierre de conexión.
 */

void enviar_aviso(int sockfd, aviso_con_ID aviso);
/*
 * Descripción: envía un mensaje a un servidor del tipo aviso_con_ID. En caso que falle, se
 * 		llama a salir_con_error().
 * Argumentos:
 * 		int sockfd: socket del servidor.
 * 		aviso_con_ID aviso: aviso a enviar.
 */

aviso_con_ID recibir_aviso(int sockfd);
/*
 * Descripción: recibe un aviso del tipo aviso_con_ID de otro proceso. En caso que falle, se
 * 		llama a salir_con_error().
 * Argumentos:
 * 		int sockfd: socket del proceso.
 */

void enviar_packed(package_int packed, int sockfd);
/*
 * Descripción: envia un paquete package_int a otro proceso. En caso que falle, se
 * 		llama a salir_con_error().
 * Argumentos:
 * 		package_int packed: el paquete a enviar.
 * 		int sockfd: socket del proceso al cual enviar el paquete.
 */

package_int recibir_packed(int sockfd);
/*
 * Descripción: recibe un paquete package_int de otro proceso. En caso que falle, se
 * 		llama a salir_con_error().
 * Argumentos:
 * 		int sockfd: socket del proceso del cual recibir el paquete.
 */

void enviar_cadena(char* cadena, int sockfd);
/*
 * Descripción: envía una cadena a otro proceso. En caso que falle, se
 * 		llama a salir_con_error().
 * Argumentos.
 * 		char* cadena: la cadena a enviar.
 * 		int sockfd: el socket del proceso al cual enviar la cadena.
 */

char* recibir_cadena(int sockfd, uint32_t size);
/*
 * Descripción: recibe una cadena de otro proceso. En caso que falle, se
 * 		llama a salir_con_error().
 * Argumentos:
 * 		int sockfd: socket del proceso del cual recibir la cadena.
 * 		int size: tamaño de la cadena a recibir.
 */

char* serializar_valores_set(int tamanio_a_enviar, parametros_set * valor_set);
/*
 * Descripción: serializa un mensaje del tipo parametros_set.
 * Argumentos:
 * 		tamanio_a_enviar: para hacer el malloc.
 * 		valor_set: el recipiente del mensaje serializado.
 */

int send_package_int(package_int package, int sockfd);
/*
 * Descripción: envía un package_int a través de un socket y devuelve el resultado del send().
 * Argumentos:
 * 		package_int package: el paquete a enviar.
 * 		int sockfd: file descriptor del socket por el cual comunicar.
 */

int send_aviso_con_ID(aviso_con_ID aviso, int sockfd);
/*
 * Descripción: envía un aviso_con_ID a través de un socket y devuelve el resultado del send().
 * Argumentos:
 * 		aviso_con_ID aviso: el aviso a enviar.
 * 		int sockfd: file descriptor del socket por el cual comunicar.
 */

int send_string(char* cadena, int sockfd);
/*
 * Descripción: envía una cadena a través de un socket y devuelve el resultado del send().
 * Argumentos:
 * 		char* cadena: la cadena a enviar.
 * 		int sockfd: file descriptor del socket por el cual comunicar.
 */

int recv_package_int(package_int* package, int sockfd);
/*
 * Descripción: recibe un package_int a través del sockfd y lo escribe en package. El retorno
 * 		es el resultado del recv().
 * Argumentos:
 * 		package_int* package: puntero al package sobre el cual escribir.
 * 		int sockfd: file descriptor del socket por el cual comunicar.
 */

int recv_aviso_con_ID(aviso_con_ID* aviso, int sockfd);
/*
 * Descripción: recibe un aviso_con_ID a través del sockfd y lo escribe en aviso. El retorno
 * 		es el resultado del recv().
 * Argumentos:
 * 		aviso_con_ID* aviso: puntero al aviso sobre el cual escribir.
 * 		int sockfd: file descriptor del socket por el cual comunicar.
 */

int recv_string(char* cadena, uint32_t size, int sockfd);
/*
 * Descripción: recibe un char* a través del sockfd y lo escribe en la cadena. El retorno
 * 		es el resultado del recv().
 * Argumentos:
 * 		char* cadena: la cadena sobre la cual escribir el recv().
 * 		int sockfd: file descriptor del socket por el cual comunicar.
 */

void send_packed_no_exit(package_int package, int sockfd);
/*
 * Descripción: envía un package_int. En caso que falle, loguea a nivel warning, pero no
 * 		corta la ejecución del proceso.
 * Argumentos:
 * 		package_int package: el package_int a enviar.
 * 		int sockfd: file descriptor del socket por el cual comunicar.
 */

void send_aviso_no_exit(aviso_con_ID aviso, int sockfd);
/*
 * Descripción: envía un aviso_con_ID. En caso que falle, loguea a nivel warning, pero no
 * 		corta la ejecución del proceso.
 * Argumentos:
 * 		aviso_con_ID aviso: el aviso_con_ID a enviar.
 * 		int sockfd: file descriptor del socket por el cual comunicar.
 */

void send_string_no_exit(char* string, int sockfd);
/*
 * Descripción: envía un string. En caso que falle, loguea a nivel warning, pero no
 * 		corta la ejecución del proceso
 * Argumentos:
 * 		char* string: la cadena a enviar.
 * 		int sockfd: file descriptor del socket por el cual comunicar.
 */

package_int recv_packed_no_exit(int sockfd);
/*
 * Descripción: recibe un package_int. En caso que falle, loguea a nivel warning y retorna
 * 		packed_recv_error.
 * Argumentos:
 * 		int sockfd: file descriptor por el cual recibir el package_int.
 */

aviso_con_ID recv_aviso_no_exit(int sockfd);
/*
 * Descripción: recibe un aviso_con_ID. En caso que falle, loguea a nivel warning y retorna
 * 		aviso_recv_error.
 * Argumentos:
 * 		int sockfd: file descriptor por el cual recibir el aviso_con_ID.
 */

char* recv_string_no_exit(int sockfd, uint32_t size);
/*
 * Descripción: recibe un char*. En caso que falle, loguea a nivel warning y retorna
 * 		string_recv_error.
 * Argumentos:
 * 		int sockfd: file descriptor por el cual recibir el char*.
 * 		uint32_t size: tamaño de la cadena a recibir.
 */

#endif /* SHARED_LIBRARY_H_ */
