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
	iniciar_log("Coordinador", "Nace el coordinador...");

	char mensaje[] = "Coordinador: taringuero profesional.";

	int listening_socket = levantar_servidor(PUERTO_COORDINADOR);
	int socketCliente;

	while (1) {
		manejar_cliente(listening_socket, socketCliente, mensaje);
	}

	loggear("Cerrando sesion...");

	close(socketCliente);
	close(listening_socket);
	return EXIT_SUCCESS;
}

