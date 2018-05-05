/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGE_SIZE 1024
//Estos tres define van a cambiar, para poder cambiar ip y puerto en runtime (en caso de que esten ocupados) y para poder mandar datos de tamaño no fijo

int main() {
	iniciar_log();

	struct addrinfo hints_coordinador;
	struct addrinfo *serverInfo_coordinador;
	memset(&hints_coordinador, 0, sizeof(hints_coordinador));
	hints_coordinador.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints_coordinador.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(IP_COORDINADOR, PUERTO_COORDINADOR, &hints_coordinador,
			&serverInfo_coordinador);
	int socket_coordinador;
	socket_coordinador = socket(serverInfo_coordinador->ai_family,
			serverInfo_coordinador->ai_socktype,
			serverInfo_coordinador->ai_protocol);

	int conexion = connect(socket_coordinador, serverInfo_coordinador->ai_addr,
			serverInfo_coordinador->ai_addrlen);

	if (conexion < 0) {
		salir_con_error("Fallo la conexion con el servidor.",
				socket_coordinador);
	}

	loggear("Conectó sin problemas");

	freeaddrinfo(serverInfo_coordinador);

	char message_coordinador[] = "A wild ESI has appeared!";
	char package_coordinador[PACKAGE_SIZE];

	send(socket_coordinador, message_coordinador,
			strlen(message_coordinador) + 1, 0);

	loggear("Mensaje enviado.");
	int res = recv(socket_coordinador, (void*) package_coordinador,
	PACKAGE_SIZE, 0);

	if (res != 0) {
		loggear("Mensaje recibido desde el servidor.");
		loggear(package_coordinador);

	} else {
		salir_con_error("Fallo el envio de mensaje de parte del servidor.",
				socket_coordinador);
	}

	loggear("Cerrando conexion con servidor y terminando.");

	struct addrinfo hints_planificador;
	struct addrinfo *serverInfo_planificador;
	memset(&hints_planificador, 0, sizeof(hints_planificador));
	hints_planificador.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints_planificador.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(IP_PLANIFICADOR, PUERTO_PLANIFICADOR, &hints_planificador,
			&serverInfo_planificador);
	int socket_planificador;
	socket_planificador = socket(serverInfo_planificador->ai_family,
			serverInfo_planificador->ai_socktype,
			serverInfo_planificador->ai_protocol);

	int conexion_planificador = connect(socket_planificador,
			serverInfo_planificador->ai_addr,
			serverInfo_planificador->ai_addrlen);

	if (conexion_planificador < 0) {
		salir_con_error("Fallo la conexion con el servidor.",
				socket_planificador);
	}

	loggear("Conectó sin problemas");

	freeaddrinfo(serverInfo_planificador);

	char message_planificador[] = "A wild ESI has appeared!";
	char package_planificador[PACKAGE_SIZE];

	send(socket_planificador, message_planificador,
			strlen(message_planificador) + 1, 0);

	loggear("Mensaje enviado.");
	int res_planificador = recv(socket_planificador,
			(void*) package_planificador,
			PACKAGE_SIZE, 0);

	if (res_planificador != 0) {
		loggear("Mensaje recibido desde el servidor.");
		loggear(package_planificador);

	} else {
		salir_con_error("Fallo el envio de mensaje de parte del servidor.",
				socket_planificador);
	}

	loggear("Cerrando conexion con servidor y terminando.");

	close(socket_planificador);

	close(socket_coordinador);
	return EXIT_SUCCESS;
}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "ESI", true, LOG_LEVEL_TRACE);
	loggear("ESI on duty.");
}

void loggear(char* mensaje) {
	log_trace(logger, mensaje);
}

void escucharRespuesta() {

	char package[PACKAGE_SIZE];
	struct addrinfo hints;
	struct addrinfo *server_info;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(IP, "6668", &hints, &server_info);
	// Tambien pruebo que se escuche el puerto 6668

	int listening_socket = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);
	//El socket que va a escuchar

	bind(listening_socket, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);
	listen(listening_socket, 1);
	if (recv(listening_socket, (void*) package, PACKAGE_SIZE, 0) < 0) {
		perror("No tuve respuesta");
	}
}
