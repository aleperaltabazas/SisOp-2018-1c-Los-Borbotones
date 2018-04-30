/*
 * coordinador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"

#define PUERTO "8000"
#define BACKLOG 5//Definimos cuantas conexiones pendientes al mismo tiempo tendremos
#define PACKAGE_SIZE 1024

int main() {
	iniciar_log();

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;		//Le indicamos localhost
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, PUERTO, &hints, &server_info);

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

	/*
	 *	int header;
	 *	char* payload;
	 *
	 *	header = recv(socketCliente, header, sizeof(int), 0);
	 *	payload = malloc(header + 1);
	 *
	 *	Intento bastante choto de serializar
	 *
	 */
	loggear("Esperando mensaje del cliente.");

	char package[PACKAGE_SIZE];
	char message[] = "Gracias por conectarse al coordinador!";
	char messageParaPlanificador[] = "Hola planificador! Trabajemos juntos para organizar los ESIs";

	int res = recv(socketCliente, (void*) package, PACKAGE_SIZE, 0);

	if (res <= 0) {
		loggear("Fallo la conexion con el cliente.");
		return 0;
	}

	if (package == "El planificador ha llegado")
	{
		send(socketCliente, messageParaPlanificador, strlen(messageParaPlanificador) + 1, 0);
	}

	else{
	loggear("Mensaje recibido exitosamente:");
	loggear(package);
	send(socketCliente, message, strlen(message) + 1, 0);

	loggear("Terminando conexion con el cliente.");
	loggear("Cerrando sesion...");
	}

	/*int i;
	 for(i = 0; i<4; i++){
	 sleep(1000);

	 loggear("...");

	 Podemos volarlo a la mierda esto, era para que quede mas cheto nomas
	 }*/

	close(socketCliente);
	close(listening_socket);
	return EXIT_SUCCESS;
}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logger, "Nace el coordinador.");
}

void loggear(char* mensaje) {
	log_trace(logger, mensaje);
}
