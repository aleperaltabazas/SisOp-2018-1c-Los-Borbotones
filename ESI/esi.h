/*
 * esi.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef ESI_H_
#define ESI_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>

int conectar_a(char* IP, char* puerto);
void salir_con_error(char* mensaje, int socket);
void exit_gracefully(int return_val);

t_log * logger;

#endif /* ESI_H_ */
