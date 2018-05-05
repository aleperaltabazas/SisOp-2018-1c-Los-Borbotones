/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "planificador.h"

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

	getaddrinfo(IP, PUERTO_COORDINADOR, &hints_coordinador,
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

	char message[] =
			"My name is Planificador.c and I'm the fastest planifier alive...!";
	char package[PACKAGE_SIZE];

	send(socket_coordinador, message, strlen(message) + 1, 0);

	loggear("Mensaje enviado.");
	int res = recv(socket_coordinador, (void*) package, PACKAGE_SIZE, 0);

	if (res != 0) {
		loggear("Mensaje recibido desde el servidor.");
		loggear(package);

	} else {
		salir_con_error("Fallo el envio de mensaje de parte del servidor.",
				socket_coordinador);
	}

	loggear("Cerrando conexion con servidor y terminando.");


	struct addrinfo hints_planificador;
	struct addrinfo *server_info_planificador;

	memset(&hints_planificador, 0, sizeof(hints_planificador));
	hints_planificador.ai_family = AF_UNSPEC;
	hints_planificador.ai_flags = AI_PASSIVE;		//Le indicamos localhost
	hints_planificador.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, PUERTO_PLANIFICADOR, &hints_planificador, &server_info_planificador);

	int listening_socket = socket(server_info_planificador->ai_family,
			server_info_planificador->ai_socktype, server_info_planificador->ai_protocol);

	bind(listening_socket, server_info_planificador->ai_addr, server_info_planificador->ai_addrlen);
	freeaddrinfo(server_info_planificador);

	int socketCliente;

	while (1) {

		listen(listening_socket, BACKLOG);

		log_trace(logger, "Esperando...");
		struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
		socklen_t addrlen = sizeof(addr);

		socketCliente = accept(listening_socket, (struct sockaddr *) &addr,
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
		loggear("Cerrando sesion...");

	}

	close(listening_socket);
	close(socketCliente);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "Planificador", true, LOG_LEVEL_TRACE);
	loggear("Nace el planificador.");
}

/*void loggear(char* mensaje) {
	log_trace(logger, mensaje);
}*/

