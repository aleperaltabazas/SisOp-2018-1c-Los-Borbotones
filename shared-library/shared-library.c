/*
 * shared-library.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "shared-library.h"

/*
 * shared-library.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "shared-library.h"

void enviar_aviso(int sockfd, aviso_ESI aviso) {
	int packageSize = sizeof(aviso.aviso) + sizeof(aviso.id);
	char* message = malloc(packageSize);

	serializar_aviso(aviso, &message);

	loggear("Serialice bien.");

	int envio = send(sockfd, message, packageSize, 0);

	if (envio < 0) {
		salir_con_error("Fallo el envio", sockfd);
	}

	free(message);

	loggear("Mensaje enviado.");
}

aviso_ESI recibir_aviso(int sockfd) {
	aviso_ESI ret_aviso;
	int packageSize = sizeof(aviso_ESI);
	char* package = malloc(packageSize);

	int res = recv(sockfd, package, packageSize, 0);

	if (res <= 0) {
		salir_con_error("Falló el recibo del aviso.", sockfd);
	}

	deserializar_aviso(&(ret_aviso), &(package));

	free(package);

	return ret_aviso;
}

void enviar_packed(package_int packed, int server_socket) {
	int packageSize = sizeof(package_int);
	char* message = malloc(packageSize);

	serializar_packed(packed, &message);

	int envio = send(server_socket, message, packageSize, 0);

	if (envio < 0) {
		salir_con_error("Falló el envío del paquete.", server_socket);
	}

	free(message);
}

package_int recibir_packed(int server_socket) {
	package_int ret_package;
	int packageSize = sizeof(package_int);
	char* package = malloc(packageSize);

	int res = recv(server_socket, package, packageSize, 0);

	if (res <= 0) {
		salir_con_error("Falló la recepción del paquete", server_socket);
	}

	deserializar_packed(&(ret_package), &(package));

	free(package);

	return ret_package;
}

void enviar_cadena(char* cadena, int server_socket) {
	int cadena_size = strlen(cadena) + 1;

	int envio = send(server_socket, cadena, cadena_size, 0);

	if (envio < 0) {
		salir_con_error("Falló el envío de la cadena.", server_socket);
	}
}

char* recibir_cadena(int server_socket, uint32_t size) {
	char* ret_string = malloc(size);

	int res = recv(server_socket, ret_string, size, 0);

	if (res <= 0) {
		salir_con_error("Falló la recepción de la cadena", server_socket);
	}

	return ret_string;
}

void terminar_conexion(int sockfd) {

	aviso_ESI aviso = { .aviso = -1 };

	int packageSize = sizeof(aviso_ESI);
	char* package = malloc(packageSize);

	serializar_aviso(aviso, &package);

	int envio = send(sockfd, package, packageSize, 0);

	if (envio < 0) {
		loggear("Fallo la terminación. Intentando de vuelta.");
		terminar_conexion(sockfd);
	}

	loggear("Terminación exitosa.");
}

void serializar_packed(package_int packed, char** message) {
	memcpy(*message, &(packed.packed), sizeof(packed.packed));
}

void deserializar_packed(package_int *packed, char** package) {
	memcpy(&packed->packed, *package, sizeof(packed->packed));
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

char * serializar_valores_set(int tamanio_a_enviar, parametros_set * valor_set) {

	char * buffer_parametros = malloc(tamanio_a_enviar);
	int offset = 0;
	int size_to_send;

	size_to_send =  sizeof(valor_set -> tamanio_clave);
	log_trace(logger, "%i", size_to_send);
	memcpy(buffer_parametros + offset, &(valor_set->tamanio_clave), size_to_send);
	offset += size_to_send;

	loggear("tamanio clave serializado");

	size_to_send =  valor_set -> tamanio_clave;
	memcpy(buffer_parametros + offset, valor_set->clave, size_to_send);
	offset += size_to_send;

	loggear("clave serializada");

	size_to_send =  sizeof(valor_set -> tamanio_valor);

	memcpy(buffer_parametros + offset, &(valor_set->tamanio_valor), size_to_send);
	offset += size_to_send;

	loggear("tamanio valor serializado");

	size_to_send =  valor_set -> tamanio_valor;
	memcpy(buffer_parametros + offset, valor_set->valor, size_to_send);
	offset += size_to_send;

	loggear("valor serializado");

	log_trace(logger, "%c, %c, %c", valor_set -> valor[0], valor_set -> valor[1], valor_set -> valor[2]);

	return buffer_parametros;
}

/*
 char* serializarOperandos(t_Package *package){

 char *serializedPackage = malloc(package->total_size);

 int offset = 0;
 int size_to_send;

 size_to_send =  sizeof(package->username_long);
 memcpy(serializedPackage + offset, &(package->username_long), size_to_send);
 offset += size_to_send;

 size_to_send =  package->username_long;
 memcpy(serializedPackage + offset, package->username, size_to_send);
 offset += size_to_send;

 size_to_send =  sizeof(package->message_long);
 memcpy(serializedPackage + offset, &(package->message_long), size_to_send);
 offset += size_to_send;

 size_to_send =  package->message_long;
 memcpy(serializedPackage + offset, package->message, size_to_send);

 return serializedPackage;
 }
 */

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

	int bindeo = bind(listening_socket, server_info->ai_addr,
			server_info->ai_addrlen);
	freeaddrinfo(server_info);

	if (bindeo < 0) {
		salir_con_error("Falló el bindeo.", 0);
	}

	return listening_socket;
}

int conectar_a(char *ip, char *puerto, package_int id) {
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

	char * buffer_id = malloc(sizeof(package_int));

	serializar_packed(id, &(buffer_id));

	send(server_socket, buffer_id, sizeof(package_int), 0);

	loggear("Mensaje enviado.");
	int res = recv(server_socket, (void*) buffer_id, sizeof(package_int), 0);

	deserializar_packed(&(id), &(buffer_id));

	if (res != 0) {
		loggear("Mensaje recibido desde el servidor. Identificando servidor...");
		chequear_servidor(id, server_socket);

	} else {
		salir_con_error("Fallo el envio de mensaje de parte del servidor.",
				server_socket);
	}

	loggear("Cerrando conexion con servidor y terminando.");

	return server_socket;
}

void chequear_servidor(package_int id, int server_socket) {
	char mensajeCoordinador[] = "Coordinador: taringuero profesional.";
	char mensajePlanificador[] =
			"My name is Planificador.c and I'm the fastest planifier alive...";

	if (id.packed == 0) {
		loggear(mensajeCoordinador);
	} else if (id.packed == 1) {
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
