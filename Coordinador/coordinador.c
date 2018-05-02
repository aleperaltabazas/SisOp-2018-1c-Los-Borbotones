/*
 * coordinador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"

int main() {
	iniciar_log();

	int* id = 3;

	int socket_oyente;
	int socket_cliente;
	int socket_receptor;
	//char message[] = "Gracias por conectarse al coordinador!";

	while (1) {
		socket_oyente = escuchar_socket(PUERTO_COORDINADOR);
		socket_cliente = aceptar_conexion(socket_oyente);
		socket_receptor = recibir_mensaje(socket_cliente);
		enviar_mensaje(socket_receptor, id);
	}

	close(socket_receptor);

	return EXIT_SUCCESS;
}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logger, "Nace el coordinador.");
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
