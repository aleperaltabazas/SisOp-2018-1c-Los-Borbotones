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

	FILE* archivo_a_parsear = levantar_archivo(argv);

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,	mensaje);
	int listening_socket = levantar_servidor(PUERTO_PLANIFICADOR);
	int socketCliente;

	while (1) {
		socketCliente = manejar_cliente(listening_socket, socketCliente, mensaje);
		che_parsea(socketCliente, archivo_a_parsear);
	}

	loggear("Cerrando sesion...");

	close(listening_socket);
	close(socketCliente);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}

FILE* levantar_archivo(char** archivo){
	FILE* fp = fopen(archivo[1], "r");

	if (fp == NULL){
		error_de_archivo("Error en abrir el archivo.", EXIT_FAILURE);
	}

	return fp;
}

void error_de_archivo(char* mensaje_de_error, int retorno){
	log_error(logger, mensaje_de_error);
	exit_gracefully(retorno);
}

void che_parsea(int socket_cliente, FILE* archivo_a_parsear){

}
