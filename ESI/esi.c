/*
 * esi.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGE_SIZE 1024
//Estos tres define van a cambiar, para poder cambiar ip y puerto en runtime (en caso de que esten ocupados) y para poder mandar datos de tamaño no fijo
int main(){

	logger = log_create("ReDisTinto.log", "ESI", true, LOG_LEVEL_TRACE); //La configuración del logger haciendo que logee TODO

	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);
		/* Ya se quien y a donde me tengo que conectar... ¿Y ahora?
		 *	Tengo que encontrar una forma por la que conectarme al server... Ya se! Un socket!
		 * 	Obtiene un socket (un file descriptor -todo en linux es un archivo-), utilizando la estructura serverInfo que generamos antes. */
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
		/* Perfecto, ya tengo el medio para conectarme (el archivo), y ya se lo pedi al sistema.
		   Ahora me conecto */

	log_trace(logger, "Nace un nuevo ESI"); //No se si sea justamente un trace, pero por ahora me cago

	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);

	log_trace(logger, "Conectó sin problemas");

	freeaddrinfo(serverInfo);

	int quieroEnviar = 1;
	char message[PACKAGE_SIZE];

	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	while(quieroEnviar){
		fgets(message, PACKAGE_SIZE, stdin);
		if (!strcmp(message,"exit\n")) quieroEnviar = 0;
		if (quieroEnviar) {send(serverSocket, message, strlen(message) + 1, 0);
					 log_trace(logger, "Mandamos un mensaje al servidor");
		}
	}

	close(serverSocket);
	return 0;

}
