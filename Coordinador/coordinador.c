/*
 * coordinador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"

#define PUERTO "6667"
#define BACKLOG 5//Definimos cuantas conexiones pendientes al mismo tiempo tendremos
#define PACKAGE_SIZE 1024

int main(int argc, char** argv) {
	iniciar_log("Coordinador", "Nace el coordinador...");

	int listening_socket = levantar_servidor(PUERTO_COORDINADOR);
	int socketCliente;

	while (1) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				mensajeCoordinador);
	}

	loggear("Cerrando sesion...");

	close(socketCliente);
	close(listening_socket);
	return EXIT_SUCCESS;
}

int manejar_cliente(int listening_socket, int socketCliente, char* mensaje) {

	loggear("Esperando cliente...");

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
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

	return socketCliente;
}

void identificar_cliente(char* mensaje, int socket_cliente) {
	char* mensajePlanificador =
			"My name is Planificador.c and I'm the fastest planifier alive...";
	char* mensajeESI = "A wild ESI has appeared!";
	char* mensajeInstancia = "It's ya boi, instancia!";

	if (strcmp(mensaje, mensajePlanificador) == 0) {
		loggear(mensajePlanificador);
		pthread_create(&hilo_planificador, NULL, atender_Planificador,
				(void*) socket_cliente);
		pthread_detach(hilo_planificador);
	} else if (strcmp(mensaje, mensajeESI) == 0) {
		loggear(mensajeESI);
		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);
		pthread_detach(hilo_ESI);
	} else if (strcmp(mensaje, mensajeInstancia) == 0) {
		loggear(mensajeInstancia);
		pthread_create(&hilo_instancia, NULL, atender_Instancia,
				(void*) socket_cliente);
		pthread_detach(hilo_instancia);
	} else {
		salir_con_error("Cliente desconocido, cerrando conexion.",
				socket_cliente);
	}

	return;
}

void* atender_ESI(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de ESI inicializado correctamente.");

	int status = 1;

	while (status) {
		status = chequear_solicitud(socket_cliente);
	}

	return NULL;
}

int chequear_solicitud(int socket_cliente) {
	aviso_ESI aviso_cliente;
	aviso_ESI aviso_servidor = { .aviso = 1 };

	int packageSize = sizeof(aviso_cliente.aviso) + sizeof(aviso_cliente.id);
	char *message = malloc(packageSize);
	char *package = malloc(packageSize);

	//Me gustaria mas que la aviso_servidor se envie como bool

	int res = recv(socket_cliente, (void*) package, packageSize, 0);

	if (res != 0) {
		loggear("Mensaje recibido del ESI.");
	} else {
		log_error(logger, "Fallo la peticion. Terminando ESI.");

		terminar_conexion(socket_cliente);
	}

	deserializar_aviso(&(aviso_cliente), &(package));

	if (aviso_cliente.aviso == 0) {
		loggear("Fin de ESI.");
		return 0;
	} else if (aviso_cliente.aviso != 1) {
		loggear("Peticion erronea.");
	}

	serializar_aviso(aviso_servidor, &message);

	send(socket_cliente, message, packageSize, 0);

	free(message);
	free(package);

	return 1;
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
