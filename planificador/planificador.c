/*
 * ========================
 * ===== PLANIFICADOR =====
 * ========================
 */

#include "planificador.h"

#define PACKAGE_SIZE 1024
//Estos tres define van a cambiar, para poder cambiar ip y puerto en runtime (en caso de que esten ocupados) y para poder mandar datos de tamaño no fijo

int main(int argc, char** argv) {
	iniciar_log("Planificador", "Nace el planificador...");
	char mensaje[] =
			"My name is Planificador.c and I'm the fastest planifier alive...";

	//FILE* fp = levantar_archivo(algunArchivo);

	archivo_de_parseo = levantar_archivo("script.esi");

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			mensaje);
	int listening_socket = levantar_servidor(PUERTO_PLANIFICADOR);
	int socketCliente;

	//parsed a_parsear = { .socket_cliente = socketCliente, .archivo_a_parsear = fp };

	while (1) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				mensaje);
	}

	loggear("Cerrando sesion...");

	close(listening_socket);
	close(socketCliente);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}

FILE* levantar_archivo(char* archivo) {
	FILE* fp = fopen(archivo, "r");

	if (fp == NULL) {
		error_de_archivo("Error en abrir el archivo.", EXIT_FAILURE);
	}

	return fp;
}

int manejar_cliente(int listening_socket, int socket_cliente, char* mensaje) {

	loggear("Esperando cliente...");

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	socket_cliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	loggear("Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	char package[PACKAGE_SIZE];

	int res = recv(socket_cliente, (void*) package, PACKAGE_SIZE, 0);

	if (res <= 0) {
		loggear("Fallo la conexion con el cliente.");
	}

	loggear("Mensaje recibido exitosamente. Identificando cliente...");
	identificar_cliente((char*) package, socket_cliente);

	loggear("Enviando mensaje al cliente.");

	send(socket_cliente, mensaje, strlen(mensaje) + 1, 0);

	loggear("Mensaje enviado. Cerrando sesion con el cliente actual.");

	return socket_cliente;
}

void identificar_cliente(char* mensaje, int socket_cliente) {
	char* mensajePlanificador =
			"My name is Planificador.c and I'm the fastest planifier alive...";
	char* mensajeESI = "A wild ESI has appeared!";
	char* mensajeInstancia = "It's ya boi, instancia!";

	if (strcmp(mensaje, mensajePlanificador) == 0) {
		loggear(mensajePlanificador);
		loggear("Wait, what the fuck?");
	} else if (strcmp(mensaje, mensajeESI) == 0) {
		loggear(mensajeESI);
		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);
		pthread_detach(hilo_ESI);
	} else if (strcmp(mensaje, mensajeInstancia) == 0) {
		loggear(mensajeInstancia);
	} else {
		salir_con_error("Cliente desconocido, cerrando conexion.",
				socket_cliente);
	}

	return;
}

char* siguiente_linea(FILE* archivo) {
	char* line = NULL;
	size_t len = 40;
	ssize_t read;

	read = getline(&line, &len, archivo);

	if (read == (-1)) {
		loggear("No hay mas lineas para parsear.");
		free(line);
		exit_gracefully(EXIT_SUCCESS);
	}

	return line;
}

void* atender_ESI(void* sockfd) {
	int socket_ESI = (int) sockfd;

	loggear("Hilo de ESI inicializado correctamente.");

	char* line = siguiente_linea(archivo_de_parseo);

	loggear("Enviando orden de parseo.");

	che_parsea(socket_ESI, line);

	fclose(archivo_de_parseo);

	return NULL;
}

void error_de_archivo(char* mensaje_de_error, int retorno) {
	log_error(logger, mensaje_de_error);
	exit_gracefully(retorno);
}

void che_parsea(int socket_cliente, char* line) {
	int envio = send(socket_cliente, line, strlen(line) + 1, 0);

	if (envio < 0) {
		salir_con_error("Fallo el envio de parseo.", socket_cliente);
	}

	loggear("Linea a parsear enviada.");

	return;
}
