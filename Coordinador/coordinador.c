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

int main(int argc, char** argv) {
	iniciar_log("Coordinador", "Nace el coordinador...");

	char mensaje[] = "Coordinador: taringuero profesional.";

	int listening_socket = levantar_servidor(PUERTO_COORDINADOR);
	int socketCliente;

	while (1) {
		socketCliente = manejar_cliente(listening_socket, socketCliente, mensaje);
	}

	loggear("Cerrando sesion...");

	close(socketCliente);
	close(listening_socket);
	return EXIT_SUCCESS;
}

void* atender_ESI(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de ESI inicializado correctamente.");

	return NULL;
}

void* atender_Planificador(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de planificador inicializado correctamente.");

	return NULL;
}

void* atender_Instancia(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de instancia inicializado correctamente.");

	return NULL;
}
