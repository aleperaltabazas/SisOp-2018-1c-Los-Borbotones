/*
 * shared-library.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef SHARED_LIBRARY_H_
#define SHARED_LIBRARY_H_

#include <stdio.h>
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

#define PUERTO_COORDINADOR "8000"
#define PUERTO_PLANIFICADOR "8001"
#define IP_COORDINADOR "127.0.0.1"
#define IP_PLANIFICADOR "127.0.0.2"
#define BACKLOG 5//Definimos cuantas conexiones pendientes al mismo tiempo tendremos
#define PACKAGE_SIZE 1024

t_log * logger;

//Funciones servidor
int levantar_servidor(char* puerto);
void manjear_cliente(int listening_socket, int socketCliente, char* mensaje);
void identificar_cliente(char* mensaje, int socket_cliente);

//Funciones cliente
int conectar_a(char* ip, char* puerto, char* mensaje);
void chequear_servidor(char* id, int server_socket);

//Misc
void salir_con_error(char* mensaje, int socket);
void exit_gracefully(int return_val);
void iniciar_log(char* nombre, char* mensajeInicial);
void loggear(char* mensaje);

#endif /* SHARED_LIBRARY_H_ */
