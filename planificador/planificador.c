/*
 * ========================
 * ===== PLANIFICADOR =====
 * ========================
 */

#include "planificador.h"

int main(int argc, char** argv) {
	iniciar();

	manejar_conexiones();
	cerrar();

	return EXIT_SUCCESS;
}

void manejar_conexiones(void) {
	int listening_socket = levantar_servidor(PUERTO_PLANIFICADOR);
	int socketCliente;

	while (seguir_ejecucion) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				id_planificador);
	}

	loggear("Terminando proceso...");

	close(listening_socket);
	close(socketCliente);
}

void cerrar(void) {
	avisar_cierre(socket_coordinador);

	close(socket_coordinador);

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

	ALFA = (float) config_get_int_value(config, "ALFA");
	log_info(logger, "Alfa: %f", ALFA);

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
			id_planificador);

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
	pthread_mutex_init(&sem_new_ESIs, NULL);
	pthread_mutex_init(&sem_ESI_ID, NULL);
}

int manejar_cliente(int listening_socket, int socket_cliente,
		package_int server_packed) {

	loggear("Esperando cliente...");

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	socket_cliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	loggear("Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	package_int cliente_packed = { .packed = -1 };

	cliente_packed = recibir_packed(socket_cliente);

	loggear("Mensaje recibido exitosamente. Identificando cliente...");
	identificar_cliente(cliente_packed, socket_cliente);

	loggear("Enviando mensaje al cliente.");

	enviar_packed(server_packed, socket_cliente);

	loggear("Mensaje enviado. Cerrando sesion con el cliente actual.");

	return socket_cliente;
}

void identificar_cliente(package_int id, int socket_cliente) {
	pthread_t hilo_ESI;

	if (id.packed == 2) {
		loggear(mensajeESI);

		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);

		loggear(mensajeESILista);

		pthread_detach(hilo_ESI);
	}

	else {

		salir_con_error("Cliente desconocido, cerrando conexion.",
				socket_cliente);
	}

	return;
}

void* atender_ESI(void* buffer) {
	int socket_ESI = (int) buffer;

	loggear("Hilo de ESI inicializado correctamente.");

	ESI esi = { .socket = socket_ESI, .rafaga_estimada = ESTIMACION_INICIAL,
			.rafaga_real = 0 };

	int this_id = asignar_ID(esi);

	esi.id = this_id;

	int status = 1;

	while (status) {
		status = recibir_mensaje(socket_ESI, this_id, esi);

		planificar();
	}

	planificar();

	log_trace(logger, "Hilo de ESI número %i terminado.", this_id);

	return NULL;
}

int recibir_mensaje(int socket_cliente, int id, ESI esi) {
	aviso_con_ID aviso = recibir_aviso(socket_cliente);

	log_trace(logger, "Mensaje recibidio del ESI numero: %i", id);

	if (aviso.aviso == 0) {
		loggear(
				"ESI terminado. Moviendo a la cola de terminados y eliminando de la cola de listos.");

		agregar_ESI(&finished_ESIs, esi);

		pthread_mutex_lock(&sem_new_ESIs);
		pthread_mutex_lock(&sem_ESIs_size);
		if (!esta(new_ESIs, esi)) {
			agregar_ESI(&new_ESIs, esi);
			ESIs_size++;
		}

		eliminar_ESI(&new_ESIs, esi);

		ESIs_size--;
		pthread_mutex_unlock(&sem_ESIs_size);
		pthread_mutex_unlock(&sem_new_ESIs);

		vaciar_ESI();

		loggear("Agregado correctamente a la cola de terminados.");

		pthread_mutex_lock(&sem_clock);
		tiempo++;
		pthread_mutex_unlock(&sem_clock);

		pthread_mutex_lock(&sem_ejecutando);
		ejecutando = false;
		pthread_mutex_unlock(&sem_ejecutando);

		pthread_mutex_unlock(&sem_ejecucion);

		return 0;
	}

	else if (aviso.aviso == 1) {
		pthread_mutex_lock(&sem_clock);
		esi.tiempo_arribo = tiempo;
		pthread_mutex_unlock(&sem_clock);

		pthread_mutex_unlock(&sem_new_ESIs);
		pthread_mutex_lock(&sem_ESIs_size);
		agregar_ESI(&new_ESIs, esi);

		ESIs_size++;
		pthread_mutex_unlock(&sem_ESIs_size);
		pthread_mutex_unlock(&sem_new_ESIs);

		log_trace(logger,
				"ESI número %i listo para ejecutar añadido a la cola.", id);

		if (ALGORITMO_PLANIFICACION.desalojo) {
			desalojar();
		}

	}

	else if (aviso.aviso == 5) {
		pthread_mutex_lock(&sem_new_ESIs);
		pthread_mutex_lock(&sem_ESIs_size);
		eliminar_ESI(&new_ESIs, esi);

		ESIs_size--;
		pthread_mutex_unlock(&sem_new_ESIs);
		pthread_mutex_unlock(&sem_ESIs_size);

		pthread_mutex_lock(&sem_ejecutando);
		ejecutando = false;
		pthread_mutex_unlock(&sem_ejecutando);

		pthread_mutex_unlock(&sem_ejecucion);

		agregar_ESI(&blocked_ESIs, esi);

		log_trace(logger, "ESI número %i fue bloqueado.", id);

		vaciar_ESI();
	}

	else if (aviso.aviso == 10) {
		pthread_mutex_lock(&sem_clock);
		tiempo++;
		pthread_mutex_unlock(&sem_clock);

		pthread_mutex_lock(&sem_ejecutando);
		ejecutando = false;
		pthread_mutex_unlock(&sem_ejecutando);

		executing_ESI.rafaga_real++;

		log_trace(logger, "ESI número %i ejecutó correctamente.", id);

		pthread_mutex_unlock(&sem_ejecucion);
	}

	else {

		loggear("El ESI se volvió loco. Terminando.");
		kill_ESI(esi);

		return 0;
	}

	return 1;
}

bool esta(t_esi_list lista, ESI esi) {
	t_esi_node* puntero = lista.head;

	while (puntero != NULL) {
		if (puntero->esi.id == esi.id) {
			return true;
		}

		puntero = puntero->sgte;
	}

	return false;
}

void desalojar(void) {
	if (executing_ESI.id != esi_vacio.id) {
		pthread_mutex_lock(&sem_new_ESIs);
		eliminar_ESI(&new_ESIs, executing_ESI);

		ESI new_ESI = executing_ESI;

		log_debug(logger, "%f", ALFA);

		log_debug(logger, "%i", new_ESI.rafaga_real);
		log_debug(logger, "%f", new_ESI.rafaga_estimada);
		log_debug(logger, "%f", estimated_time(new_ESI));

		log_debug(logger, "%f", tiempo_real(new_ESI));
		log_debug(logger, "%f", estimado(new_ESI));

		new_ESI.rafaga_estimada = estimated_time(new_ESI);

		agregar_ESI(&new_ESIs, new_ESI);

		pthread_mutex_unlock(&sem_new_ESIs);

		vaciar_ESI();

		loggear("ESI desalojado.");
	}
}

void vaciar_ESI(void) {
	executing_ESI = esi_vacio;
}

bool no_hay_ESI() {
	return executing_ESI.id == esi_vacio.id;
}

void kill_ESI(ESI esi) {
	int socket_ESI = esi.socket;

	aviso_con_ID aviso = { .aviso = -1, .id = esi.id };

	int packageSize = sizeof(aviso_con_ID);
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
	aviso_id.id = ESI_id;

	ESI_id++;

	enviar_aviso(socket_ESI, aviso_id);
	pthread_mutex_unlock(&sem_ID);

	return (int) aviso_id.id;

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

	while (puntero != NULL) {
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

	while (puntero != NULL) {
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

float estimated_time(ESI esi) {
	return tiempo_real(esi) + estimado(esi);
}

float tiempo_real(ESI esi) {
	return ((float) esi.rafaga_real) * (ALFA / 100);
}

float estimado(ESI esi) {
	return ((float) esi.rafaga_estimada) * (1 - (ALFA / 100));
}

void ejecutar(ESI esi_a_ejecutar) {

	pthread_mutex_lock(&sem_ejecutando);
	ejecutando = true;
	pthread_mutex_unlock(&sem_ejecutando);

	int socket_ESI = esi_a_ejecutar.socket;
	executing_ESI = esi_a_ejecutar;

	loggear("Enviando orden de ejecucion.");
	aviso_con_ID orden_ejecucion = { .aviso = 2, .id = esi_a_ejecutar.id };

	enviar_aviso(socket_ESI, orden_ejecucion);

	loggear("Orden enviada.");

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
	case 14:
		desalojar();
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
	scanf("%s", clave);

	avisar_desbloqueo(socket_coordinador, clave);

	printf("Clave desbloqueada \n");

}

void avisar_desbloqueo(int server_socket, char* clave) {
	aviso_con_ID aviso_coordi = { .aviso = -1 };

	uint32_t size = (uint32_t) strlen(clave) + 1;

	package_int size_package = { .packed = size };
	package_int response = { .packed = -1 };

	enviar_aviso(server_socket, aviso_desbloqueo);
	aviso_coordi = recibir_aviso(server_socket);

	if (aviso_coordi.aviso != 25) {
		salir_con_error("Respuesta errónea.", server_socket);
	}

	enviar_packed(size_package, server_socket);
	sleep(2);
	enviar_cadena(clave, server_socket);

	response = recibir_packed(server_socket);

	if (response.packed != 28) {
		salir_con_error("Falló el bloqueo de la clave", server_socket);
	}

	log_trace(logger, "La clave %s fue desbloqueada.", clave);
}

void bloquear_clave() {
	printf("Ingrese la clave a bloquear: ");
	char clave[40] = "futbol:messi";
	scanf("%s", clave);

	avisar_bloqueo(socket_coordinador, clave);

	printf("Clave bloqueada \n");
}

void avisar_bloqueo(int server_socket, char* clave) {
	aviso_con_ID aviso_coordi = { .aviso = -1 };

	uint32_t size = (uint32_t) strlen(clave) + 1;

	package_int size_package = { .packed = size };
	package_int response = { .packed = -1 };

	enviar_aviso(server_socket, aviso_bloqueo);
	aviso_coordi = recibir_aviso(server_socket);

	if (aviso_coordi.aviso != 25) {
		salir_con_error("Respuesta errónea.", server_socket);
	}

	enviar_packed(size_package, server_socket);
	sleep(2);
	enviar_cadena(clave, server_socket);

	response = recibir_packed(server_socket);

	if (response.packed != 26) {
		salir_con_error("Falló el bloqueo de la clave", server_socket);
	}

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
