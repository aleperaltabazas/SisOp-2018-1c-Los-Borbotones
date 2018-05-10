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

	FILE* archivo_a_parsear;

	int res = recv(socket_planificador, (void*) archivo_a_parsear, sizeof(FILE),
	MSG_WAITALL);

	if (res != 0) {
		loggear("Archivo recibido. Solicitando permiso al coordinador.");
	} else {
		close(socket_coordinador);
		salir_con_error("Fallo la entrega del archivo.", socket_planificador);
	}

	if (!solicitar_permiso(socket_coordinador)) {
		close(socket_coordinador);
		salir_con_error("Permiso denegado.", socket_planificador);
	}

	loggear("Parseando...");
	parsear(archivo_a_parsear);

	loggear("Parseo terminado.");
}

bool solicitar_permiso(int socket_coordinador) {
	bool package;
	int pedido_de_parseo = 1;

	send(socket_coordinador, &pedido_de_parseo, sizeof(int), 0);

	int res = recv(socket_coordinador, (void*) package, sizeof(bool),
			MSG_WAITALL);

	if(res != 0){
		loggear("Solicitud confirmada.");
	}
	else{
		salir_con_error("Fallo la solicitud", socket_coordinador);
	}

	return package;
}

void parsear(FILE* archivo_a_parsear) {
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, archivo_a_parsear)) != -1) {
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
				loggear(
						strcat("STORE <CLAVE>: ",
								parsed.argumentos.STORE.clave));
				break;
			default:
				log_error(logger,
						"No se puedo interpretar correctamente el archivo.");
				exit(EXIT_FAILURE);
			}

			destruir_operacion(parsed);
		} else {
			log_error(logger,
					"No se puedo interpretar correctamente el archivo.");
			exit(EXIT_FAILURE);
		}
	}

	fclose(archivo_a_parsear);
	if (line)
		free(line);
}
