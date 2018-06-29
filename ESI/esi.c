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
	avisar_cierre(socket_coordinador);
	avisar_cierre(socket_planificador);

	close(socket_planificador);
	close(socket_coordinador);
}

void ejecutar_sentencias(void) {
	this_id = recibir_ID(socket_planificador);

	enviar_aviso(socket_planificador, aviso_ready);

	while (parsed_ops.head != NULL) {
		esperar_ejecucion(socket_coordinador, socket_planificador);
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

uint32_t recibir_ID(int server_socket) {
	aviso_con_ID aviso = recibir_aviso(server_socket);

	log_debug(logger, "%i", aviso.aviso);

	if (aviso.aviso == 0) {
		clear(&parsed_ops);
		salir_con_error("Fin de este ESI por parte del planificador",
				server_socket);
	} else if (aviso.aviso != 1) {
		clear(&parsed_ops);
		salir_con_error("Orden desconocida.", server_socket);
	}

	fill_ID(aviso.id);

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
}

void esperar_ejecucion(int socket_coordinador, int socket_planificador) {
	loggear("Esperando orden de ejecucion del planificador.");

	aviso_con_ID orden = { .aviso = -1 };

	orden = recibir_aviso(socket_planificador);

	log_debug(logger, "%i", orden.aviso);

	if (orden.aviso == -1) {
		log_info(logger, "Orden de terminación.");
		enviar_aviso(socket_coordinador, aviso_fin);

		clear(&parsed_ops);

		exit(1);
	} else if (orden.aviso == 2) {
		loggear("Orden de ejecucion recibida.");
	} else {
		clear(&parsed_ops);
		enviar_aviso(socket_coordinador, aviso_fin);
		salir_con_error("Orden desconocida.", socket_planificador);
	}

	ejecutar(socket_coordinador, socket_planificador);

}

void ejecutar(int socket_coordinador, int socket_planificador) {
	t_esi_operacion parsed = first(parsed_ops);

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

	if (res == 20) {
		eliminar_parseo(&parsed_ops);

		enviar_aviso(socket_planificador, aviso_ejecute);
	}

	else if (res == 5) {
		enviar_aviso(socket_planificador, aviso_bloqueo);
	}

	else {
		clear(&parsed_ops);
		log_error(logger, "Falló el retorno de la operación.");
		exit(-1);
	}

}

uint32_t get(t_esi_operacion parsed, int socket_coordinador) {
	enviar_aviso(socket_coordinador, aviso_get);

	aviso_con_ID aviso_coordi = recibir_aviso(socket_coordinador);

	if (aviso_coordi.aviso != 10) {
		clear(&parsed_ops);
		salir_con_error("Aviso desconocido", socket_coordinador);
	}

	char* clave = parsed.argumentos.GET.clave;
	uint32_t clave_size = (uint32_t) strlen(clave) + 1;

	package_int size_package = { .packed = clave_size };

	enviar_packed(size_package, socket_coordinador);
	sleep(1);
	enviar_cadena(clave, socket_coordinador);

	package_int response = recibir_packed(socket_coordinador);

	return response.packed;
}

uint32_t set(t_esi_operacion parsed, int socket_coordinador) {
	enviar_aviso(socket_coordinador, aviso_set);

	aviso_con_ID aviso_coordi = recibir_aviso(socket_coordinador);

	if (aviso_coordi.aviso != 10) {
		clear(&parsed_ops);
		salir_con_error("Aviso desconocido", socket_coordinador);
	}

	char* clave = parsed.argumentos.SET.clave;
	char* valor = parsed.argumentos.SET.valor;
	uint32_t clave_size = (uint32_t) strlen(clave) + 1;
	uint32_t valor_size = (uint32_t) strlen(clave) + 1;

	package_int clave_size_package = { .packed = clave_size };

	package_int valor_size_package = { .packed = valor_size };

	enviar_packed(clave_size_package, socket_coordinador);
	sleep(1);
	enviar_cadena(clave, socket_coordinador);
	sleep(1);
	enviar_packed(valor_size_package, socket_coordinador);
	sleep(1);
	enviar_cadena(valor, socket_coordinador);

	package_int response = recibir_packed(socket_coordinador);

	return response.packed;
}

uint32_t store(t_esi_operacion parsed, int socket_coordinador) {
	enviar_aviso(socket_coordinador, aviso_store);

	aviso_con_ID aviso_coordi = recibir_aviso(socket_coordinador);

	if (aviso_coordi.aviso != 10) {
		salir_con_error("Aviso desconocido", socket_coordinador);
	}

	char* clave = parsed.argumentos.STORE.clave;
	uint32_t clave_size = (uint32_t) strlen(clave) + 1;

	package_int size_package = { .packed = clave_size };

	enviar_packed(size_package, socket_coordinador);
	sleep(1);
	enviar_cadena(clave, socket_coordinador);

	package_int response = recibir_packed(socket_coordinador);

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

	while ((read = getline(&line, &len, archivo_de_parseo)) != -1) {
		parsed = parsear(line);
		agregar_parseo(&parsed_ops, parsed);
	}

	fclose(archivo_de_parseo);

	loggear("Parseo exitoso.");

	return;
}

void cargar_configuracion(char** argv) {
	t_config* config = config_create(argv[1]);

	PUERTO_COORDINADOR = config_get_string_value(config, "PUERTO_COORDINADOR");
	log_info(logger, "Puerto Coordinador: %s", PUERTO_COORDINADOR);

	IP_COORDINADOR = config_get_string_value(config, "IP_COORDINADOR");
	log_info(logger, "IP Coordinador: %s", IP_COORDINADOR);

	PUERTO_PLANIFICADOR = config_get_string_value(config,
			"PUERTO_PLANIFICADOR");
	log_info(logger, "Puerto Planificador: %s", PUERTO_PLANIFICADOR);

	IP_PLANIFICADOR = config_get_string_value(config, "IP_PLANIFICADOR");
	log_info(logger, "IP Planificador: %s", IP_PLANIFICADOR);

	loggear("Configuración cargada.");
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

void clear(t_parsed_list* lista) {
	while (lista->head != NULL) {
		eliminar_parseo(lista);
	}
}

t_parsed_node* crear_nodo(t_esi_operacion parsed) {
	t_parsed_node* nodo = (t_parsed_node*) malloc(sizeof(t_parsed_node));
	nodo->esi_op = parsed;
	nodo->sgte = NULL;

	return nodo;
}

void agregar_parseo(t_parsed_list* lista, t_esi_operacion parsed) {
	t_parsed_node* nodo = crear_nodo(parsed);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_parsed_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}

	return;
}

void destruir_nodo(t_parsed_node* nodo) {
	free(nodo);
}

void eliminar_parseo(t_parsed_list* lista) {
	if (!esta_vacia(lista)) {
		t_parsed_node* eliminado = lista->head;
		lista->head = lista->head->sgte;
		destruir_nodo(eliminado);
	}
}

bool esta_vacia(t_parsed_list* lista) {
	return lista->head == NULL;
}

t_esi_operacion first(t_parsed_list lista) {
	t_esi_operacion parsed = lista.head->esi_op;

	return parsed;
}

void error_de_archivo(char* mensaje_de_error, int retorno) {
	log_error(logger, mensaje_de_error);
	exit_gracefully(retorno);
}
