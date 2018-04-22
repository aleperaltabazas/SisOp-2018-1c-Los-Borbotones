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

t_log * logger;

//int server_socket = socket(AF_INET, SOCK_STREAM, 0);
//int client_socket = socket (AF_INET, SOCK_STREAM, 0);

int conectar_a(char* IP, char* puerto);
void salir_con_error(char* mensaje, int socket);
void exit_gracefully(int return_val);
void protocolo_conexion(char* IP, char* Puerto);
void inicializar_log(char* nombre_archivo, char* nombre_log);
void exit_gracefully(int return_val);

t_log * logger;

#endif /* SHARED_LIBRARY_H_ */
