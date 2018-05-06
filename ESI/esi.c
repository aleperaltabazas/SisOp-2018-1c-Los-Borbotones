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
	iniciar_log("ESI", "ESI on duty!");

	char mensaje[] = "A wild ESI has appeared!";

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR, mensaje);
	int socket_planificador = conectar_a(IP_PLANIFICADOR, PUERTO_PLANIFICADOR, mensaje);

	close(socket_planificador);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}




