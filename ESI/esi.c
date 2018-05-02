/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

int main() {
	//char mensaje[] = "A wild ESI has appeared!";
	iniciar_log();

	int id = 1;

	int socket_coordinador;
	int socket_planificador;

	socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR);
	enviar_identificacion(socket_coordinador, id);
	esperar_confirmacion(socket_coordinador);

	socket_planificador = conectar_a(IP_PLANIFICADOR, PUERTO_PLANIFICADOR);
	enviar_identificacion(socket_planificador, id);
	esperar_confirmacion(socket_planificador);

	loggear("Conexion exitosa.");

	close(socket_coordinador);
	close(socket_planificador);

	return EXIT_SUCCESS;

}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "ESI", true, LOG_LEVEL_TRACE);
	loggear("ESI on duty!");
}

void escucharRespuesta() {

	char package[PACKAGE_SIZE];
	struct addrinfo hints;
	struct addrinfo *server_info;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(IP_COORDINADOR, "6668", &hints, &server_info);
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
