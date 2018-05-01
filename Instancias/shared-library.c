/*
 * shared-library.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "shared-library.h"

int conectar_a(char *ip, char *puerto, char *mensaje) {
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

	char package[PACKAGE_SIZE];

	send(serverSocket, mensaje, strlen(mensaje) + 1, 0);

	loggear("Mensaje enviado.");
	int res = recv(serverSocket, (void*) package, PACKAGE_SIZE, 0);

	if (res != 0) {
		loggear("Mensaje recibido desde el servidor.");
		loggear(package);

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

	loggear("Esperando...");

	int listening_socket = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);

	bind(listening_socket, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

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

	char package[PACKAGE_SIZE];

	int res = recv(socket_aceptado, (void*) package, PACKAGE_SIZE, 0);

	if (res <= 0) {
		loggear("Fallo la conexion con el cliente.");
	}

	loggear("Mensaje recibido exitosamente:");
	loggear(package);

	return socket_aceptado;

}

int enviar_mensaje(int un_socket, char* message) {

	send(un_socket, message, strlen(message) + 1, 0);

	loggear("Terminando conexion con el cliente.");

	return un_socket;
}

void recibir_conexion(char* puerto) {
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

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	loggear("Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	char package[PACKAGE_SIZE];
	char message[] = "Gracias por conectarse al coordinador!";

	int res = recv(socketCliente, (void*) package, PACKAGE_SIZE, 0);

	if (res <= 0) {
		loggear("Fallo la conexion con el cliente.");
	}

	loggear("Mensaje recibido exitosamente:");
	loggear(package);
	send(socketCliente, message, strlen(message) + 1, 0);

	loggear("Terminando conexion con el cliente.");

	close(socketCliente);
	close(listening_socket);
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
