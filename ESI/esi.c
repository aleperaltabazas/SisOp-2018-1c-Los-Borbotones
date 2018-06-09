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

	loggear("Parseo exitoso.");

	this_id = recibir_ID(socket_planificador);

	ready(socket_planificador, aviso_ready);

	while (lineas_parseadas->head != NULL) {
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
	}

	free(package);

	return aviso.id;
}

void ready(int socket_planificador, aviso_ESI aviso) {
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

<<<<<<< HEAD
	aviso_ESI aviso = ejecutar(socket_planificador, socket_coordinador);

	ready(socket_planificador, aviso);
=======
	if (solicitar_permiso(socket_coordinador)) {
		ejecutar();
	}
>>>>>>> parent of 722a936... agrego esqueleto al esi para sus operaciones

	ready(socket_planificador);

}

<<<<<<< HEAD
aviso_ESI ejecutar(int socket_planificador, int socket_coordinador) {
	aviso_ESI ret_aviso = determinar_operacion();
	ret_aviso.id = this_id;

	if (!solicitar_permiso(socket_coordinador, ret_aviso)) {
		return aviso_bloqueo;
	}

	eliminar_parseo(&parsed_ops);

	sleep(5);

	return ret_aviso;
=======
void ejecutar(void) {
	list_remove(lineas_parseadas, 0);

	sleep(5);
>>>>>>> parent of 722a936... agrego esqueleto al esi para sus operaciones
}

void iniciar(char** argv) {
	iniciar_log("ESI", "ESI on duty!");
	lineas_parseadas = list_create();

	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	FILE* archivo_de_parseo = levantar_archivo(argv[1]);
	//FILE* archivo_de_parseo = levantar_archivo("script.esi");

	t_esi_operacion* parsed = malloc(sizeof(t_esi_operacion));

	while ((read = getline(&line, &len, archivo_de_parseo)) != -1) {
		*parsed = parsear(line);
		list_add(lineas_parseadas, parsed);
	}

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

aviso_ESI determinar_operacion(void) {
	aviso_ESI ret_aviso;
	t_esi_operacion parsed = first(parsed_ops);

	switch (parsed.keyword) {
	case GET:
		ret_aviso.aviso = 11;
		break;
	case SET:
		ret_aviso.aviso = 12;
		break;
	case STORE:
		ret_aviso.aviso = 13;
		break;
	default:
		break;
	}

	return ret_aviso;
}

void get_clave(char* clave) {
	t_esi_operacion parsed = first(parsed_ops);

	switch (parsed.keyword) {
	case GET:
		strcpy(clave, parsed.argumentos.GET.clave);
		break;
	case SET:
		strcpy(clave, parsed.argumentos.SET.clave);
		break;
	case STORE:
		strcpy(clave, parsed.argumentos.STORE.clave);
		break;
	}
}

void get_valor(char* valor) {
	t_esi_operacion parsed = first(parsed_ops);

	strcpy(valor, parsed.argumentos.SET.valor);
}

bool es_set(aviso_ESI solicitud) {
	return solicitud.aviso == 12;
}

bool solicitar_permiso(int socket_coordinador, aviso_ESI aviso) {
	aviso_ESI respuesta;

	package_ESI package_pedido;

	int packageSize = sizeof(aviso_ESI);
	char *package = malloc(packageSize);
	char *message = malloc(packageSize);

	/*char* serialized_package = serializar_package(&package_pedido);

	int envio = send(socket_coordinador, serialized_package,
			package_pedido.size, 0);*/

	serializar_aviso(aviso, &message);

	int envio = send(socket_coordinador, message, packageSize, 0);

	if (envio < 0) {
		salir_con_error("Fallo la solicitud", socket_coordinador);
	}

	loggear("Se envió correctamente la solicitud. Esperando respuesta.");

	int res = recv(socket_coordinador, (void*) package, packageSize, 0);

	if (res != 0) {
		loggear("Solicitud confirmada.");
	} else {
		salir_con_error("Fallo la solicitud.", socket_coordinador);
	}

	deserializar_aviso(&(respuesta), &(package));

	free(package);
	free(message);

	return respuesta.aviso == 1;
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
