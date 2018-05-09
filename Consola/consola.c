/*
 * consola.c
 *
 *  Created on: 5 may. 2018
 *      Author: utnso
 */
#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGE_SIZE 1024
#include "consola.h"

int main() {
	float comando;
	printf("Bienvenido a la consola interactiva para el planificador \n");
	while (1) {
		listarOpciones();
		comando = recibirCodigo();
		interpretarYEjecutarCodigo(comando);
	}
}
/*

 struct addrinfo hints;
 struct addrinfo *serverInfo;
 memset(&hints, 0, sizeof(hints));
 hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
 hints.ai_socktype = SOCK_STREAM;	// Protocolo TCP
 getaddrinfo(IP, PUERTO, &hints, &serverInfo);
 int serverSocket;
 serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
 serverInfo->ai_protocol);
 int conexion = connect(serverSocket, serverInfo->ai_addr,
 serverInfo->ai_addrlen);
 if (conexion < 0) {
 salir_con_error("Fallo la conexion con el servidor.", serverSocket);
 }

 loggear("Conectó sin problemas");

 freeaddrinfo(serverInfo);

 char message[] = "A wild ESI has appeared!";
 char package[PACKAGE_SIZE];

 send(serverSocket, message, strlen(message) + 1, 0);

 loggear("Mensaje enviado.");
 int res = recv(serverSocket, (void*) package, PACKAGE_SIZE, 0);

 if (res != 0) {
 loggear("Mensaje recibido desde el servidor.");
 loggear(package);

 } else {
 salir_con_error("Fallo el envio de mensaje de parte del servidor.",	serverSocket);
 }

 loggear("Cerrando conexion con servidor y terminando.");

 close(serverSocket);
 return EXIT_SUCCESS;

 }*/
float recibirCodigo() {
	float code = 0;
	scanf("%f", &code);
	return code;
}

void interpretarYEjecutarCodigo(float comando) {
	int opcionElegida;
	float codigoSubsiguiente;
	opcionElegida = comando / 1;

	switch (opcionElegida) {
	case 1:
		pausarOContinuar();
		break;
	case 2:
		codigoSubsiguiente = comando - opcionElegida;
		bloquear(codigoSubsiguiente);
		break;
	case 3:
		codigoSubsiguiente = comando - opcionElegida;
		desbloquear(codigoSubsiguiente);
		break;
	case 4:
		listar();
		break;
	case 5:
		codigoSubsiguiente = comando - opcionElegida;
		kill(codigoSubsiguiente);
		break;
	case 6:
		codigoSubsiguiente = comando - opcionElegida;
		status(codigoSubsiguiente);
		break;
	case 7:
		deadlock();
		break;
	default:
		printf("Codigo incorrecto, recuerde que se introduce un float \n");
		break;
	};
}

void listarOpciones() {
	printf("1 : Pausar o reactivar la planificación \n");
	printf("2.<ESI ID> : Bloquea al ESI elegido \n");
	printf("3.<ESI ID> : Desbloquea al ESI elegido \n");
	printf("4.<Recurso> : Lista procesos esperando dicho recurso \n");
	printf("5.<ESI ID> : Mata al ESI elegido \n");
	printf("6.<ESI ID> : Brinda el estado del ESI elegido \n");
	printf("7 : Lista los ESI en deadlock \n");
	printf("Introduzca la opcion deseada \n");
}

void pausarOContinuar(void) {
	printf("Eligio pausar o continuar \n");
}
void bloquear(float codigo) {
	printf("Eligio bloquear el ESI \n");
}
void desbloquear(float codigo) {

}
void listar(void) {

}
void kill(float codigo) {

}
void status(float codigo) {

}
void deadlock(void) {

}

//Estos tres define van a cambiar, para poder cambiar ip y puerto en runtime (en caso de que esten ocupados) y para poder mandar datos de tamaño no fijo

void iniciar_log() {
	logger = log_create("ReDisTinto.log", "ESI", true, LOG_LEVEL_TRACE);
	loggear("ESI on duty.");
}

void loggear(char* mensaje) {
	log_trace(logger, mensaje);
}

void escucharRespuesta() {

	char package[PACKAGE_SIZE];
	struct addrinfo hints;
	struct addrinfo *server_info;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(IP, "6668", &hints, &server_info);
	// Tambien pruebo que se escuche el puerto 6668

	int listening_socket = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);
	//El socket que va a escuchar

	bind(listening_socket, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);
	listen(listening_socket, 1);
	if (recv(listening_socket, (void*) package, PACKAGE_SIZE, 0) < 0) {
		perror("No tuve respuesta");
	}
}
