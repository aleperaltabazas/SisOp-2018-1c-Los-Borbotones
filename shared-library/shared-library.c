/*
 * shared-library.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "shared-library.h"

int conectar_a(char *ip, char *puerto, int* id) {
	loggear("Intentando conexion al servidor.");
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;		// Protocolo TCP

	getaddrinfo(ip, puerto, &hints, &serverInfo);
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	int conexion = connect(serverSocket, serverInfo->ai_addr,
			serverInfo->ai_addrlen);

	if (conexion < 0) {
		salir_con_error("Fallo la conexion con el servidor.", serverSocket);
	}

	loggear("Conectó sin problemas");

	freeaddrinfo(serverInfo);

	int* package;

	send(serverSocket, id, sizeof(int), 0);

	loggear("Mensaje enviado. Esperando mensaje del servidor.");
	int res = recv(serverSocket, package, sizeof(int), 0);

	if (res != 0) {
		loggear("Mensaje recibido.");
		chequear_servidor(package, serverSocket);

	} else {
		salir_con_error("Fallo el envio de mensaje de parte del servidor.",
				serverSocket);
	}

	loggear("Cerrando conexion con servidor y terminando.");

	return serverSocket;
}

int escuchar_socket(char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;		//Le indicamos localhost
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, puerto, &hints, &server_info);

	int listening_socket = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);

	bind(listening_socket, server_info->ai_addr, server_info->ai_addrlen);
	freeaddrinfo(server_info);

	loggear("Esperando cliente...");

	listen(listening_socket, BACKLOG);

	return listening_socket;
}

int aceptar_conexion(int listening_socket) {
	struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	loggear("Cliente conectado.");

	return (socketCliente);

}

int recibir_mensaje(int socket_aceptado) {
	loggear("Esperando mensaje del cliente.");

	int* package;

	int res = recv(socket_aceptado, package, sizeof(int) + 1, 0);

	if (res <= 0) {
		loggear("Fallo la conexion con el cliente.");
	}

	loggear("Mensaje recibido exitosamente. Identificando cliente...");

	identificar_cliente(package, socket_aceptado);

	return socket_aceptado;

}

int enviar_mensaje(int un_socket, int* id) {

	send(un_socket, id, sizeof(int), 0);

	loggear("Terminando conexion con el cliente.");

	return un_socket;
}

void identificar_cliente(int* id, int socket_cliente){
	if(id == 0){
		loggear("My name is Planificador.c, and I'm the fastest planifier alive...");
	}
	else if(*id == 1){
		loggear("A wild ESI has appeared!");
	}
	else if(*id == 2){
		loggear("It's ya boi, Instancia!");
	}
	else{
		salir_con_error("Cliente desconocido, cerrando conexion.", socket_cliente);
	}

	return;
}

void chequear_servidor(int* id, int server_socket){
	if(*id == 0){
		loggear("Gracias por conectarse al planificador!");
	}
	if(*id == 3){
		loggear("Gracias por conectarse al planificador!");
	}
	else{
		salir_con_error("Servidor desconocido, cerrando conexion", server_socket);
	}
}

void loggear(char* mensaje) {
	log_trace(logger, mensaje);
}

void salir_con_error(char* mensaje, int socket) {
	log_error(logger, mensaje);
	close(socket);

	exit_gracefully(1);
}

void exit_gracefully(int return_val) {
	//log_destroy(logger);
	exit(return_val);
}
