/*
 * ========================
 * ===== PLANIFICADOR =====
 * ========================
 */

#include "planificador.h"

#define PACKAGE_SIZE 1024
//Estos tres define van a cambiar, para poder cambiar ip y puerto en runtime (en caso de que esten ocupados) y para poder mandar datos de tamaño no fijo

int main(int argc, char** argv) {
	iniciar();

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			mensajePlanificador);
	int listening_socket = levantar_servidor(PUERTO_PLANIFICADOR);
	int socketCliente;

	while (1) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				mensajePlanificador);
	}

	loggear("Cerrando sesion...");

	close(listening_socket);
	close(socketCliente);
	close(socket_coordinador);
	return EXIT_SUCCESS;
}

void iniciar() {
	/*pthread_create(hiloDeConsola, NULL, consola, NULL);
	 pthread_detach(hiloDeConsola);*/
	iniciar_log("Planificador", "Nace el planificador...");

	//Por ahora intento hacer una lista con todos los hilos de ESIs sin discriminarlos para simplificar
	ESIs = list_create();
	ESIs_bloqueados = list_create();
	ESIs_en_ejecucion = list_create();
	ESIs_listos = list_create();
	ESIs_finalizados = list_create();
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

	if (strcmp(mensaje, mensajePlanificador) == 0) {
		loggear(mensajePlanificador);
		loggear("Wait, what the fuck?");
	} else if (strcmp(mensaje, mensajeESI) == 0) {
		loggear(mensajeESI);

		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);

		//Esto me agrega el hilo de ESI a la lista y me devuelve su posicion, la cual podria usarse como id
		ESI_id = list_add(ESIs, (void*) hilo_ESI);
		loggear(mensajeESILista);

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

void* atender_ESI(void* sockfd) {
	int socket_ESI = (int) sockfd;

	loggear("Hilo de ESI inicializado correctamente.");

	loggear("Enviando orden de parseo.");

	while(1){
		che_parsea(socket_ESI);
		sleep(10);
	}

	return NULL;
}

void che_parsea(int socket_cliente) {
	package_pedido pedido_parseo = { .pedido = 1 };

	int packageSize = sizeof(pedido_parseo.pedido);
	char* message = malloc(packageSize);

	serializar_pedido(pedido_parseo, &message);

	int envio = send(socket_cliente, message, packageSize, 0);

	if (envio < 0) {
		salir_con_error("Fallo el envio de parseo.", socket_cliente);
	}

	loggear("Orden de parseo enviada.");

	return;
}

void cerrar_listas() {
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

	cerrar_listas();
}

/*	=====================
 *	===== CONSOLITA =====
 *	=====================
 */

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
	case 0:
		pthread_cancel(pthread_self()); //Usar esto lo hace no portable, preguntarle a Lean
		break;
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
		printf(
				"Codigo incorrecto, recuerde que se introduce un codigo de tipo float \n");
		break;
	};
}
void listarOpciones() {
	printf("0 : Cancelar consola \n");
	printf("1 : Pausar o reactivar la planificación \n");
	printf("2.<ESI ID> : Bloquea al ESI elegido \n");
	printf("3.<ESI ID> : Desbloquea al ESI elegido \n");
	printf("4.<Recurso> : Lista procesos esperando dicho recurso \n");
	printf("5.<ESI ID> : Mata al ESI elegido \n");
	printf("6.<ESI ID> : Brinda el estado del ESI elegido \n");
	printf("7 : Lista los ESI en deadlock \n");
	printf("Introduzca la opcion deseada \n");
}
void* consola(void) {
	float comando;
	printf("Bienvenido a la consola interactiva para el planificador \n");
	while (1) {
		listarOpciones();
		comando = recibirCodigo();
		interpretarYEjecutarCodigo(comando);
	}
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
