/*
 * instancias.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "instancias.h"

int main(){
	iniciar_log();
	conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR, "It's ya boi, instancia!");

	return EXIT_SUCCESS;
}

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logger, "An instance joins the brawl!");
}



