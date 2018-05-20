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
	iniciar(argv);

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			mensajeESI);
	int socket_planificador = conectar_a(IP_PLANIFICADOR, PUERTO_PLANIFICADOR,
			mensajeESI);

	/*while (1) {
		esperar_orden_de_parseo(socket_planificador,
				socket_coordinador, archivo_de_parseo);
	}*/

	loggear("Parseo exitoso. Cerrando sesion");

	avisar_cierre(socket_coordinador);
	avisar_cierre(socket_planificador);

	close(socket_planificador);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}

void iniciar(char** argv){
	iniciar_log("ESI", "ESI on duty!");
	lineas_parseadas = list_create();

	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	FILE* archivo_de_parseo = levantar_archivo(argv[1]);
	//FILE* archivo_de_parseo = levantar_archivo("script.esi");

	t_esi_operacion* parsed = malloc(sizeof(t_esi_operacion));

	while((read = getline(&line, &len, archivo_de_parseo)) != -1){
		*parsed = parsear(line);
		list_add(lineas_parseadas, parsed);
	}

	return;
}

FILE* levantar_archivo(char* archivo) {
	FILE* fp = fopen(archivo, "r");

	if (fp == NULL) {
		error_de_archivo("Error en abrir el archivo.", EXIT_FAILURE);
	}

	loggear("Archivo abierto correctamente.");

	return fp;
}

bool solicitar_permiso(int socket_coordinador) {
	package_pedido pedido_permiso = { .pedido = 1 };
	package_pedido respuesta;

	int packageSize = sizeof(pedido_permiso.pedido);
	char *message = malloc(packageSize);
	char *package = malloc(packageSize);

	serializar_pedido(pedido_permiso, &message);

	send(socket_coordinador, message, packageSize, 0);

	loggear("Solicitud enviada.");

	int res = recv(socket_coordinador, (void*) package, packageSize, 0);

	if (res != 0) {
		loggear("Solicitud confirmada.");
	} else {
		salir_con_error("Fallo la solicitud", socket_coordinador);
	}

	deserializar_pedido(&(respuesta), &(package));

	free(package);

	return respuesta.pedido == 1;
}

t_esi_operacion parsear(char* line) {
	t_esi_operacion parsed = parse(line);

	if (parsed.valido) {
		switch (parsed.keyword) {
		case GET:
			loggear("GET.");
			break;
		case SET:
			loggear("SET.");
			break;
		case STORE:
			loggear("STORE.");
			break;
		default:
			log_error(logger, "No se pudo interpretar la linea.");
			exit(EXIT_FAILURE);
		}

		destruir_operacion(parsed);
	} else {
		log_error(logger, "No se puedo interpretar correctamente el archivo.");
		exit(EXIT_FAILURE);
	}

	return parsed;
}

void error_de_archivo(char* mensaje_de_error, int retorno) {
	log_error(logger, mensaje_de_error);
	exit_gracefully(retorno);
}
