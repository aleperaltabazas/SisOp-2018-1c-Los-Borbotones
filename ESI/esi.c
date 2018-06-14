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

int this_id;

int main(int argc, char** argv) {
	iniciar(argv);

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			mensajeESI);
	int socket_planificador = conectar_a(IP_PLANIFICADOR, PUERTO_PLANIFICADOR,
			mensajeESI);

	//asignar_ID(socket_planificador);

	this_id = recibir_ID(socket_planificador);

	ready(socket_planificador);

	while (parsed_ops.head != NULL) {
		esperar_ejecucion(socket_coordinador, socket_planificador);
	}

	avisar_cierre(socket_coordinador);
	avisar_cierre(socket_planificador);

	close(socket_planificador);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}

int recibir_ID(int server_socket) {
	aviso_ESI aviso;
	int packageSize = sizeof(aviso.aviso) + sizeof(aviso.id);
	char* package = malloc(packageSize);

	int res = recv(server_socket, (void*) package, packageSize, 0);

	deserializar_aviso(&(aviso), &(package));

	if (aviso.aviso == 0) {
		salir_con_error("Fin de este ESI por parte del planificador",
				server_socket);
	} else if (aviso.aviso != 1) {
		salir_con_error("Orden desconocida.", server_socket);
	}

	if (res != 0) {
		log_trace(logger, "ID: %i", aviso.id);
	} else {
		salir_con_error("Fallo el envio de ID.", server_socket);
	}

	free(package);

	return aviso.id;
}

void ready(int socket_planificador) {
	aviso_ESI aviso = { .aviso = 1, .id = this_id };

	int packageSize = sizeof(aviso.aviso) + sizeof(aviso.id);
	char* message = malloc(packageSize);

	serializar_aviso(aviso, &message);

	loggear("Serialice bien.");

	int envio = send(socket_planificador, message, packageSize, 0);

	if (envio < 0) {
		salir_con_error("Fallo el envio", socket_planificador);
	}

	free(message);

	loggear("Preparado.");
}

void esperar_ejecucion(int socket_coordinador, int socket_planificador) {
	loggear("Esperando orden de ejecucion del planificador.");

	aviso_ESI orden;

	int packageSize = sizeof(orden.aviso) + sizeof(orden.id);
	char *package = malloc(packageSize);

	int res = recv(socket_planificador, (void*) package, packageSize, 0);

	if (res != 0) {
		loggear("Orden confirmada.");
	} else {
		close(socket_coordinador);
		salir_con_error("Fallo la orden.", socket_planificador);
	}

	deserializar_aviso(&(orden), &(package));

	if (orden.aviso == -1) {
		close(socket_coordinador);
		loggear("Orden de terminación.");
		exit(1);
	} else if (orden.aviso == 2) {
		loggear(
				"Orden de ejecucion recibida. Solicitando permiso al coordinador.");
	} else {
		close(socket_coordinador);
		salir_con_error("Orden desconocida.", socket_planificador);
	}

	if (solicitar_permiso(socket_coordinador)) {
		ejecutar();
	}

	ready(socket_planificador);

}

void ejecutar(void) {
	t_esi_operacion parsed = first(parsed_ops);

	if (parsed.valido) {
		switch (parsed.keyword) {
		case GET:
			log_trace(logger, "GET %s", parsed.argumentos.GET.clave);
			break;
		case SET:
			log_trace(logger, "SET %s %s", parsed.argumentos.SET.clave,
					parsed.argumentos.SET.valor);
			break;
		case STORE:
			log_trace(logger, "STORE %s", parsed.argumentos.STORE.clave);
			break;
		default:
			break;
		}

	}

	eliminar_parseo(&parsed_ops);

	sleep(2);

}

void iniciar(char** argv) {
	iniciar_log("ESI", "ESI on duty!");
	lineas_parseadas = list_create();

	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	FILE* archivo_de_parseo = levantar_archivo(argv[1]);
	//FILE* archivo_de_parseo = levantar_archivo("script.esi");

	t_esi_operacion parsed;

	while ((read = getline(&line, &len, archivo_de_parseo)) != -1) {
		parsed = parsear(line);
		agregar_parseo(&parsed_ops, parsed);
	}

	loggear("Parseo exitoso.");

	return;
}

FILE* levantar_archivo(char* archivo) {
	FILE* fp = fopen(archivo, "r");

	if (fp == NULL) {
		error_de_archivo("Error en abrir el archivo.", 2);
	}

	loggear("Archivo abierto correctamente.");

	return fp;
}

bool solicitar_permiso(int socket_coordinador) {
	aviso_ESI pedido_permiso = { .aviso = 1, .id = this_id };

	aviso_ESI respuesta;

	int packageSize = sizeof(pedido_permiso.aviso) + sizeof(pedido_permiso.id);
	char *message = malloc(packageSize);
	char *package = malloc(packageSize);

	serializar_aviso(pedido_permiso, &message);

	send(socket_coordinador, message, packageSize, 0);

	loggear("Solicitud enviada.");

	int res = recv(socket_coordinador, (void*) package, packageSize, 0);

	if (res != 0) {
		loggear("Solicitud confirmada.");
	} else {
		salir_con_error("Fallo la solicitud.", socket_coordinador);
	}

	deserializar_aviso(&(respuesta), &(package));

	free(package);

	return respuesta.aviso == 1;
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
