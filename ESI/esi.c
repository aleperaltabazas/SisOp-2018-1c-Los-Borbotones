/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

#define IP "127.0.0.1"
#define PUERTO "6667"

int main(int argv, char** argc){
	inicializar_log("esi.log", "ESI");

	int server_socket = conectar_a(IP, PUERTO);

	return EXIT_SUCCESS;
}

