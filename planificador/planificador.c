/*
 * planificador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */
#include "planificador.h"

int main() {
	iniciar_log();

	//char mensaje[] = "My name is Planificador.c, and I'm the fastest planifier alive...";
	int id = 0;

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR);
	enviar_identificacion(socket_coordinador, id);
	esperar_confirmacion(socket_coordinador);

	int socket_oyente;

	while (1) {
		socket_oyente = escuchar_socket(PUERTO_PLANIFICADOR);
		aceptar_conexion(socket_oyente);
		enviar_mensaje(socket_oyente, id);
		recibir_mensaje(socket_oyente);
	}

	close(socket_oyente);
}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logger, "Nace el planificador.");
}

/*---Consolita---
 *  {
 *  int * comando = malloc(sizeof(int));
 *
 *  while (1) {
 *  printf ("Bienvenido a la consola interactiva para el planificador /n /n");
 *  listarOpciones();
 *  recibirCodigo();
 *  interpretarYEjecutarCodigo(comando);
 *  }
 * }
 */
