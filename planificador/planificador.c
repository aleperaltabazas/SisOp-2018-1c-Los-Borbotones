/*
 * ========================
 * ===== PLANIFICADOR =====
 * ========================
 */

#include "planificador.h"

int listening_socket;

int main(int argc, char** argv) {
	iniciar(argv);

	manejar_conexiones();
	cerrar();

	return EXIT_SUCCESS;
}

void manejar_conexiones(void) {
	listening_socket = levantar_servidor(PUERTO_PLANIFICADOR, 0);
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

void iniciar(char** argv) {
	iniciar_log("Planificador", "Nace el planificador...");

	cargar_configuracion(argv);
	iniciar_semaforos();
	iniciar_hilos();
	startSigHandlers();
}

void startSigHandlers(void) {
	signal(SIGINT, sigHandler_sigint);
	signal(SIGSEGV, sigHandler_segfault);
}

void sigHandler_segfault(int signo) {
	log_warning(logger, "uh la puta madre, seg fault.");
	log_error(logger, strerror(errno));

	close(listening_socket);
	exit(-1);
}

void sigHandler_sigint(int signo) {
	log_warning(logger, "Tiraste un CTRL+C, macho, abortaste el proceso.");
	log_error(logger, strerror(errno));

	close(listening_socket);
	exit(0);
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

void cargar_configuracion(char** argv) {

	executing_ESI = esi_vacio;

	ESI_id = 1;

	ESIs_size = 0;
	tiempo = 0;

	t_config* config = config_create(argv[1]);

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

	CLAVES_BLOQUEADAS = config_get_array_value(config, "CLAVES_BLOQUEADAS");

	loggear("Configuración cargada.");
}

void bloqueo_inicial(void) {
	int i = 0;

	while (CLAVES_BLOQUEADAS[i] != NULL) {
		aviso_con_ID aviso_bloqueo = { .aviso = 25 };

		enviar_aviso(socket_coordinador, aviso_bloqueo);
		aviso_con_ID ok = recibir_aviso(socket_coordinador);

		if (ok.aviso != 25) {
			salir_con_error("Falló el ok", socket_coordinador);
		}

		uint32_t size = (uint32_t) strlen(CLAVES_BLOQUEADAS[i]) + 1;
		package_int size_package = { .packed = size };

		log_debug(logger, "%s", CLAVES_BLOQUEADAS[i]);

		enviar_packed(size_package, socket_coordinador);
		sleep(1);
		enviar_cadena(CLAVES_BLOQUEADAS[i], socket_coordinador);

		package_int response = recibir_packed(socket_coordinador);

		if (response.packed != 26) {
			salir_con_error("Falló el bloqueo", socket_coordinador);
		}

		i++;
	}

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
			id_planificador, 0);

	int local_socket = socket_coordinador;

	bloqueo_inicial();

	int status = 1;

	while (status) {
		status = recibir_respuesta(local_socket);
	}

	return NULL;
}

int recibir_respuesta(int server_socket) {

	aviso_con_ID aviso_coordi = recibir_aviso(server_socket);

	log_debug(logger, "%i", aviso_coordi.aviso);

	if (aviso_coordi.aviso == 25) {
		pthread_mutex_unlock(&sem_socket_coordi);
	}

	else if (aviso_coordi.aviso == 26) {
		pthread_mutex_unlock(&sem_socket_coordi);
	}

	else if (aviso_coordi.aviso == 27) {
		pthread_mutex_unlock(&sem_socket_coordi);
	}

	else if (aviso_coordi.aviso == 28) {
		pthread_mutex_unlock(&sem_socket_coordi);
		if (aviso_coordi.id != -5) {
			desbloquear_ESI(aviso_coordi.id);
		}
	}

	else if (aviso_coordi.aviso == 61) {
		loggear("Finish Status");
		finishStatus();
	} else if (aviso_coordi.aviso == 71) {
		loggear("Finish listar");
		finishListar();
	}

	else {
		avisar_cierre(socket_coordinador);
		log_error(logger, "%s", strerror(errno));
		salir_con_error("Mensaje erróneo recibido del coordinador.",
				server_socket);
	}

	return 1;
}

void finishListar(void) {
	package_int size_package = recibir_packed(socket_coordinador);
	char* bloqueados = recibir_cadena(socket_coordinador, size_package.packed);

	printf("Los ESIs bloqueados esperando la clave son: %s", bloqueados);
	pthread_mutex_unlock(&sem_console_coordi);
}

void finishStatus() {
	package_int valor_size = recibir_packed(socket_coordinador);
	char* valor = recibir_cadena(socket_coordinador, valor_size.packed);
	log_debug(logger, "Valor: %s", valor);

	package_int instancia_size = recibir_packed(socket_coordinador);
	char* instancia = recibir_cadena(socket_coordinador, instancia_size.packed);
	log_debug(logger, "Instancia: %s", instancia);

	package_int blockedSize = recibir_packed(socket_coordinador);
	char* blockeds = recibir_cadena(socket_coordinador, blockedSize.packed);
	log_debug(logger, "Bloqueados: %s", blockeds);

	if (mismoString(valor, "La clave no existe.")) {
		printf("La clave no existe \n.");
		free(valor);
		free(instancia);
		free(blockeds);
		return;
	}

	printf("Valor: %s \n", valor);
	printf("Instancia: %s \n", instancia);
	printf("Bloqueados esperando: %s \n", blockeds);

	free(valor);
	free(instancia);
	free(blockeds);

	pthread_mutex_unlock(&sem_console_coordi);
	loggear("Hice signal");
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
	pthread_mutex_init(&sem_socket_coordi, NULL);
	pthread_mutex_init(&sem_server_socket, NULL);
	pthread_mutex_init(&sem_console_coordi, NULL);

	pthread_mutex_lock(&sem_socket_coordi);
	pthread_mutex_lock(&sem_console_coordi);
}

int manejar_cliente(int listening_socket, int socket_cliente,
		package_int server_packed) {

	log_info(logger, "Esperando cliente...");

	listen(listening_socket, BACKLOG);

	loggear("Esperando...");
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	socket_cliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	log_info(logger, "Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	package_int cliente_packed = { .packed = -1 };

	cliente_packed = recibir_packed(socket_cliente);

	log_debug(logger, "%i", cliente_packed.packed);

	loggear("Mensaje recibido exitosamente. Identificando cliente...");
	identificar_cliente(cliente_packed, socket_cliente);

	loggear("Enviando mensaje al cliente.");

	enviar_packed(server_packed, socket_cliente);

	loggear("Mensaje enviado. Handshake realizado.");

	return socket_cliente;
}

void identificar_cliente(package_int id, int socket_cliente) {
	pthread_t hilo_ESI;

	log_debug(logger, "%i", id);

	if (id.packed == 2) {
		loggear(mensajeESI);

		pthread_create(&hilo_ESI, NULL, atender_ESI,
				(void *) (intptr_t) socket_cliente);

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
	int socket_ESI = (intptr_t) buffer;

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
	close(socket_ESI);

	return NULL;
}

int recibir_mensaje(int socket_cliente, int id, ESI esi) {
	aviso_con_ID aviso = recv_aviso_no_exit(socket_cliente);

	log_trace(logger, "Mensaje recibido del ESI numero: %i", id);

	if (aviso.aviso == 0) {
		log_info(logger,
				"ESI número %i terminado. Moviendo a la cola de terminados y eliminando de la cola de listos.",
				id);

		agregar_ESI(&finished_ESIs, esi);

		pthread_mutex_lock(&sem_new_ESIs);
		pthread_mutex_lock(&sem_ESIs_size);
		if (!esta(new_ESIs, esi)) {
			return 0;
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

		pthread_mutex_lock(&sem_new_ESIs);
		pthread_mutex_lock(&sem_ESIs_size);
		agregar_ESI(&new_ESIs, esi);

		ESIs_size++;
		pthread_mutex_unlock(&sem_ESIs_size);
		pthread_mutex_unlock(&sem_new_ESIs);

		log_info(logger, "ESI número %i listo para ejecutar añadido a la cola.",
				id);

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

		log_info(logger, "ESI número %i fue bloqueado.", id);

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

		log_info(logger, "ESI número %i ejecutó correctamente.", id);

		pthread_mutex_unlock(&sem_ejecucion);
	}

	else {
		log_warning(logger, "El ESI %i se cayó o fue abortado.", id);
		if (executing_ESI.id == (uint32_t) id) {

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

			pthread_mutex_lock(&sem_ejecutando);
			ejecutando = false;
			pthread_mutex_unlock(&sem_ejecutando);

			pthread_mutex_unlock(&sem_ejecucion);
		}

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

		log_debug(logger, "Alfa: %f", ALFA);

		log_debug(logger, "Rafaga real: %i", new_ESI.rafaga_real);
		log_debug(logger, "Rafaga estimada anterior: %f",
				new_ESI.rafaga_estimada);
		log_debug(logger, "Estimación de la próxima ráfaga: %f",
				estimated_time(new_ESI));

		new_ESI.rafaga_estimada = estimated_time(new_ESI);

		log_debug(logger, "Rafaga real: %i", new_ESI.rafaga_real);
		log_debug(logger, "Rafaga estimada anterior: %f",
				new_ESI.rafaga_estimada);
		log_debug(logger, "Estimación de la próxima ráfaga: %f",
				estimated_time(new_ESI));

		agregar_ESI(&new_ESIs, new_ESI);

		pthread_mutex_unlock(&sem_new_ESIs);

		vaciar_ESI();

		log_info(logger, "ESI desalojado.");
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
		loggear("Falló la terminación. Intentando de vuelta.");
		kill_ESI(esi);
	}

	log_info(logger, "ESI número %i has fainted!", esi.id);
}

int asignar_ID(ESI esi) {
	int socket_ESI = esi.socket;

	aviso_con_ID aviso;
	aviso.aviso = 1;

	pthread_mutex_lock(&sem_ID);
	aviso.id = ESI_id;

	ESI_id++;
	pthread_mutex_unlock(&sem_ID);

	enviar_aviso(socket_ESI, aviso);

	log_debug(logger, "Aviso: %i", aviso.aviso);
	log_debug(logger, "ID: %i", aviso.id);

	return (int) aviso.id;

}

void planificar(void) {
	if (consola_planificacion && ESIs_size > 0 && !ejecutando) {
		pthread_mutex_lock(&sem_ejecucion);

		ESI next_esi = executing_ESI;

		if (no_hay_ESI()) {
			next_esi = dame_proximo_ESI();
			log_info(logger, "ESI número %i elegido.", next_esi.id);

		}

		ejecutar(next_esi);
	}
}

void ejecutar(ESI esi_a_ejecutar) {

	pthread_mutex_lock(&sem_ejecutando);
	ejecutando = true;
	pthread_mutex_unlock(&sem_ejecutando);

	int socket_ESI = esi_a_ejecutar.socket;
	executing_ESI = esi_a_ejecutar;

	loggear("Enviando orden de ejecucion.");
	aviso_con_ID orden_ejecucion = { .aviso = 2, .id = esi_a_ejecutar.id };

	send_aviso_no_exit(orden_ejecucion, socket_ESI);

}

ESI dame_proximo_ESI() {
	ESI next_esi;
	switch (ALGORITMO_PLANIFICACION.tipo) {
	case FIFO:
		next_esi = headESIs(new_ESIs);
		break;
	case SJF:
		next_esi = shortest(new_ESIs);
		break;
	case HRRN:
		next_esi = highest_RR(new_ESIs);
		break;
	default:
		log_error(logger, "FALLO EN EL ALGORITMO.");
		break;

	}

	return next_esi;
}

ESI shortest(t_esi_list lista) {
	t_esi_node* puntero = lista.head;

	ESI esi = headESIs(lista);

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

	ESI esi = headESIs(lista);

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

/*	=====================
 *	===== CONSOLITA =====
 *	=====================
 */

float recibirCodigo() {
	int code = 0;
	scanf("%i", &code);
	return code;
}

bool existe(uint32_t id) {
	ESI blocked = findByIDIn(id, blocked_ESIs);
	if (blocked.id != ESI_error.id) {
		return true;
	}

	ESI finished = findByIDIn(id, finished_ESIs);
	if (finished.id != ESI_error.id) {
		return true;
	}

	ESI ready = findByIDIn(id, new_ESIs);
	if (ready.id != ESI_error.id) {
		return true;
	}

	return false;
}

void bloquearSegunClave(void) {
	int id = 0;
	printf("Ingrese el ID: ");
	scanf("%i", &id);

	if (!existe(id)) {
		printf("Ese ESI no se encuentra en el sistema.");
		return;
	}

	ESI finished = get_ESI((uint32_t) id, finished_ESIs);

	if (finished.id != ESI_error.id) {
		printf("El ESI ya terminó.");
		return;
	}

	ESI blocked = get_ESI((uint32_t) id, blocked_ESIs);

	if (blocked.id != ESI_error.id) {
		printf("El ESI ya está bloqueado.");
		return;
	}

	ESI esi = get_ESI((uint32_t) id, new_ESIs);
	pthread_mutex_lock(&sem_ESIs_size);
	eliminar_ESI(&new_ESIs, esi);

	ESIs_size--;
	pthread_mutex_unlock(&sem_ESIs_size);
	agregar_ESI(&blocked_ESIs, esi);

	char clave[40] = "futbol:messi";
	printf("Ingrese la clave: ");
	scanf("%s", clave);

	log_warning(logger, "El ESI %i fue bloqueado por la clave %s", id, clave);

	avisarBloqueoESIPorClave(esi, clave, socket_coordinador);
}

void avisarBloqueoESIPorClave(ESI esi, char* clave, int sockfd) {
	aviso_con_ID aviso_bloqueo = { .aviso = 70 };

	enviar_aviso(sockfd, aviso_bloqueo);

	package_int id_package = { .packed = esi.id };

	enviar_packed(id_package, sockfd);

	package_int size_package = { .packed = strlen(clave) + 1 };

	enviar_packed(size_package, sockfd);
	enviar_cadena(clave, sockfd);
}

void listar_bloqueados(void) {
	printf("Ingrese la clave: ");
	char clave[255] = "futbol:messi";

	scanf("%s", clave);

	aviso_con_ID aviso_bloqueados = { .aviso = 71 };

	enviar_aviso(socket_coordinador, aviso_bloqueados);

	uint32_t clave_size = (uint32_t) strlen(clave) + 1;

	package_int size_package = { .packed = clave_size };

	enviar_packed(size_package, socket_coordinador);
	enviar_cadena(clave, socket_coordinador);

	pthread_mutex_lock(&sem_console_coordi);
}

void interpretarYEjecutarCodigo(int comando) {
	int codigoSubsiguiente;
	switch (comando) {
	case 1:
		pausarOContinuar();
		break;
	case 2:
		bloquearSegunClave();
		break;
	case 3:
		desbloquear_clave();
		break;
	case 4:
		listar_bloqueados();
		break;
	case 5:
		printf("Introduzca el ESI ID: ");
		scanf("%i", &codigoSubsiguiente);
		kill_esi(codigoSubsiguiente);
		break;
	case 6:
		status();
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
		desalojar();
		break;
	case 420:
		weed();
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

	int local_socket = socket_coordinador;

	avisar_desbloqueo(local_socket, clave);

	printf("Clave desbloqueada \n");

}

void avisar_desbloqueo(int server_socket, char* clave) {
	uint32_t size = (uint32_t) strlen(clave) + 1;

	package_int size_package = { .packed = size };

	enviar_aviso(server_socket, aviso_desbloqueo);

	pthread_mutex_lock(&sem_socket_coordi);

	enviar_packed(size_package, server_socket);
	enviar_cadena(clave, server_socket);

	pthread_mutex_lock(&sem_socket_coordi);

	log_trace(logger, "La clave %s fue desbloqueada.", clave);

}

void desbloquear_ESI(uint32_t id) {
	log_debug(logger, "%i", id);

	ESI desbloqueado = get_ESI(id, blocked_ESIs);

	eliminar_ESI(&blocked_ESIs, desbloqueado);

	pthread_mutex_lock(&sem_clock);
	desbloqueado.tiempo_arribo = tiempo;
	pthread_mutex_unlock(&sem_clock);

	pthread_mutex_lock(&sem_new_ESIs);
	pthread_mutex_lock(&sem_ESIs_size);
	agregar_ESI(&new_ESIs, desbloqueado);

	ESIs_size++;
	pthread_mutex_unlock(&sem_new_ESIs);
	pthread_mutex_unlock(&sem_ESIs_size);

	log_trace(logger, "El ESI número %i fue desbloqueado.", id);

	if (ALGORITMO_PLANIFICACION.desalojo) {
		desalojar();
	}

	planificar();
}

ESI get_ESI(uint32_t id, t_esi_list lista) {
	t_esi_node* puntero = lista.head;

	while (puntero != NULL) {
		if (id == puntero->esi.id) {
			return puntero->esi;
		}

		puntero = puntero->sgte;
	}

	return ESI_error;
}

void bloquear_clave() {
	printf("Ingrese la clave a bloquear: ");
	char clave[40] = "futbol:messi";
	scanf("%s", clave);

	int local_socket = socket_coordinador;

	avisar_bloqueo(local_socket, clave);

	printf("Clave bloqueada \n");
}

void avisar_bloqueo(int server_socket, char* clave) {
	uint32_t size = (uint32_t) strlen(clave) + 1;

	package_int size_package = { .packed = size };

	enviar_aviso(server_socket, aviso_bloqueo);
	pthread_mutex_lock(&sem_socket_coordi);

	enviar_packed(size_package, server_socket);
	enviar_cadena(clave, server_socket);

	pthread_mutex_lock(&sem_socket_coordi);

	log_trace(logger, "La clave %s fue bloqueada.", clave);

}

void dame_datos() {
	printf("ESI ejecutando: %i \n", executing_ESI.id);

	t_esi_node* puntero = new_ESIs.head;
	printf("ESIs listos para ejecutar: ");
	mostrar(puntero);

	puntero = blocked_ESIs.head;
	printf("ESIs bloqueados: ");
	mostrar(puntero);

	puntero = finished_ESIs.head;
	printf("ESIs terminados: ");
	mostrar(puntero);
}

void mostrar(t_esi_node* puntero) {
	while (puntero != NULL) {
		printf("%i, ", puntero->esi.id);
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
	printf("1: Pausar o reactivar la planificación \n");
	printf("2: Bloquea un ESI detrás de una clave \n");
	printf("3: Desbloquea una clave \n");
	printf("4: Lista los procesos esperando un recurso \n");
	printf("5: Mata al ESI elegido \n");
	printf("6: Brinda el estado del ESI elegido \n");
	printf("7: Lista los ESI en deadlock \n");
	printf("8: Termina el proceso \n");
	printf("9: Muestra el clock interno del planificador \n");
	printf("10: Enciende o apaga el display de las opciones de consola \n");
	printf(
			"11: Muestra datos de la ejecución (ESI ejecutando, ESIs listos, bloqueados y terminados \n");
	printf("12: Bloquea una clave \n");
	printf("13: Desaloja al ESI actual \n");
	printf("Introduzca la opcion deseada \n");

}

void* consola(void* nada) {
	int comando;
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
void bloquear(int codigo) {
	printf("Eligio bloquear el ESI \n");
}
void desbloquear(int codigo) {
}
void listar(char* clave) { //Comunicarse con el coordi para que busque al ESI

}

ESI findByIDIn(uint32_t id, t_esi_list lista) {
	t_esi_node* puntero = lista.head;

	while (puntero != NULL) {
		if (puntero->esi.id == id) {
			return puntero->esi;
		}

		puntero = puntero->sgte;
	}

	return ESI_error;
}

void kill_esi(int id) {
	uint32_t id_as_uint = (uint32_t) id;

	ESI asesina3 = findByIDIn(id_as_uint, new_ESIs);

	if (asesina3.id != ESI_error.id) {
		kill_ESI(asesina3);
		eliminar_ESI(&new_ESIs, asesina3);
		printf("ESI %i abortado. \n", id);
		return;
	}

	asesina3 = findByIDIn(id_as_uint, blocked_ESIs);

	if (asesina3.id != ESI_error.id) {
		kill_ESI(asesina3);
		printf("ESI %i abortado. \n", id);
		return;
	}

	asesina3 = findByIDIn(id_as_uint, finished_ESIs);

	if (asesina3.id != ESI_error.id) {
		kill_ESI(asesina3);
		printf("El ESI %i ya terminó, así que no pudo ser abortado. \n", id);
		return;
	}

	printf(
			"El ESI %i no se encuentra en el sistema, por lo que no se pudo abortar. \n",
			id);
}

void status(void) {
	char recurso[255] = "futbol:messi";
	printf("Introduzca el recurso: ");
	scanf("%s", recurso);

	cerrar_cadena(recurso);

	aviso_con_ID aviso_status = { .aviso = 60 };

	enviar_aviso(socket_coordinador, aviso_status);

	uint32_t size = strlen(recurso) + 1;
	package_int size_package = { .packed = size };

	enviar_packed(size_package, socket_coordinador);
	enviar_cadena(recurso, socket_coordinador);

	pthread_mutex_lock(&sem_console_coordi);
	loggear("Hice wait");
	return;
}

void deadlock(void) {
	package_int paquete;
	char * ids;
	paquete.packed = 62;
	enviar_packed(paquete, socket_coordinador);
	paquete.packed = recibir_packed(socket_coordinador).packed;
	strcpy(ids, recibir_cadena(socket_coordinador, paquete.packed));
	printf("%s", ids);
}

void listarESI(t_esi_node lista) {
	/*printf("Los ESI esperando la clave pedida son: \n");
	 while (lista.sgte != NULL){
	 printf("%i", lista.esi.id);
	 printf("\n");
	 lista = lista.sgte;
	 }*/
}
ESI copiarEsi(t_esi_node * lista, ESI esiACopiar) {
	while (lista->sgte != NULL) {
		if (lista->esi.id == esiACopiar.id) {
			esiACopiar.ejecutando = lista->esi.ejecutando;
			esiACopiar.rafaga_estimada = lista->esi.rafaga_estimada;
			esiACopiar.rafaga_real = lista->esi.rafaga_real;
			esiACopiar.socket = lista->esi.socket;
			esiACopiar.tiempo_arribo = lista->esi.tiempo_arribo;
			return esiACopiar;
		}
		lista = lista->sgte;
	}
	return esiACopiar;
}

void weed() {
	/*
	 printf(WEED "                     .                          ", 48);
	 printf(WEED "                     M                          ", 48);
	 printf(WEED "                    dM                          ", 48);
	 printf(WEED "                    MMr                         ", 48);
	 printf(WEED "                   4MMML                  .     ", 48);
	 printf(WEED "                   MMMMM.                xf     ", 48);
	 printf(WEED "   .              \"MMMMM               .MM-     ", 48);
	 printf(WEED "    Mh..          +MMMMMM            .MMMM      ", 48);
	 printf(WEED "    .MMM.         .MMMMML.          MMMMMh      ", 48);
	 printf(WEED "     )MMMh.        MMMMMM         MMMMMMM       ", 48);
	 printf(WEED "      3MMMMx.     'MMMMMMf      xnMMMMMM\"       ", 48);
	 printf(WEED "      '*MMMMM      MMMMMM.     nMMMMMMP\"        ", 48);
	 printf(WEED "        *MMMMMx    \"MMMMM\    .MMMMMMM=         ", 48);
	 printf(WEED "         *MMMMMh   \"MMMMM\"   JMMMMMMP           ", 48);
	 printf(WEED "           MMMMMM   3MMMM.  dMMMMMM            .", 48);
	 printf(WEED "            MMMMMM  \"MMMM  .MMMMM(        .nnMP\"", 48);
	 printf(WEED "=..          *MMMMx  MMM\"  dMMMM\"    .nnMMMMM*  ", 48);
	 printf(WEED "  \"MMn...     'MMMMr 'MM   MMM\"   .nMMMMMMM*\"   ", 48);
	 printf(WEED "   \"4MMMMnn..   *MMM  MM  MMP\"  .dMMMMMMM\"\"     ", 48);
	 printf(WEED "     ^MMMMMMMMx.  *ML \"M .M*  .MMMMMM**\"        ", 48);
	 printf(WEED "        *PMMMMMMhn. *x > M  .MMMM**\"\"           ", 48);
	 printf(WEED "           " "**MMMMhx/.h/ .=*\"                  ", 48);
	 printf(WEED "                    .3P\"%....                   ", 48);
	 printf(WEED "                  nP" "*MMnx                ", 48);
	 */

	printf(WEED "S M O K E   W E E D   E V E R Y D A Y \n");
}
