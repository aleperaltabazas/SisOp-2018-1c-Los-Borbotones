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

	//Por ahora intento hacer una lista con todos los hilos de ESIs sin discriminarlos para simplificar
	ESIs = list_create();
	ESIs_bloqueados = list_create();
	ESIs_en_ejecucion = list_create();
	ESIs_listos = list_create();
	ESIs_finalizados = list_create();

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

	//Borro todos los datos de las listas...
	//Habria que liberar la memoria por cada elemento que fue agregado o con el clean ya alcanza?
	list_clean(ESIs);
	list_clean(ESIs_bloqueados);
	list_clean(ESIs_en_ejecucion);
	list_clean(ESIs_listos);
	list_clean(ESIs_finalizados);

	//Libero las cabezas de las listas...
	free(ESIs);
	free(ESIs_bloqueados);
	free(ESIs_en_ejecucion);
	free(ESIs_listos);
	free(ESIs_bloqueados);

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

	loggear("Archivo abierto correctamente.");

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
	char* mensajeESI_lista = "Hilo de ESI agregado a la lista de ESIs";
	char* mensajeInstancia = "It's ya boi, instancia!";

	if (strcmp(mensaje, mensajePlanificador) == 0) {
		loggear(mensajePlanificador);
		loggear("Wait, what the fuck?");
	} else if (strcmp(mensaje, mensajeESI) == 0) {
		loggear(mensajeESI);

		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);

		//Esto me agrega el hilo de ESI a la lista y me devuelve su posicion, la cual podria usarse como id
		ESI_id = list_add(ESIs, (void*) hilo_ESI);
		loggear(mensajeESI_lista);

		//Creo que el detach no se haria de inmediato, en base al algoritmo se va a hacer detach a un ESI determinado
		//Sino siempre que llegue un ESI mientras que se este ejecutando otro va a tomar prioridad el que llega
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
	int packageSize = strlen(line) + 1;
	char *message = malloc(packageSize);

	package_line linea = {
			.line = line
	};

	serializar_linea(linea, &message);

	int envio = send(socket_cliente, message, packageSize, 0);

	if (envio < 0) {
		salir_con_error("Fallo el envio de parseo.", socket_cliente);
	}

	loggear("Linea a parsear enviada.");

	return;
}
