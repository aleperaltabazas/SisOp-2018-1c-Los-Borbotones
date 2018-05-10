/*
 * instancias.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "instancias.h"

int main(int argc, char** argv) {
	iniciar_log("Instancias", "A new Instance joins the brawl!");

	char mensaje[] = "It's ya boi, instancia!";

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR, mensaje);

	loggear("Cerrando conexion con servidor y terminando.");

	close(socket_coordinador);

	return EXIT_SUCCESS;
}

