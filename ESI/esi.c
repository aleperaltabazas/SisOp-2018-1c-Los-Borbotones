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

int main(int argv, char** argc){
	//printf("%i", 42);
	//printf("%i", 42);
	inicializar_log("esi.log", "ESI");

	//printf("%i", 42);
	//Llega a tirar hasta este 42, lo que quiere decir que no sigue porque esta tratando de conectarse
	//Pero no puede porque el otro programa no esta disponible ;)

	int server_socket = conectar_a(IP, PUERTO);

	printf("%i", 42);

	char mensaje[PACKAGE_SIZE] = "ESI reportandose al trabajo.";

	send(server_socket, mensaje, strlen(mensaje) + 1, 0);
	printf("%i", 42);

	return EXIT_SUCCESS;
}

