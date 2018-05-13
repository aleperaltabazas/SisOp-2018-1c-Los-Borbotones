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

	char mensaje[] = "Coordinador: taringuero profesional.";

	int listening_socket = levantar_servidor(PUERTO_COORDINADOR);
	int socketCliente;

	while (1) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				mensaje);
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

	uint32_t pedido;
	uint32_t respuesta = 1;

	//Me gustaria mas que la respuesta se envie como bool

	int res = recv(socket_cliente, (void*) pedido, sizeof(int), 0);

	if (res != 0){
		loggear("Peticion de parseo recibida.");
	}
	else{
		salir_con_error("Fallo la peticion de parseo.", socket_cliente);
	}

	if(pedido != 1){
		loggear("Peticion erronea.");
	}

	bool orden_de_parseo = puede_parsear();

	if(!orden_de_parseo){
		respuesta = 0;
	}

	send(socket_cliente, &respuesta, sizeof(uint32_t), 0);

	return NULL;
}

bool puede_parsear(){
	return true;

	//Para que una funcion que devuelva true? Porque por ahora queremos testear
	//Que el envio de mensajes de parseo y funcione
	//Mas adelante tendremos que ver si lo deja parsear o se bloquea
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
