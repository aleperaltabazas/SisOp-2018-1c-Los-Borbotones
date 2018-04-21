/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

int conectar_a(char *IP, char* puerto){

	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(IP);
	direccion_servidor.sin_port = htons(puerto);

	int socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
	int res = connect(socket_cliente, (void*) &direccion_servidor, sizeof(direccion_servidor));

	if(res != 0)
		tira_error("Error en conexion");

		//Tira error logea el error y exitea

	return socket_cliente;

}

void tira_error(char* mensaje){
	t_log * logger;
	log_error(logger, mensaje);

	exit(-1);
}
