/*
 * ========================
 * ===== PLANIFICADOR =====
 * ========================
 */

#include "planificador.h"

#define PACKAGE_SIZE 1024
//Estos tres define van a cambiar, para poder cambiar ip y puerto en runtime (en caso de que esten ocupados) y para poder mandar datos de tamaño no fijo

int i;

ESI test;

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

	cerrar();

	return EXIT_SUCCESS;
}

void iniciar(void) {
	/*pthread_create(hiloDeConsola, NULL, consola, NULL);
	 pthread_detach(hiloDeConsola);*/
	iniciar_log("Planificador", "Nace el planificador...");

	algoritmo_planificacion.desalojo = false;
	algoritmo_planificacion.tipo = FIFO;

	//executing_ESI = malloc(sizeof(executing_ESI));
	executing_ESI.id = -1;

	ESI_id = 1;

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
	pthread_t hilo_ESI;

	if (strcmp(mensaje, mensajePlanificador) == 0) {
		loggear(mensajePlanificador);
		loggear("Wait, what the fuck?");
	} else if (strcmp(mensaje, mensajeESI) == 0) {
		loggear(mensajeESI);

		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);

		//loggear(mensajeESILista);

		pthread_detach(hilo_ESI);
	} else if (strcmp(mensaje, mensajeInstancia) == 0) {
		loggear(mensajeInstancia);
	} else {
		cerrar();
		salir_con_error("Cliente desconocido, cerrando conexion.",
				socket_cliente);
	}

	return;
}

void* atender_ESI(void* buffer) {
	int socket_ESI = (int) buffer;

	aviso_ESI aviso;

	int packageSize = sizeof(aviso);
	char* package = malloc(packageSize);

	loggear("Hilo de ESI inicializado correctamente.");

	ESI esi = { .socket = socket_ESI, .rafaga_estimada =
	ESTIMACION_INICIAL };

	int this_id = asignar_ID(esi);

	esi.id = this_id;

	while (1) {
		recv(socket_ESI, package, packageSize, 0);

		log_trace(logger, "Mensaje recibidio del ESI numero: %i", this_id);

		deserializar_aviso(&(aviso), &(package));

		if (aviso.aviso == 0) {
			loggear("ESI terminado.");

			//procesar_cierre(socket_ESI);

			list_remove(ESIs, this_id);

			break;
		}

		else if (aviso.aviso == 1) {
			esi.tiempo_arribo = tiempo;

			agregar_ESI(&new_ESIs, esi);

			loggear("ESI listo para ejecutar añadido a la cola.");

		}

		else {
			cerrar();
			loggear("El ESI se volvió loco. Terminando.");
			kill_ESI(esi);
		}

		planificar();
	}

	return NULL;
}

void kill_ESI(ESI esi) {
	int socket_ESI = esi.socket;

	aviso_ESI aviso = { .aviso = -1, .id = esi.id };

	int packageSize = sizeof(aviso_ESI);
	char* package = malloc(packageSize);

	serializar_aviso(aviso, &package);

	int envio = send(socket_ESI, package, packageSize, 0);

	if (envio < 0) {
		loggear("Fallo la terminación. Intentando de vuelta.");
		kill_ESI(esi);
	}

	log_trace(logger, "ESI número %i has fainted!", esi.id);
}

int asignar_ID(ESI esi) {
	int socket_ESI = esi.socket;
	aviso_ESI aviso = { .aviso = 1, .id = ESI_id };

	ESI_id++;

	int packageSize = sizeof(aviso.aviso) + sizeof(aviso.id);
	char* package = malloc(packageSize);

	serializar_aviso(aviso, &package);

	int envio = send(socket_ESI, package, packageSize, 0);

	if (envio < 0) {
		loggear("Fallo el envio de identificacion. Terminando ESI.");
		kill_ESI(esi);
	}

	free(package);

	return aviso.id;
}

void procesar_cierre(int socket_ESI) {
	loggear("Esperando id de ejecucion del planificador.");

	package_pedido id;

	int packageSize = sizeof(id.pedido);
	char *package = malloc(packageSize);

	int res = recv(socket_ESI, (void*) package, packageSize, 0);

	if (res != 0) {
		loggear("Solicitud confirmada.");
	} else {
		cerrar();
		salir_con_error("Fallo la solicitud.", socket_ESI);
	}

	deserializar_pedido(&(id), &(package));

	int id_as_int = (int) id.pedido;

	list_remove(ESIs, id_as_int);

}

void planificar(void) {
	if (executing_ESI.id == -1) {
		executing_ESI = first(new_ESIs);
		//*executing_ESI = test;

		//list_remove(ESIs, 0);

		log_trace(logger, "ESI número %i elegido.", executing_ESI.id);

		ejecutar(executing_ESI);

		return;
	}

	if (algoritmo_planificacion.desalojo) {
		desalojar();
	}

	switch (algoritmo_planificacion.tipo) {
	case FIFO:
		executing_ESI = first(new_ESIs);
		//*executing_ESI = test;
		break;
	case SJF:
		executing_ESI = shortest(new_ESIs);
		break;
	case HRRN:
		executing_ESI = highest_RR(new_ESIs);
		break;
	default:
		loggear("FALLO EN EL ALGORITMO.");
		break;
	}

	log_trace(logger, "ESI número %i elegido.", executing_ESI.id);

	ejecutar(executing_ESI);

}

void desalojar(void) {
	//list_add(ESIs, (void*) &executing_ESI);
	executing_ESI = esi_vacio;
}

t_esi_node* crear_nodo(ESI esi) {
	t_esi_node* nodo = (t_esi_node*) malloc(sizeof(t_esi_node));
	nodo->esi = esi;
	nodo->sgte = NULL;

	return nodo;
}

void agregar_ESI(t_esi_list* lista, ESI esi) {
	t_esi_node* nodo = crear_nodo(esi);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_esi_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}

	return;
}

void destruir_nodo(t_esi_node* nodo) {
	free(nodo);
}

ESI first(t_esi_list lista) {
	ESI esi = lista.head->esi;

	return esi;

}

ESI shortest(t_esi_list lista) {
	t_esi_node* puntero = lista.head;

	ESI esi = first(lista);

	while (puntero->sgte != NULL) {
		if (es_mas_corto(esi, puntero->esi)) {
			esi = puntero->esi;
		}

		puntero = puntero->sgte;
	}

	return esi;
}

ESI highest_RR(t_esi_list lista) {
	t_esi_node* puntero = lista.head;

	ESI esi = first(lista);

	while (puntero->sgte != NULL) {
		if (tiene_mas_RR(esi, puntero->esi)) {
			esi = puntero->esi;
		}

		puntero = puntero->sgte;
	}

	return esi;
}

bool es_mas_corto(ESI primer_ESI, ESI segundo_ESI) {
	return estimated_time(segundo_ESI) < estimated_time(primer_ESI);
}

bool tiene_mas_RR(ESI primer_ESI, ESI segundo_ESI) {
	int primer_RR = 1 + wait_time(primer_ESI) / estimated_time(primer_ESI);
	int segundo_RR = 1 + wait_time(segundo_ESI) / estimated_time(segundo_ESI);

	return segundo_RR > primer_RR;
}

int wait_time(ESI esi) {
	return tiempo - esi.tiempo_arribo;
}

int estimated_time(ESI esi) {
	return esi.rafaga_real * (ALFA / 100)
			+ esi.rafaga_estimada * ((100 - ALFA) / 100);
}

void cerrar(void) {
	cerrar_listas();
}

void ejecutar(ESI esi_a_ejecutar) {
	int socket_ESI = esi_a_ejecutar.socket;

	loggear("Enviando orden de ejecucion.");
	aviso_ESI orden_ejecucion = { .aviso = 2, .id = esi_a_ejecutar.id };

	int packageSize = sizeof(orden_ejecucion.aviso)
			+ sizeof(orden_ejecucion.id);
	char* message = malloc(packageSize);

	serializar_aviso(orden_ejecucion, &message);

	int envio = send(socket_ESI, message, packageSize, 0);

	if (envio < 0) {
		cerrar();
		log_error(logger, "Fallo el envio. Terminando ESI.");

		kill_ESI(esi_a_ejecutar);

		free(message);
		return;
	}

	loggear("Orden enviada.");

	list_remove(ESIs, esi_a_ejecutar.id);

	free(message);
}

void deserializar_esi(void* esi_copiado, ESI* esi_receptor) {
	int offset = 0;

	loggear("a");

	memcpy(&esi_receptor->id, esi_copiado, sizeof(esi_receptor->id));

	loggear("b");

	offset = sizeof(esi_receptor->id);

	memcpy(&esi_receptor->socket, esi_copiado + offset,
			sizeof(esi_receptor->socket));
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
