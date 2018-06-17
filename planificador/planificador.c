/*
 * ========================
 * ===== PLANIFICADOR =====
 * ========================
 */

#include "planificador.h"

int i;

ESI test;

int main(int argc, char** argv) {
	iniciar();

	int listening_socket = levantar_servidor(PUERTO_PLANIFICADOR);
	int socketCliente;

	while (seguir_ejecucion) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				mensajePlanificador);
	}

	loggear("Terminando proceso...");

	avisar_cierre(socket_coordinador);

	close(listening_socket);
	close(socketCliente);
	close(socket_coordinador);

	return EXIT_SUCCESS;
}

void iniciar(void) {
	iniciar_log("Planificador", "Nace el planificador...");

	cargar_configuracion();
	iniciar_semaforos();
	iniciar_hilos();

}

void iniciar_hilos(void) {
	pthread_attr_t console_attr;
	pthread_attr_init(&console_attr);

	pthread_attr_t coordi_attr;
	pthread_attr_init(&coordi_attr);

	pthread_create(&hiloDeConsola, &console_attr, consola, NULL);
	pthread_detach(hiloDeConsola);

	pthread_create(&hilo_coordinador, &coordi_attr, atender_coordinador, NULL);
}

void cargar_configuracion(void) {

	executing_ESI = esi_vacio;

	ESI_id = 1;

	ESIs_size = 0;
	tiempo = 0;

	t_config* config = config_create("planificador.config");

	PUERTO_COORDINADOR = config_get_string_value(config, "PUERTO_COORDINADOR");
	log_info(logger, "Puerto Coordinador: %s", PUERTO_COORDINADOR);

	PUERTO_PLANIFICADOR = config_get_string_value(config,
			"PUERTO_PLANIFICADOR");
	log_info(logger, "Puerto Planificador: %s", PUERTO_PLANIFICADOR);

	IP_COORDINADOR = config_get_string_value(config, "IP_COORDINADOR");
	log_info(logger, "IP Coordinador: %s", IP_COORDINADOR);

	IP_PLANIFICADOR = config_get_string_value(config, "IP_PLANIFICADOR");
	log_info(logger, "IP Planificador: %s", IP_COORDINADOR);

	char* algoritmo = config_get_string_value(config,
			"ALGORITMO_PLANIFICACION");
	ALGORITMO_PLANIFICACION = dame_algoritmo(algoritmo);
	log_info(logger, "Algoritmo: %s", algoritmo);

	ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");
	log_info(logger, "Estimación inicial: %i", ESTIMACION_INICIAL);

	ALFA = config_get_int_value(config, "ALFA");
	log_info(logger, "Alfa: %i", ALFA);

	loggear("Configuración cargada.");
}

algoritmo_planificacion dame_algoritmo(char* algoritmo_src) {
	algoritmo_planificacion algoritmo_ret;

	if (strcmp(algoritmo_src, "FIFO") == 0) {
		algoritmo_ret.tipo = FIFO;
		algoritmo_ret.desalojo = false;
	}

	else if (strcmp(algoritmo_src, "SJF-SD") == 0) {
		algoritmo_ret.tipo = SJF;
		algoritmo_ret.desalojo = false;
	}

	else if (strcmp(algoritmo_src, "SJF-CD") == 0) {
		algoritmo_ret.tipo = SJF;
		algoritmo_ret.desalojo = true;
	}

	else if (strcmp(algoritmo_src, "HRRN") == 0) {
		algoritmo_ret.tipo = HRRN;
		algoritmo_ret.desalojo = false;
	}

	return algoritmo_ret;
}

void* atender_coordinador(void* nada) {
	socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			mensajePlanificador);

	int status = 1;

	while (status) {
		status = recibir_respuesta(socket_coordinador);
	}

	return NULL;
}

int recibir_respuesta(int server_socket) {
	return 1;
}

void iniciar_semaforos() {
	pthread_mutex_init(&mutex_consola_planificacion, NULL);
	pthread_mutex_init(&sem_ESIs_size, NULL);
	pthread_mutex_init(&sem_ID, NULL);
	pthread_mutex_init(&sem_clock, NULL);
	pthread_mutex_init(&sem_planificacion, NULL);
	pthread_mutex_init(&sem_ejecucion, NULL);
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

	ESI esi = { .socket = socket_ESI, .rafaga_estimada = ESTIMACION_INICIAL,
			.rafaga_real = 0 };

	int this_id = asignar_ID(esi);

	esi.id = this_id;

	while (1) {
		recv(socket_ESI, package, packageSize, 0);

		log_trace(logger, "Mensaje recibidio del ESI numero: %i", this_id);

		deserializar_aviso(&(aviso), &(package));

		if (aviso.aviso == 0) {
			loggear(
					"ESI terminado. Moviendo a la cola de terminados y eliminando de la cola de listos.");

			agregar_ESI(&finished_ESIs, esi);

			pthread_mutex_lock(&sem_ESIs_size);
			eliminar_ESI(&new_ESIs, esi);

			ESIs_size--;
			pthread_mutex_unlock(&sem_ESIs_size);

			loggear("Agregado correctamente a la cola de terminados.");

			loggear("Eliminado correctamente de la cola de listos.");

			desalojar();

			pthread_mutex_unlock(&sem_ejecucion);

			break;
		}

		else if (aviso.aviso == 1) {
			pthread_mutex_lock(&sem_clock);
			esi.tiempo_arribo = tiempo;
			pthread_mutex_unlock(&sem_clock);

			pthread_mutex_lock(&sem_ESIs_size);
			agregar_ESI(&new_ESIs, esi);

			ESIs_size++;
			pthread_mutex_unlock(&sem_ESIs_size);

			log_trace(logger,
					"ESI número %i listo para ejecutar añadido a la cola.",
					this_id);

			if (ALGORITMO_PLANIFICACION.desalojo) {
				desalojar();
			}

		}

		else if (aviso.aviso == 10) {
			pthread_mutex_lock(&sem_clock);
			tiempo++;
			pthread_mutex_unlock(&sem_clock);

			pthread_mutex_lock(&sem_ejecutando);
			ejecutando = false;
			pthread_mutex_unlock(&sem_ejecutando);

			esi.rafaga_real++;

			pthread_mutex_lock(&sem_ESIs_size);
			agregar_ESI(&new_ESIs, esi);

			ESIs_size++;
			pthread_mutex_unlock(&sem_ESIs_size);

			log_trace(logger, "ESI número %i ejecutó correctamente.", this_id);

			pthread_mutex_unlock(&sem_ejecucion);
		}

		else {

			loggear("El ESI se volvió loco. Terminando.");
			kill_ESI(esi);
		}

		planificar();
	}

	planificar();

	log_trace(logger, "Hilo de ESI número %i terminado.", this_id);

	return NULL;
}

void desalojar(void) {
	if (executing_ESI.id != esi_vacio.id) {
		executing_ESI = esi_vacio;

		loggear("ESI desalojado.");
	}
}

bool no_hay_ESI() {
	return executing_ESI.id == esi_vacio.id;
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

	pthread_mutex_lock(&sem_ID);
	aviso_ESI aviso = { .aviso = 1, .id = ESI_id };

	ESI_id++;
	pthread_mutex_unlock(&sem_ID);

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

void planificar(void) {
	if (consola_planificacion && ESIs_size > 0 && !ejecutando) {
		pthread_mutex_lock(&sem_ejecucion);

		ESI next_esi = dame_proximo_ESI();

		log_trace(logger, "ESI número %i elegido.", next_esi.id);

		ejecutar(next_esi);
	}
}

ESI dame_proximo_ESI() {
	ESI next_esi = executing_ESI;

	if (no_hay_ESI()) {
		switch (ALGORITMO_PLANIFICACION.tipo) {
		case FIFO:
			next_esi = first(new_ESIs);
			break;
		case SJF:
			next_esi = shortest(new_ESIs);
			break;
		case HRRN:
			next_esi = highest_RR(new_ESIs);
			break;
		default:
			loggear("FALLO EN EL ALGORITMO.");
			break;
		}
	}

	return next_esi;
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

void eliminar_ESI(t_esi_list* lista, ESI esi) {
	if (lista->head != NULL) {
		ESI head = first(*lista);
		if (esi.id == head.id) {
			t_esi_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo(eliminado);
		} else {
			t_esi_node* puntero = lista->head;

			while (puntero->esi.id != esi.id) {
				puntero = puntero->sgte;
			}

			t_esi_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo(eliminado);
		}
	}
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
			+ esi.rafaga_estimada * (1 - (ALFA/100));
}

void ejecutar(ESI esi_a_ejecutar) {

	pthread_mutex_lock(&sem_ejecutando);
	ejecutando = true;
	pthread_mutex_unlock(&sem_ejecutando);

	int socket_ESI = esi_a_ejecutar.socket;
	executing_ESI = esi_a_ejecutar;

	loggear("Enviando orden de ejecucion.");
	aviso_ESI orden_ejecucion = { .aviso = 2, .id = esi_a_ejecutar.id };

	int packageSize = sizeof(orden_ejecucion.aviso)
			+ sizeof(orden_ejecucion.id);
	char* message = malloc(packageSize);

	serializar_aviso(orden_ejecucion, &message);

	int envio = send(socket_ESI, message, packageSize, 0);

	if (envio < 0) {

		log_error(logger, "Fallo el envio. Terminando ESI.");

		kill_ESI(esi_a_ejecutar);

		free(message);
		return;
	}

	loggear("Orden enviada.");

	pthread_mutex_lock(&sem_ESIs_size);
	eliminar_ESI(&new_ESIs, esi_a_ejecutar);

	ESIs_size--;
	pthread_mutex_unlock(&sem_ESIs_size);

	loggear("ESI eliminado de la cola de la listos.");

	free(message);

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
	opcionElegida = (int) comando;
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
	case 8:
		terminar();
		break;
	case 9:
		mostrame_clock();
		break;
	case 10:
		display_console();
		break;
	case 11:
		dame_datos();
		break;
	case 12:
		bloquear_clave();
		break;
	case 13:
		desbloquear_clave();
		break;
	default:
		printf(
				"Codigo incorrecto, recuerde que se introduce un codigo de tipo float \n");
		break;
	};
}

void desbloquear_clave() {
	printf("Ingrese la clave a desbloquear: ");
	char clave[40] = "futbol:messi";
	scanf(" %s", clave);

	avisar_desbloqueo(socket_coordinador, clave);

	printf("Clave desbloqueada \n");

}

void avisar_desbloqueo(int server_socket, char* clave) {
	int packageSize = sizeof(aviso_ESI);
	char* message = malloc(packageSize);
	char* res_message = malloc(packageSize);

	serializar_aviso(aviso_desbloqueo, &message);

	int envio = send(server_socket, message, packageSize, 0);

	if (envio < 0) {
		salir_con_error("Falló el aviso de bloqueo de clave.", server_socket);
	}

	int res = recv(server_socket, res_message, packageSize, 0);

	if (res <= 0) {
		salir_con_error("No se pudo recibir bien la confirmación.",
				server_socket);
	}

	aviso_ESI aviso_coordi;

	deserializar_aviso(&(aviso_coordi), &(res_message));

	if (aviso_coordi.aviso != 25) {
		salir_con_error("Respuesta errónea.", server_socket);
	}

	uint32_t size = (uint32_t) strlen(clave) + 1;

	package_int size_package = { .packed = size };

	packageSize = sizeof(package_int);
	char* package_de_size = malloc(packageSize);

	serializar_packed(size_package, &package_de_size);

	send(server_socket, package_de_size, packageSize, 0);

	packageSize = size;

	sleep(2);

	send(server_socket, clave, packageSize, 0);

	packageSize = sizeof(package_int);
	char* response_package = malloc(packageSize);
	package_int response;

	res = recv(server_socket, response_package, packageSize, 0);

	if (res <= 0) {
		salir_con_error("Falló la respuesta del coordinador.", server_socket);
	}

	deserializar_packed(&(response), &(response_package));

	if (response.packed != 28) {
		salir_con_error("Falló el desbloqueo de la clave", server_socket);
	}

	free(message);
	free(res_message);
	free(package_de_size);
	free(response_package);

	log_trace(logger, "La clave %s fue desbloqueada.", clave);
}

void bloquear_clave() {
	printf("Ingrese la clave a bloquear: ");
	char clave[40] = "futbol:messi";
	scanf(" %s", clave);

	avisar_bloqueo(socket_coordinador, clave);

	printf("Clave bloqueada \n");
}

void avisar_bloqueo(int server_socket, char* clave) {
	int packageSize = sizeof(aviso_ESI);
	char* message = malloc(packageSize);
	char* res_message = malloc(packageSize);

	serializar_aviso(aviso_bloqueo, &message);

	int envio = send(server_socket, message, packageSize, 0);

	if (envio < 0) {
		salir_con_error("Falló el aviso de bloqueo de clave.", server_socket);
	}

	int res = recv(server_socket, res_message, packageSize, 0);

	if (res <= 0) {
		salir_con_error("No se pudo recibir bien la confirmación.",
				server_socket);
	}

	aviso_ESI aviso_coordi;

	deserializar_aviso(&(aviso_coordi), &(res_message));

	if (aviso_coordi.aviso != 25) {
		salir_con_error("Respuesta errónea.", server_socket);
	}

	uint32_t size = (uint32_t) strlen(clave) + 1;

	package_int size_package = { .packed = size };

	packageSize = sizeof(package_int);
	char* package_de_size = malloc(packageSize);

	serializar_packed(size_package, &package_de_size);

	send(server_socket, package_de_size, packageSize, 0);

	packageSize = size;

	sleep(2);

	send(server_socket, clave, packageSize, 0);

	packageSize = sizeof(package_int);
	char* response_package = malloc(packageSize);
	package_int response;

	res = recv(server_socket, response_package, packageSize, 0);

	if (res <= 0) {
		salir_con_error("Falló la respuesta del coordinador.", server_socket);
	}

	deserializar_packed(&(response), &(response_package));

	if (response.packed != 26) {
		salir_con_error("Falló el bloqueo de la clave", server_socket);
	}

	free(message);
	free(res_message);
	free(package_de_size);
	free(response_package);

	log_trace(logger, "La clave %s fue bloqueada.", clave);

}

void dame_datos() {
	t_esi_node* puntero = new_ESIs.head;

	printf("ESI ejecutando: %i \n", executing_ESI.id);
	printf("ESIs listos para ejecutar: ");
	while (puntero != NULL) {
		printf("%i, ", new_ESIs.head->esi.id);
		puntero = puntero->sgte;
	}

	printf("\n");

	puntero = blocked_ESIs.head;

	printf("ESIs bloqueados: ");
	while (puntero != NULL) {
		printf("%i, ", new_ESIs.head->esi.id);
		puntero = puntero->sgte;
	}

	printf("\n");

	puntero = finished_ESIs.head;

	printf("ESIs terminados: ");
	while (puntero != NULL) {
		printf("%i, ", new_ESIs.head->esi.id);
		puntero = puntero->sgte;
	}

	printf("\n");
}

void display_console() {
	display = !display;
}

void mostrame_clock(void) {
	pthread_mutex_lock(&sem_clock);
	printf("El clock de instrucciones va por %i. \n", tiempo);
	pthread_mutex_unlock(&sem_clock);
}

void terminar(void) {
	printf("Eligio cerrar el planificador \n");
	seguir_ejecucion = false;

	avisar_cierre(socket_coordinador);

	exit(42);
}

void cerrar_ESIs() {
	t_esi_node* puntero = new_ESIs.head;

	while (puntero != NULL) {
		kill_ESI(puntero->esi);

		sleep(1);

		puntero = puntero->sgte;
	}

}

void listarOpciones() {
	printf("0: Cancelar consola \n");
	printf("1: Pausar o reactivar la planificación \n");
	printf("2.<ESI ID>: Bloquea al ESI elegido \n");
	printf("3.<ESI ID>: Desbloquea al ESI elegido \n");
	printf("4.<Recurso>: Lista procesos esperando dicho recurso \n");
	printf("5.<ESI ID>: Mata al ESI elegido \n");
	printf("6.<ESI ID>: Brinda el estado del ESI elegido \n");
	printf("7: Lista los ESI en deadlock \n");
	printf("8: Termina el proceso \n");
	printf("9: Muestra el clock interno del planificador \n");
	printf("10: Enciende o apaga el display de las opciones de consola \n");
	printf(
			"11: Muestra datos de la ejecución (ESI ejecutando, ESIs listos, bloqueados y terminados \n");
	printf("12: Bloquea una clave \n");
	printf("13: Desbloquea una clave \n");
	printf("Introduzca la opcion deseada \n");

}

void* consola(void* nada) {
	float comando;
	printf("Bienvenido a la consola interactiva para el planificador \n");
	while (1) {
		if (display) {
			listarOpciones();
		}

		comando = recibirCodigo();
		interpretarYEjecutarCodigo(comando);

		if (!seguir_ejecucion) {
			break;
		}
	}

	return NULL;
}
void pausarOContinuar(void) {
	if (consola_planificacion) {
		printf("Pausando planificación...\n");

		consola_planificacion = false;

		sleep(1);

		printf("Planificación pausada.\n");

	}

	else {
		printf("Reanudando planificacion...\n");

		consola_planificacion = true;

		sleep(1);

		printf("Planificación reanudada.\n");
		planificar();

	}

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
