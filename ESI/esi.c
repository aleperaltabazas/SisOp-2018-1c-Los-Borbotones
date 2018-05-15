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

	FILE* archivo_de_parseo = levantar_archivo("script.esi");

	esperar_orden_de_parseo(socket_planificador, socket_coordinador, archivo_de_parseo);

	close(socket_planificador);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}

FILE* levantar_archivo(char* archivo){
	FILE* fp = fopen(archivo, "r");

	if(fp == NULL){
		error_de_archivo("Error en abrir el archivo.", EXIT_FAILURE);
	}

	loggear("Archivo abierto correctamente.");

	return fp;
}

void esperar_orden_de_parseo(int socket_planificador, int socket_coordinador, FILE* archivo_de_parseo) {
	loggear("Esperando orden de parseo del planificador");

	package_pedido pedido_parseo;
	int packageSize = sizeof(pedido_parseo.pedido);

	char *package = malloc(packageSize);

	int res = recv(socket_planificador, (void*) package, packageSize, 0);

	deserializar_pedido(&(pedido_parseo), &(package));

	if (res != 0) {
		loggear("Orden recibida. Solicitando permiso al coordinador.");
	} else {
		close(socket_coordinador);
		salir_con_error("Fallo la entrega de orden.",
				socket_planificador);
	}

	if (!solicitar_permiso(socket_coordinador)) {
		close(socket_coordinador);
		salir_con_error("Permiso denegado.", socket_planificador);
	}

	loggear("Parseando...");
	t_esi_operacion parseo = parsear(siguiente_linea(archivo_de_parseo));

	loggear("Parseo terminado.");
}

char* siguiente_linea(FILE* fp){
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	read = getline(&line, &len, fp);

	if(read == -1){
		loggear("No hay mas lineas para parsear");
		free(line);
	}

	return line;
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
			log_error(logger, "No se puedo parsear la linea.");
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
