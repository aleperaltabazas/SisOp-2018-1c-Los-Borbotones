/*
 * shared-library.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "shared-library.h"

<<<<<<< HEAD
void fill_package(t_esi_operacion parsed, package_ESI* solicitud) {
	int long_clave;
	int long_valor;
	char* clave;
	char* valor;

	switch (parsed.keyword) {
	case GET:
		strcpy(clave, parsed.argumentos.GET.clave);

		long_clave = strlen(clave);
		long_valor = 0;

		solicitud->aviso = 11;
		solicitud->long_clave = long_clave;
		strcpy(solicitud->clave, clave);
		solicitud->long_valor = long_valor;

		break;

	case SET:
		strcpy(clave, parsed.argumentos.GET.clave);
		strcpy(valor, parsed.argumentos.GET.clave);

		long_clave = strlen(clave);
		long_valor = strlen(valor);

		solicitud->aviso = 12;
		strcpy(solicitud->clave, clave);
		solicitud->long_clave = long_clave;
		strcpy(solicitud->valor, valor);
		solicitud->long_valor = long_valor;

		break;

	case STORE:
		strcpy(clave, parsed.argumentos.GET.clave);

		long_clave = strlen(clave);
		long_valor = 0;

		solicitud->aviso = 13;
		strcpy(solicitud->clave, clave);
		solicitud->long_clave = long_clave;
		solicitud->long_valor = long_valor;

		break;
	default:
		break;
	}

	solicitud->size = sizeof(solicitud->aviso) + sizeof(solicitud->clave) + long_clave + sizeof(solicitud->long_valor) + long_valor;
}

char* serializar_package(package_ESI *package) {
	char* serialized_package = malloc(package->size);

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(package->aviso);
	memcpy(serialized_package + offset, &(package->aviso), size_to_send);

	offset += size_to_send;

	size_to_send = sizeof(package->long_clave);
	memcpy(serialized_package + offset, &(package->long_clave), size_to_send);

	offset += size_to_send;

	size_to_send = package->long_clave;
	memcpy(serialized_package + offset, &package->clave, size_to_send);

	size_to_send = sizeof(package->long_valor);
	memcpy(serialized_package + offset, &(package->long_valor), size_to_send);

	offset += size_to_send;

	size_to_send = package->long_valor;
	memcpy(serialized_package + offset, &package->valor, size_to_send);

	return serialized_package;
}

int recibir_y_deserializar(package_ESI* package, int sockfd) {
	int buffer_size = sizeof(int);
	int status;
	char* buffer = malloc(buffer_size);

	int aviso;
	status = recv(sockfd, buffer, sizeof(package->aviso), 0);
	memcpy(&(aviso), buffer, buffer_size);
	if(!status) return 0;

	int long_clave;
	status = recv(sockfd, buffer, sizeof(package->long_clave), 0);
	memcpy(&(long_clave), buffer, buffer_size);
	if(!status) return 0;

	status = recv(sockfd, package->clave, long_clave, 0);
	if(!status) return 0;

	int long_valor;
	status = recv(sockfd, buffer, sizeof(package->long_valor), 0);
	memcpy(&(long_valor), buffer, buffer_size);
	if(!status) return 0;

	status = recv(sockfd, package->valor, long_valor, 0);
	if(!status) return 0;

	free(buffer);

	return status;
}

void terminar_conexion(int sockfd) {

	aviso_ESI aviso = { .aviso = -1 };
=======
void kill_ESI(int socket_cliente) {
	int status = 1;
	aviso_ESI orden_cierre = { .aviso = -1 };

	int packageSize = sizeof(orden_cierre.aviso) + sizeof(orden_cierre.id);
	char* message = malloc(packageSize);

	serializar_aviso(orden_cierre, &message);
>>>>>>> parent of 875678e... agregado t_esi_list y t_esi_nodo porque no puedo hacer andar las listas de las commons. agregue los dos algoritmos

	loggear("Terminando...");

	while (status) {
		int envio = send(socket_cliente, message, packageSize, 0);

		status = 0;

		if (envio < 0) {
			loggear("Fallo el envio. Intentando de nuevo en 5.");
			status = 1;

			sleep(5);
		}
	}

	loggear("Aviso exitoso.");
}

void serializar_pedido(package_pedido pedido, char** message) {
	memcpy(*message, &(pedido.pedido), sizeof(pedido.pedido));
}

void deserializar_pedido(package_pedido *pedido, char** package) {
	memcpy(&pedido->pedido, *package, sizeof(pedido->pedido));
}

void serializar_aviso(aviso_ESI aviso, char** message) {
	int offset = 0;

	memcpy(*message, &(aviso.aviso), sizeof(aviso.aviso));

	offset = sizeof(aviso.aviso);

	memcpy(*message + offset, &(aviso.id), sizeof(aviso.id));
}

void deserializar_aviso(aviso_ESI *aviso, char** package) {
	int offset = 0;

	memcpy(&aviso->aviso, *package, sizeof(aviso->aviso));

	offset = sizeof(aviso->aviso);

	memcpy(&aviso->id, *package + offset, sizeof(aviso->id));
}

void avisar_cierre(int server_socket) {
	int status = 1;
	aviso_ESI aviso_de_fin = { .aviso = 0 };

	int packageSize = sizeof(aviso_de_fin.aviso) + sizeof(aviso_de_fin.id);
	char *message = malloc(packageSize);

	serializar_aviso(aviso_de_fin, &message);

	loggear("Enviando aviso de fin.");

	while (status) {
		int envio = send(server_socket, message, packageSize, 0);

		status = 0;

		if (envio < 0) {
			loggear("Fallo el envio. Intentando de nuevo en 5.");
			status = 1;

			sleep(5);

			//HORRIBLE pero no se me ocurre mucho mas de como hacerlo
		}
	}

	loggear("Aviso exitoso.");
}

int levantar_servidor(char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;		//Le indicamos localhost
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, puerto, &hints, &server_info);

	int listening_socket = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);

	bind(listening_socket, server_info->ai_addr, server_info->ai_addrlen);
	freeaddrinfo(server_info);

	return listening_socket;
}

int conectar_a(char *ip, char *puerto, char* mensaje) {
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
		salir_con_error("Fallo la conexion con el servidor.", server_socket);
	}

	loggear("Conectó sin problemas");

	freeaddrinfo(serverInfo);

	char package[PACKAGE_SIZE];

	send(server_socket, mensaje, strlen(mensaje) + 1, 0);

	loggear("Mensaje enviado.");
	int res = recv(server_socket, (void*) package, PACKAGE_SIZE, 0);

	if (res != 0) {
		loggear(
				"Mensaje recibido desde el servidor. Identificando servidor...");
		chequear_servidor((char*) package, server_socket);

	} else {
		salir_con_error("Fallo el envio de mensaje de parte del servidor.",
				server_socket);
	}

	loggear("Cerrando conexion con servidor y terminando.");

	return server_socket;
}

void chequear_servidor(char* mensaje, int server_socket) {
	char mensajeCoordinador[] = "Coordinador: taringuero profesional.";
	char mensajePlanificador[] =
			"My name is Planificador.c and I'm the fastest planifier alive...";

	if (strcmp(mensaje, mensajeCoordinador) == 0) {
		loggear(mensajeCoordinador);
	} else if (strcmp(mensaje, mensajePlanificador) == 0) {
		loggear(mensajePlanificador);
	} else {
		salir_con_error("Servidor desconocido, cerrando conexion.",
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
	close(socket);

	exit_gracefully(1);
}

void exit_gracefully(int return_val) {
	//log_destroy(logger);
	exit(return_val);
}
