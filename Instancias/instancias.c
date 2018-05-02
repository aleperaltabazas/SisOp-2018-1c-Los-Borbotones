/*
 * instancias.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "instancias.h"

int main(){
	iniciar_log();

	int id = 2;

	conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR);

	return EXIT_SUCCESS;
}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logger, "A new instance joins the brawl!");
}



