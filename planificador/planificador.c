/*
 * ========================
 * ===== PLANIFICADOR =====
 * ========================
 */

#include "planificador.h"

#define PACKAGE_SIZE 1024
//Estos tres define van a cambiar, para poder cambiar ip y puerto en runtime (en caso de que esten ocupados) y para poder mandar datos de tamaño no fijo

int main() {
	iniciar_log("Planificador", "Nace el planificador...");
	char mensaje[] =
			"My name is Planificador.c and I'm the fastest planifier alive...";

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,	mensaje);
	int listening_socket = levantar_servidor(PUERTO_PLANIFICADOR);
	int socketCliente;

	while (1) {
		manejar_cliente(listening_socket, socketCliente, mensaje);
	}

	loggear("Cerrando sesion...");

	close(listening_socket);
	close(socketCliente);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}

