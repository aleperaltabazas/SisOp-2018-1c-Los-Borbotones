/*
 * coordinador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"

#define PUERTO "6667"
#define BACKLOG 5		//Definimos cuantas conexiones pendientes al mismo tiempo tendremos

int main(int argc, char** argv){
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;		//Le indicamos localhost
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, PUERTO, &hints, &server_info);
	//Le pasamos NULL en IP por el AI_PASSIVE

	int listening_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	//El socket que va a escuchar

	bind(listening_socket, server_info->ai_addr, server_info->ai_addrlen);
	freeaddrinfo(server_info);
	//Bindeamos el socket oyente y liberamos server_info

	listen(listening_socket, BACKLOG);
	//Le decimos que escuche

	//printf("%i", 42);

	return EXIT_SUCCESS;
}
