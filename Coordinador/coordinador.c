/*
 * coordinador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"

#define PUERTO "6667"
#define BACKLOG 5//Definimos cuantas conexiones pendientes al mismo tiempo tendremos
#define PACKAGE_SIZE 1024

int main() {
	logger = log_create("ReDisTinto.log", "Coordinador", true, LOG_LEVEL_TRACE);

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;		//Le indicamos localhost
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, PUERTO, &hints, &server_info);
	//Le pasamos NULL en IP por el AI_PASSIVE

	log_trace(logger, "Nace el coordinador");

	int listening_socket = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);
	//El socket que va a escuchar

	bind(listening_socket, server_info->ai_addr, server_info->ai_addrlen);
	freeaddrinfo(server_info);
	//Bindeamos el socket oyente y liberamos server_info

	listen(listening_socket, BACKLOG);
	//Le decimos que escuche
	log_trace(logger, "Esperando ");
	struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	int socketCliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);
	//Nota: Tenemos n sockets address, y vamos a tener que ver como podemos tratar con ellos, se me ocurre threads pero también existe select
	char package[PACKAGE_SIZE];
	int stat = 1;		// Estructura que manjea el status de los recieve.

	printf("Cliente conectado. Esperando mensajes:\n");
	log_trace(logger, "Escuché"); //Creo que el log es mejor pero en el ejemplito usan printf

	while (stat != 0) {
		stat = recv(socketCliente, (void*) package, PACKAGE_SIZE, 0);
		if (stat != 0) {
			printf("%s", package);
		}

		responder();

	}

	close(socketCliente);
	close(listening_socket);
	return 0;
}

void responder() {

	char respuesta[1024] = "Todo OK";
	struct addrinfo hints;
	struct addrinfo *server_info;

	getaddrinfo(NULL, "6668", &hints, &server_info);
	//No se si se puede responder por el mismo puerto asi que pruebo con el 6668
	//Le pasamos NULL en IP por el AI_PASSIVE

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;		//Le indicamos localhost
	hints.ai_socktype = SOCK_STREAM;

	int socket_respuesta = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);
	//Socket para responder al que me mande un mensaje

	if (send(socket_respuesta, respuesta, PACKAGE_SIZE, 0) < 0) {
		perror("No pude contestar");
		//return(-1);
	}
	close(socket_respuesta);
}
