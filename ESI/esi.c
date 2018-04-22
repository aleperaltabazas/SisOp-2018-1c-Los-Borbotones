/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

int main(int argv, char** argc){
	inicializar_log("esi.log", "ESI");
	char* ip_planificador, puerto_planificador;
	char* ip_coordinador, puerto_coordinador;

	protocolo_conexion(ip_planificador, puerto_planificador);
	protocolo_conexion(ip_coordinador, puerto_coordinador);

	//log_info(logger, "ESI: Conexiones ok");

	return EXIT_SUCCESS;
}

int conectar_a(char *IP, char* puerto){
	/*struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(IP);
	direccion_servidor.sin_port = htons(puerto);

	int socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
	int res = connect(socket_cliente, (void*) &direccion_servidor, sizeof(direccion_servidor));

	if(res != 0)
		salir_con_error("Error en conexion", socket);

		//Tira error logea el error y exitea

	return socket_cliente;*/

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP, puerto, &hints, &server_info);

	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	int res = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	if (res < 0) {
	  salir_con_error("ESI: Fallo la conexion", server_socket);
	}
	//log_info(logger, "Conectado!");
	return server_socket;

}

void protocolo_conexion(char* IP, char* puerto){
	int socket = conectar_a(IP, puerto);
	wait_hello(socket);
	send_hello(socket);
	void * content = wait_content(socket);
}

//Por alguna razon los logs no me estan funcionando
//Si alguno tiene mejor suerte avise

void inicializar_log(char* nombre_archivo, char* nombre_log){
	//log_create(nombre_archivo, nombre_log, true, LOG_LEVEL_INFO);
	return;
}

void salir_con_error(char* mensaje, int socket){
	//log_error(logger, mensaje);
	close(socket);

	exit_gracefully(1);
}

void exit_gracefully(int return_val){
	//log_destroy(logger);
	exit(return_val);
}
