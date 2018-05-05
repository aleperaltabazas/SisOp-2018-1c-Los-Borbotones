/*
 * instancias.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "instancias.h"

int main() {
	iniciar_log();

	char message_coordinador[PACKAGE_SIZE] = "It's ya boi, instancia!";

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

	return EXIT_SUCCESS;
}

void iniciar_log(void) {
	logger = log_create("ReDisTinto.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logger, "A new instance joins the brawl!");
}

