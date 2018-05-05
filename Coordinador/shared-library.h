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

//int server_socket = socket(AF_INET, SOCK_STREAM, 0);
//int client_socket = socket (AF_INET, SOCK_STREAM, 0);

int conectar_a(char* IP, char* puerto);
int escuchar_socket(char * puerto);
int aceptar_conexion(int listening_socket);
int recibir_mensaje(int socket_aceptado);
int enviar_mensaje(int un_socket, int id);
void esperar_confirmacion(int socket_servidor);
void enviar_identificacion(int server_socket, int id);
void recibir_conexion(char* puerto);
void salir_con_error(char* mensaje, int socket);
void exit_gracefully(int return_val);
void inicializar_log(char* nombre_archivo, char* nombre_log);
void loggear(char* mensaje);
void identificar_cliente(int* id, int socket_cliente);
void chequear_servidor(int* id, int server_socket);

t_log * logger;

#endif /* SHARED_LIBRARY_H_ */