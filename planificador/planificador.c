/*
 * planificador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */
#include "planificador.h"

int main() {
	iniciar_log();
	conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR, "My name is Planificador.c, and I'm the fastest planifier alive...");

	int socket_oyente;

	while (1) {
		socket_oyente = escuchar_socket(PUERTO_PLANIFICADOR);
		listen(socket_oyente, BACKLOG);
		aceptar_conexion(socket_oyente);
	}
}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logger, "Nace el planificador.");
}
