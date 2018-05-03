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

	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);
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

	char message[] = "A wild ESI has appeared!";
	char package[PACKAGE_SIZE];

	send(serverSocket, message, strlen(message) + 1, 0);

	loggear("Mensaje enviado.");
	int res = recv(serverSocket, (void*) package, PACKAGE_SIZE, 0);

	if (res != 0) {
		loggear("Mensaje recibido desde el servidor.");
		loggear(package);

	} else {
		salir_con_error("Fallo el envio de mensaje de parte del servidor.",	serverSocket);
	}

	loggear("Cerrando conexion con servidor y terminando.");

	close(serverSocket);
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
