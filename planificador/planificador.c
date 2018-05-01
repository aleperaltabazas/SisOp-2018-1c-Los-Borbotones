/*
 * planificador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */
#include "planificador.h"

int main() {
	iniciar_log();

	char mensaje[] = "My name is Planificador.c, and I'm the fastest planifier alive...";

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR, mensaje);

	int socket_oyente;
	int socket_receptor;
	int socket_cliente;

	while (1) {
		socket_oyente = escuchar_socket(PUERTO_PLANIFICADOR);
		socket_cliente = aceptar_conexion(socket_oyente);
		socket_receptor = recibir_mensaje(socket_cliente);
		enviar_mensaje(socket_receptor, mensaje);
	}
}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logger, "Nace el planificador.");
}
