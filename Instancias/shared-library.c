/*
 * shared-library.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "shared-library.h"

int levantar_servidor(char* puerto) {
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

	return listening_socket;
}

void manejar_cliente(int listening_socket, int socketCliente, char* mensaje) {

	loggear("Esperando cliente...");

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	loggear("Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	char package[PACKAGE_SIZE];

	int res = recv(socketCliente, (void*) package, PACKAGE_SIZE, 0);

	if (res <= 0) {
		loggear("Fallo la conexion con el cliente.");
	}

	loggear("Mensaje recibido exitosamente:");
	loggear(package);
	send(socketCliente, mensaje, strlen(mensaje) + 1, 0);

	loggear("Terminando conexion con el cliente actual.");
}

int conectar_a(char *ip, char *puerto, char* mensaje) {
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(ip, puerto, &hints, &serverInfo);
	int server_socket;
	server_socket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	int conexion = connect(server_socket, serverInfo->ai_addr,
			serverInfo->ai_addrlen);

	if (conexion < 0) {
		salir_con_error("Fallo la conexion con el servidor.", server_socket);
	}

	loggear("Conectó sin problemas");

	freeaddrinfo(serverInfo);

	char package[PACKAGE_SIZE];

	send(server_socket, mensaje, strlen(mensaje) + 1, 0);

	loggear("Mensaje enviado.");
	int res = recv(server_socket, (void*) package, PACKAGE_SIZE, 0);

	if (res != 0) {
		loggear("Mensaje recibido desde el servidor.");
		loggear(package);

	} else {
		salir_con_error("Fallo el envio de mensaje de parte del servidor.",
				server_socket);
	}

	loggear("Cerrando conexion con servidor y terminando.");

	return server_socket;
}

void identificar_cliente(int* id, int socket_cliente) {
	if (*id == 0) {
		loggear(
				"My name is Planificador.c, and I'm the fastest planifier alive...");
	} else if (*id == 1) {
		loggear("A wild ESI has appeared!");
	} else if (*id == 2) {
		loggear("It's ya boi, Instancia!");
	} else {
		free(id);
		salir_con_error("Cliente desconocido, cerrando conexion.",
				socket_cliente);
	}

	free(id);

	return;
}

void chequear_servidor(int* id, int server_socket) {
	if (*id == 0) {
		loggear("Gracias por conectarse al planificador!");
	}
	if (*id == 3) {
		loggear("Gracias por conectarse al coordinador!");
	} else {
		free(id);
		salir_con_error("Servidor desconocido, cerrando conexion",
				server_socket);
	}

	free(id);
}

void iniciar_log(char* nombre, char* mensajeInicial) {
	logger = log_create("ReDisTinto.log", nombre, true, LOG_LEVEL_TRACE);
	loggear(mensajeInicial);
}

void loggear(char* mensaje){
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
