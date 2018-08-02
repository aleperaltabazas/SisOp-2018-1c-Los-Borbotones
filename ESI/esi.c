/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

int main(int argc, char** argv) {

	iniciar(argv);
	establecer_conexiones();

	ejecutar_sentencias();
	cerrar();

	return EXIT_SUCCESS;
}

void cerrar(void) {
	avisar_cierre(socket_coordinador, this_id);
	avisar_cierre(socket_planificador, this_id);

	close(socket_planificador);
	close(socket_coordinador);
}

void ejecutar_sentencias(void) {
	this_id = recibir_ID(socket_planificador);

	avisar_ID(socket_coordinador);

	enviar_aviso(socket_planificador, aviso_ready);

	while (parsed_ops.head != NULL) {
		esperar_ejecucion(socket_coordinador, socket_planificador);
	}
}

void avisar_ID(int sockfd) {
	enviar_aviso(sockfd, aviso_ID);

	loggear("ID enviado correctamente.");

	package_int packed = recibir_packed(sockfd);

	if (packed.packed != 42) {
		salir_con_error("Falló el aviso OK del coordinador.", sockfd);
	}
}

void establecer_conexiones(void) {
	loggear("Estableciendo conexiones...");

	socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR, id_ESI,
			0);
	socket_planificador = conectar_a(IP_PLANIFICADOR, PUERTO_PLANIFICADOR,
			id_ESI, 0);

	loggear("Conexiones realizadas.");
}

void swap_values(aviso_con_ID* aviso) {
	uint32_t aviso_aux = aviso->aviso;
	aviso->aviso = aviso->id;
	aviso->id = aviso_aux;

	log_debug(logger, "Nuevo aviso: %i", aviso->aviso);
	log_debug(logger, "Nuevo ID: %i", aviso->id);
}

uint32_t recibir_ID(int server_socket) {
	aviso_con_ID aviso = recibir_aviso(server_socket);

	log_debug(logger, "Aviso: %i", aviso.aviso);
	log_debug(logger, "ID: %i", aviso.id);

	if (aviso.id == 1 && aviso.aviso > 1) {
		swap_values(&aviso);
	}

	if (aviso.id <= 0) {
		salir_con_error("Id erróneo.", server_socket);
	}

	if (aviso.aviso == 0) {
		clear(&parsed_ops);
		salir_con_error("Fin de este ESI por parte del planificador",
				server_socket);
	} else if (aviso.aviso != 1) {
		clear(&parsed_ops);
		salir_con_error("Orden desconocida.", server_socket);
	}

	fill_ID(aviso.id);

	log_info(logger, "ID: %i", aviso.id);

	return aviso.id;
}

void fill_ID(uint32_t id) {
	aviso_fin.id = id;
	aviso_ready.id = id;
	aviso_bloqueo.id = id;
	aviso_ejecute.id = id;
	aviso_get.id = id;
	aviso_set.id = id;
	aviso_store.id = id;
	aviso_ID.id = id;
}

void esperar_ejecucion(int socket_coordinador, int socket_planificador) {
	loggear("Esperando orden de ejecucion del planificador.");

	aviso_con_ID orden = { .aviso = -1 };

	orden = recibir_aviso(socket_planificador);

	log_debug(logger, "%i", orden.aviso);

	if (orden.aviso == -1) {
		log_info(logger, "Orden de terminación.");
		cerrar();

		exit(1);
	} else if (orden.aviso == 2) {
		loggear("Orden de ejecucion recibida.");
	} else {
		salir_con_error("Orden desconocida.", socket_planificador);
	}

	ejecutar(socket_coordinador, socket_planificador);

}

void ejecutar(int socket_coordinador, int socket_planificador) {
	t_esi_operacion parsed = headParsed(parsed_ops);

	int res;

	if (parsed.valido) {
		switch (parsed.keyword) {
		case GET:
			log_info(logger, "GET %s", parsed.argumentos.GET.clave);
			res = get(parsed, socket_coordinador);
			break;
		case SET:
			log_info(logger, "SET %s %s", parsed.argumentos.SET.clave,
					parsed.argumentos.SET.valor);
			res = set(parsed, socket_coordinador);
			break;
		case STORE:
			log_info(logger, "STORE %s", parsed.argumentos.STORE.clave);
			res = store(parsed, socket_coordinador);
			break;
		default:
			break;
		}

	}

	if (res == -1) {
		log_error(logger, "ABORT");

		enviar_aviso(socket_coordinador, aviso_abort);
		enviar_aviso(socket_planificador, aviso_abort);
		exit_gracefully(5);
	}

	if (res == 20) {
		eliminar_parseo(&parsed_ops);

		enviar_aviso(socket_planificador, aviso_ejecute);
	}

	else if (res == 5) {
		enviar_aviso(socket_planificador, aviso_bloqueo);
	}

	else {
		log_error(logger, "Falló el retorno de la operación.");
		exit(-1);
	}

}

op_response recibir_respuesta(int sockfd) {
	op_response response = recibir_packed(sockfd);

	return response;
}

uint32_t get(t_esi_operacion parsed, int socket_coordinador) {
	GET_Op get = { .id = this_id };
	strcpy(get.clave, parsed.argumentos.GET.clave);

	send_get(get, socket_coordinador);
	op_response response = recibir_respuesta(socket_coordinador);

	return response.packed;
}

uint32_t set(t_esi_operacion parsed, int socket_coordinador) {
	SET_Op set = { .id = this_id };
	strcpy(set.clave, parsed.argumentos.SET.clave);
	strcpy(set.valor, parsed.argumentos.SET.valor);

	send_set(set, socket_coordinador);
	op_response response = recibir_respuesta(socket_coordinador);
	return response.packed;
}

uint32_t store(t_esi_operacion parsed, int socket_coordinador) {
	STORE_Op store = { .id = this_id };
	strcpy(store.clave, parsed.argumentos.STORE.clave);

	send_store(store, socket_coordinador);
	op_response response = recibir_respuesta(socket_coordinador);

	return response.packed;
}

void iniciar(char** argv) {
	iniciar_log("ESI", "ESI on duty!");
	loggear("Cargando configuración.");
	cargar_configuracion(argv);

	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	FILE* archivo_de_parseo = levantar_archivo(argv[2]);

	t_esi_operacion parsed;

	loggear("Parseando sentencias.");

	int totalLines = 0;
	int parsedLines = 0;
	int skippedLines = 0;

	while ((read = getline(&line, &len, archivo_de_parseo)) != -1) {
		if (line[0] == '\n') {
			log_warning(logger, "Empty line %i.", totalLines);
			skippedLines++;
			continue;
		}

		parsed = parsear(line);

		if (parsed.valido) {
			agregar_parseo(&parsed_ops, parsed);
			parsedLines++;
		} else {
			skippedLines++;
		}

		totalLines++;
	}

	log_info(logger, "Total de líneas leídas: %i", totalLines);
	log_info(logger, "Total de líneas parseadas: %i", parsedLines);
	log_info(logger, "Total de líneas salteadas: %i", skippedLines);

	if (line)
		free(line);

	fclose(archivo_de_parseo);

	loggear("Parseo exitoso.");

	return;
}

void cargar_configuracion(char** argv) {
	t_config* config = config_create(argv[1]);

	char* puerto_coordi = config_get_string_value(config, "PUERTO_COORDINADOR");
	PUERTO_COORDINADOR = transfer(puerto_coordi, strlen(puerto_coordi) + 1);
	log_info(logger, "Puerto Coordinador: %s", PUERTO_COORDINADOR);

	char* ip_coordi = config_get_string_value(config, "IP_COORDINADOR");
	IP_COORDINADOR = transfer(ip_coordi, strlen(ip_coordi) + 1);
	log_info(logger, "IP Coordinador: %s", IP_COORDINADOR);

	char* puerto_plani = config_get_string_value(config, "PUERTO_PLANIFICADOR");
	PUERTO_PLANIFICADOR = transfer(puerto_plani, strlen(puerto_plani) + 1);
	log_info(logger, "Puerto Planificador: %s", PUERTO_PLANIFICADOR);

	char* ip_plani = config_get_string_value(config, "IP_PLANIFICADOR");
	IP_PLANIFICADOR = transfer(ip_plani, strlen(ip_plani) + 1);
	log_info(logger, "IP Planificador: %s", IP_PLANIFICADOR);

	loggear("Configuración cargada.");
	config_destroy(config);
}

FILE* levantar_archivo(char* archivo) {
	FILE* fp = fopen(archivo, "r");

	if (fp == NULL) {
		error_de_archivo("Error en abrir el archivo.", 2);
	}

	loggear("Archivo abierto correctamente.");

	return fp;
}

t_esi_operacion parsear(char* line) {
	t_esi_operacion parsed = parse(line);

	if (parsed.valido) {
		switch (parsed.keyword) {
		case GET:
			break;
		case SET:
			break;
		case STORE:
			break;
		default:
			log_warning(logger, "No se pudo interpretar la linea.");
			log_debug(logger, "Line: %s", line);
			parsed.valido = false;
			break;
		}

	}

	return parsed;
}

void clear(t_parsed_list* lista) {
	while (lista->head != NULL) {
		eliminar_parseo(lista);
	}
}

void error_de_archivo(char* mensaje_de_error, int retorno) {
	log_error(logger, mensaje_de_error);
	exit_gracefully(retorno);
}
