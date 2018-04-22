/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

int main(int argv, char** argc){
	return EXIT_SUCCESS;
}

int conectar_a(char *IP, char* puerto){

	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(IP);
	direccion_servidor.sin_port = htons(puerto);

	int socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
	int res = connect(socket_cliente, (void*) &direccion_servidor, sizeof(direccion_servidor));

	if(res != 0)
		salir_con_error("Error en conexion", socket);

		//Tira error logea el error y exitea

	return socket_cliente;

}

//Por alguna razon los logs no me estan funcionando
//Si alguno tiene mejor suerte avise

void salir_con_error(char* mensaje, int socket){
	//log_error(logger, mensaje);
	close(socket);

	exit_gracefully(1);
}

void exit_gracefully(int return_val){
	//log_destroy(logger);
	exit(return_val);
}
