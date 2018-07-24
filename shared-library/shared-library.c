/*
 * shared-library.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "shared-library.h"

STORE_Op recv_store(int sockfd) {
	STORE_Op store;

	package_int size_package = recv_packed_no_exit(sockfd);
	char* clave = recv_string_no_exit(sockfd, size_package.packed);

	strcpy(store.clave, clave);

	return store;
}

SET_Op recv_set(int sockfd) {
	SET_Op set;

	package_int clave_size_package = recv_packed_no_exit(sockfd);
	char* clave = recv_string_no_exit(sockfd, clave_size_package.packed);

	strcpy(set.clave, clave);

	package_int valor_size_package = recv_packed_no_exit(sockfd);
	char* valor = recv_string_no_exit(sockfd, valor_size_package.packed);

	strcpy(set.valor, valor);

	return set;
}

GET_Op recv_get(int sockfd) {
	GET_Op get;

	package_int size_package = recv_packed_no_exit(sockfd);
	char* clave = recv_string_no_exit(sockfd, size_package.packed);

	strcpy(get.clave, clave);

	return get;
}

void send_store(STORE_Op store, int sockfd) {
	aviso_con_ID aviso_store = { .aviso = 13, .id = store.id };

	enviar_aviso(sockfd, aviso_store);

	uint32_t size = (uint32_t) strlen(store.clave) + 1;
	package_int size_package = { .packed = size };

	sleep(0.5);

	enviar_packed(size_package, sockfd);
	enviar_cadena(store.clave, sockfd);
}

void send_set(SET_Op set, int sockfd) {
	aviso_con_ID aviso_set = { .aviso = 12, .id = set.id };

	enviar_aviso(sockfd, aviso_set);

	uint32_t clave_size = (uint32_t) strlen(set.clave) + 1;
	package_int clave_size_package = { .packed = clave_size };

	sleep(0.5);

	enviar_packed(clave_size_package, sockfd);
	enviar_cadena(set.clave, sockfd);

	uint32_t valor_size = (uint32_t) strlen(set.valor) + 1;
	package_int valor_size_package = { .packed = valor_size };

	enviar_packed(valor_size_package, sockfd);
	enviar_cadena(set.valor, sockfd);
}

void send_get(GET_Op get, int sockfd) {
	aviso_con_ID aviso_get = { .aviso = 11, .id = get.id };

	enviar_aviso(sockfd, aviso_get);

	uint32_t size = (uint32_t) strlen(get.clave) + 1;
	package_int size_package = { .packed = size };

	sleep(0.5);

	enviar_packed(size_package, sockfd);
	enviar_cadena(get.clave, sockfd);
}

char* recv_string_no_exit(int sockfd, uint32_t size) {
	char* ret_string = malloc(size);

	int res = recv_string(ret_string, size, sockfd);

	if (res <= 0) {
		log_warning(logger, "Falló la recepción de la cadena: %s",
				strerror(errno));
		return string_recv_error;
	}

	loggear("Cadena recibida.");

	return ret_string;
}

aviso_con_ID recv_aviso_no_exit(int sockfd) {
	aviso_con_ID ret_aviso;

	int res = recv_aviso_con_ID(&ret_aviso, sockfd);

	if (res < 0) {
		log_warning(logger, "Falló la recepción del Aviso con ID: %s",
				strerror(errno));
		return aviso_recv_error;
	}

	loggear("Aviso con ID recibido.");

	return ret_aviso;
}

package_int recv_packed_no_exit(int sockfd) {
	package_int ret_package;

	int res = recv_package_int(&ret_package, sockfd);

	if (res < 0) {
		log_warning(logger, "Falló la recepción del Package Int: %s",
				strerror(errno));
		return packed_recv_error;
	}

	loggear("Package Int recibido.");

	return ret_package;
}

void send_string_no_exit(char* string, int sockfd) {
	int res = send_string(string, sockfd);

	if (res < 0) {
		log_warning(logger, "Falló el envío de la cadena: %s", strerror(errno));
		return;
	}

	loggear("Cadena enviada.");
}

void send_aviso_no_exit(aviso_con_ID aviso, int sockfd) {
	int res = send_aviso_con_ID(aviso, sockfd);

	if (res < 0) {
		log_warning(logger, "Falló el envío del Aviso con ID: %s",
				strerror(errno));
		return;
	}

	loggear("Aviso con ID enviado.");
}

void send_packed_no_exit(package_int package, int sockfd) {
	int res = send_package_int(package, sockfd);

	if (res < 0) {
		log_warning(logger, "Falló el envío del Package Int: %s",
				strerror(errno));
		return;
	}

	loggear("Package Int enviado.");

}

int recv_string(char* string, uint32_t size, int sockfd) {
	int res = recv(sockfd, string, size, 0);

	return res;
}

int recv_aviso_con_ID(aviso_con_ID* aviso, int sockfd) {
	int packageSize = sizeof(aviso_con_ID);
	char* message = malloc(packageSize);

	int res = recv(sockfd, message, packageSize, 0);

	deserializar_aviso(aviso, &(message));

	free(message);

	return res;
}

int recv_package_int(package_int* package, int sockfd) {
	int packageSize = sizeof(package_int);
	char* message = malloc(packageSize);

	int res = recv(sockfd, message, packageSize, 0);

	deserializar_packed(package, &(message));

	free(message);

	return res;
}

int send_package_int(package_int package, int sockfd) {
	int packageSize = sizeof(package_int);
	char* message = malloc(packageSize);

	serializar_packed(package, &message);

	int envio = send(sockfd, message, packageSize, MSG_NOSIGNAL);

	log_debug(logger, "Bytes enviados: %i", envio);

	free(message);

	return envio;
}

int send_aviso_con_ID(aviso_con_ID aviso, int sockfd) {
	int packageSize = sizeof(aviso.aviso) + sizeof(aviso.id);
	char* message = malloc(packageSize);

	serializar_aviso(aviso, &message);

	loggear("Serialice bien.");

	int envio = send(sockfd, message, packageSize, 0);

	log_debug(logger, "Bytes enviados: %i", envio);

	free(message);

	return envio;
}

int send_string(char* string, int sockfd) {
	cerrar_cadena(string);
	int cadena_size = strlen(string) + 1;

	int envio = send(sockfd, string, cadena_size, 0);

	log_debug(logger, "Bytes enviados: %i", envio);

	return envio;
}

void enviar_aviso(int sockfd, aviso_con_ID aviso) {
	int res = send_aviso_con_ID(aviso, sockfd);

	if (res < 0) {
		log_error(logger, "%s", strerror(errno));
		salir_con_error("Falló el envío del Aviso con ID.", sockfd);
	}

	loggear("Aviso con ID enviado.");
}

aviso_con_ID recibir_aviso(int sockfd) {
	aviso_con_ID ret_aviso;

	int res = recv_aviso_con_ID(&ret_aviso, sockfd);

	if (res < 0) {
		log_error(logger, "%s", strerror(errno));
		salir_con_error("Falló la recepción del Aviso con ID", sockfd);
	}

	loggear("Aviso con ID recibido.");

	return ret_aviso;
}

void enviar_packed(package_int packed, int sockfd) {
	int res = send_package_int(packed, sockfd);

	if (res < 0) {
		log_error(logger, "%s", strerror(errno));
		salir_con_error("Falló el envío del Package Int.", sockfd);
	}

	loggear("Package Int enviado.");
}

package_int recibir_packed(int sockfd) {
	package_int ret_package;

	int res = recv_package_int(&ret_package, sockfd);

	if (res < 0) {
		log_error(logger, "%s", strerror(errno));
		salir_con_error("Falló la recepción del Package Int.", sockfd);
	}

	loggear("Package Int recibido.");

	return ret_package;
}

void enviar_cadena(char* cadena, int sockfd) {
	int res = send_string(cadena, sockfd);

	if (res < 0) {
		log_error(logger, "%s", strerror(errno));
		salir_con_error("Falló el envío de la cadena", sockfd);
	}

	loggear("Cadena enviada.");
}

char* recibir_cadena(int sockfd, uint32_t size) {
	char* ret_string = malloc(size);

	int res = recv_string(ret_string, size, sockfd);

	if (res <= 0) {
		log_error(logger, "%s", strerror(errno));
		salir_con_error("Falló la recepción de la cadena", sockfd);
	}

	loggear("Cadena recibida");

	return ret_string;
}

void terminar_conexion(int sockfd, bool retry) {

	aviso_con_ID aviso = { .aviso = -1 };

	int packageSize = sizeof(aviso_con_ID);
	char* package = malloc(packageSize);

	serializar_aviso(aviso, &package);

	int envio = send(sockfd, package, packageSize, 0);

	if (envio < 0) {
		if (retry) {
			log_warning(logger,
					"Fallo la terminación. Intentando de vuelta: %s",
					strerror(errno));
			terminar_conexion(sockfd, true);
		} else {
			log_warning(logger, "Falló la terminación: %s", strerror(errno));
		}
	}

	log_trace(logger, "Terminación exitosa.");
}

void serializar_packed(package_int packed, char** message) {
	memcpy(*message, &(packed.packed), sizeof(packed.packed));
}

void deserializar_packed(package_int *packed, char** package) {
	memcpy(&packed->packed, *package, sizeof(packed->packed));
}

void serializar_aviso(aviso_con_ID aviso, char** message) {
	int offset = 0;

	memcpy(*message, &(aviso.aviso), sizeof(aviso.aviso));

	offset = sizeof(aviso.aviso);

	memcpy(*message + offset, &(aviso.id), sizeof(aviso.id));
}

void deserializar_aviso(aviso_con_ID *aviso, char** package) {
	int offset = 0;

	memcpy(&aviso->aviso, *package, sizeof(aviso->aviso));

	offset = sizeof(aviso->aviso);

	memcpy(&aviso->id, *package + offset, sizeof(aviso->id));
}

char * serializar_valores_set(int tamanio_a_enviar, parametros_set * valor_set) {

	char * buffer_parametros = malloc(tamanio_a_enviar);
	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(uint32_t);

	memcpy(buffer_parametros + offset, &(valor_set->tamanio_clave),
			size_to_send);
	offset += size_to_send;

	size_to_send = valor_set->tamanio_clave;
	memcpy(buffer_parametros + offset, valor_set->clave, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);

	memcpy(buffer_parametros + offset, &(valor_set->tamanio_valor),
			size_to_send);
	offset += size_to_send;

	size_to_send = valor_set->tamanio_valor;
	memcpy(buffer_parametros + offset, valor_set->valor, size_to_send);
	offset += size_to_send;

	return buffer_parametros;
}

void avisar_cierre(int server_socket) {
	int status = 1;
	aviso_con_ID aviso_de_fin = { .aviso = 0 };

	int packageSize = sizeof(aviso_de_fin.aviso) + sizeof(aviso_de_fin.id);
	char *message = malloc(packageSize);

	serializar_aviso(aviso_de_fin, &message);

	loggear("Enviando aviso de fin.");

	while (status) {
		int envio = send(server_socket, message, packageSize, 0);

		status = 0;

		if (envio < 0) {
			log_warning(logger, "Fallo el envio. Intentando de nuevo en 5: %s",
					strerror(errno));
			status = 1;

			sleep(5);
		}
	}

	loggear("Aviso exitoso.");
}

int levantar_servidor(char* puerto, int tries) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;		//Le indicamos localhost
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, puerto, &hints, &server_info);

	int listening_socket = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);

	int yes = 1;
	int setsock = setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(yes));

	if (setsock == -1) {
		salir_con_error("Falló el setsockopt", listening_socket);
	}

	int bindeo = bind(listening_socket, server_info->ai_addr,
			server_info->ai_addrlen);
	freeaddrinfo(server_info);

	if (bindeo < 0) {
		if (tries == 5) {
			log_error(logger, "%s", strerror(errno));
			salir_con_error(
					"El bindeo falló demasiadas veces. Intente de vuelta más tarde. Cerrando proceso...",
					0);
		}

		log_warning(logger, "Falló el bindeo. Intentando de nuevo: %s",
				strerror(errno));
		sleep(3);

		levantar_servidor(puerto, tries + 1);
	}

	log_info(logger, "Servidor levantado.");

	return listening_socket;
}

int conectar_a(char *ip, char *puerto, package_int id, int tries) {
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(ip, puerto, &hints, &serverInfo);
	int server_socket;
	server_socket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	int conexion = connect(server_socket, serverInfo->ai_addr,
			serverInfo->ai_addrlen);

	if (conexion < 0) {
		if (tries == 5) {
			log_error(logger, "%s", strerror(errno));
			salir_con_error(
					"Falló la conexión con el servidor. Por favor intente de nuevo más tarde. Cerrando proceso...",
					server_socket);
		}

		log_warning(logger,
				"Falló la conexión con el servidor. Intentando de nuevo: %s",
				strerror(errno));
		sleep(3);

		conectar_a(ip, puerto, id, tries + 1);
	}

	log_info(logger, "Conectó sin problemas.");

	freeaddrinfo(serverInfo);

	enviar_packed(id, server_socket);

	loggear("Mensaje enviado.");

	package_int server_package = { .packed = -1 };
	server_package = recibir_packed(server_socket);

	chequear_servidor(server_package, server_socket);

	log_info(logger, "Handshake realizado sin problemas.");

	return server_socket;
}

void chequear_servidor(package_int id, int server_socket) {

	log_debug(logger, "%i", id);

	if (id.packed == 0) {
		log_info(logger, "Servidor reconocido.");
		log_info(logger, mensajeCoordinador);
	} else if (id.packed == 1) {
		log_info(logger, "Servidor reconocido.");
		log_info(logger, mensajePlanificador);
	} else {
		salir_con_error("Servidor desconocido, cerrando conexión.",
				server_socket);
	}

	return;
}

void iniciar_log(char* nombre, char* mensajeInicial) {
	logger = log_create("ReDisTinto.log", nombre, true, LOG_LEVEL_TRACE);
	loggear(mensajeInicial);
}

void loggear(char* mensaje) {
	log_trace(logger, mensaje);
}

void salir_con_error(char* mensaje, int socket) {
	log_error(logger, mensaje);
	log_error(logger, "Error de errno: %s", strerror(errno));
	close(socket);

	exit_gracefully(1);
}

void exit_gracefully(int return_val) {
	exit(return_val);
}
