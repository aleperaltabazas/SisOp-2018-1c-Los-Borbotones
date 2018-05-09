/*
 * shared-library.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "shared-library.h"

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

void manejar_cliente(int listening_socket, int socketCliente, char* mensaje) {

	loggear("Esperando cliente...");

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	loggear("Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	char package[PACKAGE_SIZE];

	int res = recv(socketCliente, (void*) package, PACKAGE_SIZE, 0);

	if (res <= 0) {
		loggear("Fallo la conexion con el cliente.");
	}

	loggear("Mensaje recibido exitosamente. Identificando cliente...");
	identificar_cliente((char*) package, socketCliente);

	loggear("Enviando mensaje al cliente.");

	send(socketCliente, mensaje, strlen(mensaje) + 1, 0);

	loggear("Mensaje enviado. Cerrando sesion con el cliente actual.");
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

void identificar_cliente(char* mensaje, int socket_cliente) {
	char* mensajePlanificador =
			"My name is Planificador.c and I'm the fastest planifier alive...";
	char* mensajeESI = "A wild ESI has appeared!";
	char* mensajeInstancia = "It's ya boi, instancia!";

	/*
	 * Oh, Ale, para que mierda haces esto? Porque cuando no me de paja y entienda hilos
	 * aca le asignamos un thread. Tal vez tengamos que agregar el thread a la firma de la función -.-
	 */

	if (strcmp(mensaje, mensajePlanificador) == 0) {
		loggear(mensajePlanificador);
		pthread_create(&hilo_planificador, NULL, atender_Planificador,
				(void*) socket_cliente);
	} else if (strcmp(mensaje, mensajeESI) == 0) {
		loggear(mensajeESI);
		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);
	} else if (strcmp(mensaje, mensajeInstancia) == 0) {
		loggear(mensajeInstancia);
		pthread_create(&hilo_instancia, NULL, atender_Instancia,
				(void*) socket_cliente);
	} else {
		salir_con_error("Cliente desconocido, cerrando conexion.",
				socket_cliente);
	}

	return;
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

void* atender_Planificador(void* un_socket) {
	int i = 0;
	while (1) {
		loggear("Hola soy el hilo que atiende el planificador y estoy vivo.");
		sleep(5);

		if (i == 50) {
			break;
		}

		i++;
	}

	loggear("Hola soy el hilo del planificador y me voy a cerrar.");

	return NULL;
}

void* atender_ESI(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de ESI inicializado correctamente.");

	return NULL;
}

void* atender_Planificador(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de planificador inicializado correctamente.");

	return NULL;
}

void* atender_Instancia(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de instancia inicializado correctamente.");

	return NULL;
}
