/*
 * planificador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */
#include "planificador.h"

#define PUERTO "8001"
#define PUERTOALCOORDINADOR "8000"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(){
	logger = log_create("ReDisTinto.log", "Planificador", true, LOG_LEVEL_TRACE); //Le ponemos el mismo nombre? Es importante eso? Podríamos tener un solo mega logger con todos los proyectos?

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	//Propongo que lo primero que haga el planificador sea conectarse con el coordinador, luego que se quede escuchando
	getaddrinfo(NULL, PUERTO, &hints, &serverInfo);
	log_trace(logger, "Conectando con el coordinador...");
	int serverSocketCoordinador;
		serverSocketCoordinador = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	int conexionCoordinador = connect(serverSocketCoordinador, serverInfo->ai_addr,
				serverInfo->ai_addrlen);

	if (conexionCoordinador < 0) {
			salir_con_error("Falló la conexion con el coordinador.", serverSocketCoordinador);
	}

	freeaddrinfo(serverInfo);

	log_trace(logger, "Conectado al coordinador");

	char message[] = "El planificador ha llegado";
	send(serverSocketCoordinador, message, strlen(message) + 1, 0);

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE
/*Nota: Lean nos dijo que mantengamos esto así por ahora.
Nota de la nota: Luego de meditarlo, considero que escapa de lo que es operativos en si y como son conexiones pasan
a ser parte de redes (o eso me gustaría creer)
*/
	log_trace(logger, "Nace el planificador");
	int listenningSocket; //Más adelante creo que vamos a tener que meter esto dentro de un while, con un thread propio para estar "siempre" listo para recibir nuevas conexiones y "reenviarlas"
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	listen(listenningSocket, BACKLOG); //En esta parte tendriamos que tener en cuenta un timeout (no recuerdo si era acá que Lean nos dijo que lo tiene incluído)
	log_trace(logger, "Esperando ");
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
//Nota: Tenemos n sockets address, y vamos a tener que ver como podemos tratar con ellos, se me ocurre threads pero también existe select
	char package[PACKAGESIZE];
	int status = 1;		// Estructura que manjea el status de los recieve.

	printf("Cliente conectado. Esperando mensajes:\n");
	log_trace(logger, "Hable mas fuerte ESI, tengo una toalla"); //Creo que el log es mejor pero en el ejemplito usan printf

	while (status != 0) {
		status = recv(socketCliente, (void*) package, PACKAGESIZE, 0);
		if (status != 0) printf("%s", package);
		send(socketCliente, (void*) package, PACKAGESIZE, 0);
	}
	close(socketCliente);
	close(listenningSocket);
	return 0;
	//pthread_create (); esta funcion la vemos despues con hilos
}

