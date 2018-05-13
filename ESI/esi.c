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

int main(int argc, char** argv) {
	iniciar_log("ESI", "ESI on duty!");

	char mensaje[] = "A wild ESI has appeared!";

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			mensaje);
	int socket_planificador = conectar_a(IP_PLANIFICADOR, PUERTO_PLANIFICADOR,
			mensaje);

	esperar_orden_de_parseo(socket_planificador, socket_coordinador);

	close(socket_planificador);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}

void esperar_orden_de_parseo(int socket_planificador, int socket_coordinador) {
	loggear("Esperando orden de parseo del planificador");

	char* line;

	int res = recv(socket_planificador, (void*) line, LINE_MAX, 0);

	if (res != 0) {
		loggear("Linea de parseo recibido. Solicitando permiso al coordinador.");
	} else {
		close(socket_coordinador);
		salir_con_error("Fallo la entrega de la linea de parseo.", socket_planificador);
	}

	if (!solicitar_permiso(socket_coordinador)) {
		close(socket_coordinador);
		salir_con_error("Permiso denegado.", socket_planificador);
	}

	loggear("Parseando...");
	parsear(line);

	loggear("Parseo terminado.");
}

bool solicitar_permiso(int socket_coordinador) {
	uint32_t package;
	uint32_t pedido_de_parseo = 1;

	send(socket_coordinador, &pedido_de_parseo, sizeof(int), 0);

	loggear("Solicitud enviada.");

	int res = recv(socket_coordinador, (void*) package, sizeof(uint32_t), 0);

	if (res != 0) {
		loggear("Solicitud confirmada.");
	} else {
		salir_con_error("Fallo la solicitud", socket_coordinador);
	}

	return package == 1;
}

void parsear(char* line) {
	t_esi_operacion parsed = parse(line);

	if (parsed.valido) {
		switch (parsed.keyword) {
		case GET:
			loggear(strcat("GET <CLAVE>: ", parsed.argumentos.GET.clave));
			break;
		case SET:
			loggear(strcat("SET <CLAVE>: ", parsed.argumentos.SET.clave));
			loggear(strcat("SET <VALOR>: ", parsed.argumentos.SET.valor));
			break;
		case STORE:
			loggear(strcat("STORE <CLAVE>: ", parsed.argumentos.STORE.clave));
			break;
		default:
			log_error(logger,
					"No se puedo parsear la linea.");
			exit(EXIT_FAILURE);
		}

		destruir_operacion(parsed);
	} else {
		log_error(logger, "No se puedo interpretar correctamente el archivo.");
		exit(EXIT_FAILURE);
	}
}

