/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

#define IP "127.0.0.1"
#define PUERTOALCOORDINADOR "8000"
#define PUERTOALPLANIFICADOR "8001"
#define PACKAGE_SIZE 1024
//Estos tres define van a cambiar, para poder cambiar ip y puerto en runtime (en caso de que esten ocupados) y para poder mandar datos de tamaño no fijo

int main() {
	iniciar_log();

	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(IP, PUERTOALCOORDINADOR, &hints, &serverInfo);
	int serverSocketCoordinador;
	serverSocketCoordinador = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	int conexionCoordinador = connect(serverSocketCoordinador, serverInfo->ai_addr,
			serverInfo->ai_addrlen);

	if (conexionCoordinador < 0) {
		salir_con_error("Fallo la conexion con el coordinador.", serverSocketCoordinador);
	}

	loggear("Conectó sin problemas al coordinador");

	getaddrinfo(IP, PUERTOALPLANIFICADOR, &hints, &serverInfo);
		int serverSocketPlanificador;
		serverSocketPlanificador = socket(serverInfo->ai_family, serverInfo->ai_socktype,
				serverInfo->ai_protocol);

		int conexionPlanificador = connect(serverSocketPlanificador, serverInfo->ai_addr,
				serverInfo->ai_addrlen);

		if (conexionPlanificador < 0) {
			salir_con_error("Fallo la conexion con el planificador.", serverSocketPlanificador);
		}

	loggear("Conectó sin problemas al planificador");

	freeaddrinfo(serverInfo);

	char message[] = "A wild ESI has appeared!";
	char package[PACKAGE_SIZE];

	send(serverSocketCoordinador, message, strlen(message) + 1, 0);
	send(serverSocketPlanificador, message, strlen(message) + 1, 0);

	loggear("Mensaje enviado.");
	int res = recv(serverSocketCoordinador, (void*) package, PACKAGE_SIZE, 0);

	if (res != 0) {
		loggear("Mensaje recibido desde el servidor.");
		loggear(package);

	} else {
		salir_con_error("Fallo el envio de mensaje de parte del servidor.",	serverSocketCoordinador);
	}

	send(serverSocketPlanificador, 0, sizeof(int), 0);
	loggear("Cerrando conexion con servidor y terminando.");

	close(serverSocketCoordinador);
	close(serverSocketPlanificador);
	return EXIT_SUCCESS;

}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "ESI", true, LOG_LEVEL_TRACE);
	loggear("ESI on duty.");
}

void loggear(char* mensaje) {
	log_trace(logger, mensaje);
}
