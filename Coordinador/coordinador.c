/*
 * coordinador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"

int listening_socket;

int main(int argc, char** argv) {
	iniciar(argv);

	coordinar();

	loggear("Terminando proceso...");

	return EXIT_SUCCESS;
}

void coordinar(void) {
	listening_socket = levantar_servidor(PUERTO_COORDINADOR, 0);

	int socketCliente;

	while (seguir_ejecucion) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				id_coordinador);
	}

	close(socketCliente);
	close(listening_socket);
}

void iniciar(char** argv) {
	iniciar_log("Coordinador", "Nace el coordinador...");
	cargar_configuracion(argv);

	log_operaciones = log_create("Operaciones.log", "Log de operaciones", false,
			LOG_LEVEL_INFO);

	log_info(log_operaciones, "Logger iniciado correctamente.");

	cantidad_instancias = 0;
	pointer = 1;
	instancia_id = 0;

}

void iniciar_hilos(void) {
	pthread_mutex_init(&sem_socket_operaciones_coordi, NULL);
	pthread_mutex_init(&sem_instancias, NULL);
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

void cargar_configuracion(char** argv) {
	t_config* config = config_create(argv[1]);

	PUERTO_COORDINADOR = config_get_string_value(config, "PUERTO_COORDINADOR");
	log_info(logger, "Puerto Coordinador: %s", PUERTO_COORDINADOR);

	char* algoritmo = config_get_string_value(config, "ALGORITMO_DISTRIBUCION");
	ALGORITMO_DISTRIBUCION = dame_algoritmo(algoritmo);
	log_info(logger, "Algoritmo de distribución: %s", algoritmo);

	CANTIDAD_ENTRADAS = config_get_int_value(config, "CANTIDAD_ENTRADAS");
	log_info(logger, "Cantidad de entradas: %i", CANTIDAD_ENTRADAS);

	TAMANIO_ENTRADAS = config_get_int_value(config, "TAMANIO_ENTRADAS");
	log_info(logger, "Tamaño de entradas: %i", TAMANIO_ENTRADAS);

	int retardo = config_get_int_value(config, "RETARDO");
	RETARDO = dame_retardo(retardo);
	log_info(logger, "Retardo: %i (en microsegundos)", retardo);

	PUERTO_PLANIFICADOR = config_get_string_value(config,
			"PUERTO_PLANIFICADOR");
	IP_PLANIFICADOR = config_get_string_value(config, "IP_PLANIFICADOR");

	loggear("Configuración cargada.");
}

algoritmo_distribucion dame_algoritmo(char* algoritmo_src) {
	algoritmo_distribucion algoritmo_ret;

	if (strcmp(algoritmo_src, "LSU") == 0) {
		algoritmo_ret = LSU;
	}

	else if (strcmp(algoritmo_src, "EL") == 0) {
		algoritmo_ret = EL;
	}

	else if (strcmp(algoritmo_src, "KE") == 0) {
		algoritmo_ret = KE;
	}

	return algoritmo_ret;
}

float dame_retardo(int retardo_int) {
	float ret_val = (float) retardo_int;
	ret_val = ret_val / 1000;

	return ret_val;
}

int manejar_cliente(int server_socket, int socketCliente, package_int id) {

	log_info(logger, "Esperando cliente...");

	listen(server_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(server_socket, (struct sockaddr *) &addr, &addrlen);

	log_info(logger, "Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	package_int id_cliente = { .packed = -1 };

	id_cliente = recv_packed_no_exit(socketCliente);

	log_info(logger, "Mensaje recibido exitosamente. Identificando cliente...");
	identificar_cliente(id_cliente, socketCliente);

	loggear("Enviando id al cliente.");

	send_packed_no_exit(id, socketCliente);

	log_info(logger, "Handshake realizado correctamente.");

	return socketCliente;
}

void identificar_cliente(package_int id, int socket_cliente) {
	if (id.packed == 1) {
		log_info(logger, mensajePlanificador);
		pthread_create(&hilo_planificador, NULL, atender_Planificador,
				(void *) (intptr_t) socket_cliente);
		pthread_detach(hilo_planificador);
	}

	else if (id.packed == 2) {
		log_info(logger, mensajeESI);
		pthread_create(&hilo_ESI, NULL, atender_ESI,
				(void *) (intptr_t) socket_cliente);
		pthread_detach(hilo_ESI);
	}

	else if (id.packed == 3) {
		log_info(logger, mensajeInstancia);
		pthread_create(&hilo_instancia, NULL, atender_instancia,
				(void *) (intptr_t) socket_cliente);
		pthread_detach(hilo_instancia);
	}

	else {
		salir_con_error("Cliente desconocido, cerrando conexion.",
				socket_cliente);
	}

	return;
}

void* atender_ESI(void* un_socket) {
	int socket_cliente = (intptr_t) un_socket;

	loggear("Hilo de ESI inicializado correctamente.");

	int status = 1;

	uint32_t id = decimeID(socket_cliente);

	if (id == -1) {
		abortar_ESI(socket_cliente);
		return NULL;
	}

	if (id <= 0) {
		close(socket_cliente);
		log_error(logger, "ESI con id inválido (0).");
	}

	send_OK(socket_cliente);

	while (status) {

		status = chequear_solicitud(socket_cliente, id);
	}

	log_trace(logger, "Hilo de ESI número %i terminado", id);

	return NULL;
}

void send_OK(int sockfd) {
	package_int package_ok = { .packed = 42 };

	send_packed_no_exit(package_ok, sockfd);
}

uint32_t decimeID(int sockfd) {
	aviso_con_ID aviso_id = recv_aviso_no_exit(sockfd);

	log_debug(logger, "Aviso: %i", aviso_id.aviso);
	log_debug(logger, "ID: %i", aviso_id.id);

	if (aviso_id.aviso != 2 || aviso_id.aviso == aviso_recv_error.aviso) {
		log_warning(logger, "Mensaje erróneo. Abortando ESI.");
		return -1;
	}

	log_debug(logger, "ID recibido: %i", aviso_id.id);

	return aviso_id.id;
}

void liberar_claves(uint32_t id) {

	if (claves_bloqueadas.head == NULL) {
		loggear("No hay claves bloqueadas");
		return;
	}

	t_clave_node* puntero = claves_bloqueadas.head;
	t_clave_node* aux = puntero->sgte;

	while (puntero != NULL) {
		log_debug(logger, "Blocker: %i", puntero->block_id);
		log_debug(logger, "ESI: %i", id);
		if (puntero->block_id == id) {
			log_debug(logger, "Bloqueando...");
			desbloquear(puntero->clave);
			puntero = aux;
			if (puntero != NULL) {
				aux = puntero->sgte;
			}
		} else {
			puntero = puntero->sgte;
			if (puntero != NULL) {
				aux = puntero->sgte;
			} else {
				aux = NULL;
			}
		}
	}
}

int chequear_solicitud(int socket_cliente, uint32_t id) {
	aviso_con_ID aviso_cliente = recv_aviso_no_exit(socket_cliente);

	log_debug(logger, "%i", aviso_cliente.aviso);

	int status;

	if (aviso_cliente.aviso == 0) {
		log_info(logger, "Fin de ESI.");
		if (aviso_cliente.id == id) {
			liberar_claves(id);
		}
		return 0;
	}

	else if (aviso_cliente.aviso == 1) {
		log_info(logger, "Ejecución de ESI.");
	}

	else if (aviso_cliente.aviso == 11) {
		log_debug(logger, "%i", aviso_cliente.id);
		status = get(socket_cliente, aviso_cliente.id);

	}

	else if (aviso_cliente.aviso == 12) {
		log_debug(logger, "%i", aviso_cliente.id);
		status = set(socket_cliente, aviso_cliente.id);

	}

	else if (aviso_cliente.aviso == 13) {
		log_debug(logger, "%i", aviso_cliente.id);
		status = store(socket_cliente, aviso_cliente.id);

	}

	else {
		log_warning(logger, "Mensaje erróneo. Abortando ESI.");
		abortar_ESI(socket_cliente);

		liberar_claves(id);

		return 0;
	}

	if (status == -5) {
		abortar_ESI(socket_cliente);
		liberar_claves(id);
		return 0;
	}

	return 1;
}

int get(int socket_cliente, uint32_t id) {
	aviso_con_ID aviso_ok = { .aviso = 10 };

	sleep(RETARDO);

	send_aviso_no_exit(aviso_ok, socket_cliente);

	package_int size_packed = recv_packed_no_exit(socket_cliente);
	uint32_t clave_size = size_packed.packed;

	char* clave = recv_string_no_exit(socket_cliente, clave_size);

	package_int response;

	response.packed = dame_response(clave, id);

	log_debug(logger, "%i", response.packed);

	send_packed_no_exit(response, socket_cliente);

	return 20;
}

int dame_response(char* clave, uint32_t id) {
	if (!existe(clave)) {
		bloquear(clave, id);
		loggear("Get exitoso.");

		operacion op = { .op_type = op_GET, .id = id };
		strcpy(op.clave, clave);

		log_op(op);

		return 20;
	}

	else if (existe(clave) && !esta_bloqueada(clave)) {
		bloquear(clave, id);
		loggear("Get exitoso.");

		operacion op = { .op_type = op_GET, .id = id };
		strcpy(op.clave, clave);
		log_op(op);

		return 20;
	}

	else {
		log_warning(logger, "Bloqueando ESI %i.", id);
		bloquear_ESI(clave, id);
		return 5;
	}
}

int set(int socket_cliente, uint32_t id) {
	aviso_con_ID aviso_ok = { .aviso = 10 };

	sleep(RETARDO);

	send_aviso_no_exit(aviso_ok, socket_cliente);

	package_int clave_size_packed = recv_packed_no_exit(socket_cliente);
	uint32_t clave_size = clave_size_packed.packed;
	char* clave = recv_string_no_exit(socket_cliente, clave_size);

	package_int valor_size_packed = recv_packed_no_exit(socket_cliente);
	uint32_t valor_size = valor_size_packed.packed;
	char* valor = recv_string_no_exit(socket_cliente, valor_size);

	package_int response;

	response.packed = settear(valor, clave, id);

	if (response.packed == -3) {
		return -5;
	}

	log_debug(logger, "%i", response.packed);

	send_packed_no_exit(response, socket_cliente);

	return 20;
}

int store(int socket_cliente, uint32_t id) {
	aviso_con_ID aviso_ok = { .aviso = 10 };

	sleep(RETARDO);

	send_aviso_no_exit(aviso_ok, socket_cliente);

	package_int size_packed = recv_packed_no_exit(socket_cliente);
	uint32_t clave_size = size_packed.packed;

	char* clave = recv_string_no_exit(socket_cliente, clave_size);

	package_int response;
	response.packed = get_packed(clave, id);

	send_packed_no_exit(response, socket_cliente);

	if (response.packed == -3) {
		return -5;
	}

	if (response.packed != 5) {
		int status = hacer_store(clave);

		if (status == -1) {
			return -5;
		}

		if (status == 5) {
			abortar_ESI(socket_cliente);
		}

		if (status == -7) {
			abortar_ESI(socket_cliente);
			//desconectar instancia?
		}

		if (!emptyBlocked(&blocked_ESIs)) {
			aviso_con_ID unlock = { .aviso = 28, .id = dame_desbloqueado(clave,
					blocked_ESIs) };
			liberar_ESI(&blocked_ESIs, unlock.id);
			enviar_aviso(socket_planificador, unlock);

		}

		operacion op = { .op_type = op_STORE, .id = id };
		strcpy(op.clave, clave);

		log_op(op);

	}

	sleep(2);

	log_debug(logger, "%i", response.packed);

	return 20;
}

int settear(char* valor, char* clave, uint32_t id) {
	t_clave_node* puntero = claves_bloqueadas.head;

	if (!existe(clave)) {
		return -3;
	}

	if (!esta_bloqueada(clave)) {
		log_warning(logger, "Abortando ESI %i.", id);
		return -3;
	}

	while (puntero != NULL) {

		if (strcmp(puntero->clave, clave) == 0) {
			if (puntero->block_id != id) {
				log_debug(logger, "%i %i", puntero->block_id, id);

				log_warning(logger, "Abortando ESI %i.", id);
				return -3;
			}

			operacion op = { .op_type = op_SET, .id = id };
			strcpy(op.clave, clave);
			strcpy(op.valor, valor);

			log_op(op);

			log_info(logger, "SET %s %s", clave, valor);
			int status = do_set(valor, clave);

			if (status == -1) {
				return -3;
			}

			return 20;
		}
		puntero = puntero->sgte;
	}

	return -3;
}

void actualizarClave(char* clave, char* valor) {
	t_clave_node* puntero = claves_bloqueadas.head;

	while (puntero != NULL) {
		log_debug(logger, "Clave en la lista: %s", puntero->clave);
		log_debug(logger, "Mi clave: %s", clave);
		if (mismoString(puntero->clave, clave)) {
			strcpy(puntero->valor, valor);

			return;
		}

		puntero = puntero->sgte;
	}
}

int do_set(char* valor, char* clave) {
	uint32_t valor_size = (uint32_t) strlen(valor);
	uint32_t clave_size = (uint32_t) strlen(clave);

	valor_set.tamanio_clave = clave_size;
	valor_set.tamanio_valor = valor_size;
	valor_set.clave = clave;
	valor_set.valor = valor;

	Instancia instancia = getInstanciaSet(clave);

	if (mismoString(instancia.nombre, inst_error.nombre)) {
		return -1;
	}
	actualizarInstancia(instancia, clave);
	actualizarClave(clave, valor);

	int tamanio_parametros_set = 2 * sizeof(uint32_t) + valor_set.tamanio_clave
			+ valor_set.tamanio_valor;

	log_trace(logger, "CLAVE: %d VALOR: %d TAMANIO_PARAMETROS: %d", clave_size,
			valor_size, tamanio_parametros_set);

	pthread_mutex_lock(&sem_socket_operaciones_coordi);

	enviar_orden_instancia(tamanio_parametros_set,
			(void*) (intptr_t) instancia.sockfd, 11);
	enviar_valores_set(tamanio_parametros_set,
			(void*) (intptr_t) instancia.sockfd);

	log_debug(logger, "Esperando confirmacion...");

	esperar_confirmacion_de_exito((int) instancia.sockfd);

	package_int cantidad_entradas_ocupadas_instancia;

	cantidad_entradas_ocupadas_instancia = recibir_packed(instancia.sockfd);

	log_debug(logger, "Cantidad de entradas ocupadas por la instancia %s: %i",
			instancia.nombre, cantidad_entradas_ocupadas_instancia.packed);

	log_debug(logger, "%s tiene la clave %s", instancia.nombre, clave);

	actualizarEntradas(instancia, cantidad_entradas_ocupadas_instancia.packed);

	enviar_orden_instancia(0, (void*) (intptr_t) instancia.sockfd, 15);

	log_debug(logger, "Esperando confirmacion...");

	esperar_confirmacion_de_exito((int) instancia.sockfd);

	pthread_mutex_unlock(&sem_socket_operaciones_coordi);

	return 1;
}

void actualizarInstancia(Instancia instancia, char* clave) {
	t_instancia_node* puntero = instancias.head;

	while (puntero != NULL) {
		if (mismoString(puntero->instancia.nombre, instancia.nombre)) {
			t_clave_list claves = puntero->instancia.claves;
			agregar_clave(&claves, clave, 42);
			puntero->instancia.claves = claves;
		}

		puntero = puntero->sgte;
	}
}

Instancia elQueLaTiene(char* clave) {
	t_instancia_node* puntero = instancias.head;

	while (puntero != NULL) {
		if (tieneLaClave(puntero->instancia, clave)) {
			return puntero->instancia;
		}

		puntero = puntero->sgte;
	}

	return inst_error;
}

bool estaAsignada(char* clave) {
	t_instancia_node* puntero = instancias.head;

	while (puntero != NULL) {
		if (tieneLaClave(puntero->instancia, clave)) {
			return true;
		}

		puntero = puntero->sgte;
	}

	return false;
}

Instancia getInstanciaSet(char* clave) {
	Instancia ret_inst;

	switch (ALGORITMO_DISTRIBUCION) {
	case EL:
		ret_inst = equitativeLoad();
		break;

	case LSU:
		ret_inst = leastSpaceUsed();
		break;
	case KE:
		ret_inst = keyExplicit(clave);
		break;
	default:
		log_warning(logger, "Fallo en el algoritmo");
		break;
	}

	return ret_inst;
}

void avanzar_puntero(void) {
	pointer++;

	pthread_mutex_lock(&sem_instancias);
	if (pointer > cantidad_instancias) {
		pointer = 1;
	}
	pthread_mutex_unlock(&sem_instancias);
}

Instancia equitativeLoad(void) {
	if (instancias.head == NULL) {
		log_warning(logger,
				"No hay ninguna instancia disponible para poder despachar el pedido.");
		return inst_error;
	}

	Instancia ret_inst;
	t_instancia_node* puntero = instancias.head;

	while (puntero != NULL) {
		if (puntero->index == pointer) {
			if (puntero->instancia.disponible) {
				ret_inst = puntero->instancia;
			} else {
				puntero = instancias.head;
				avanzar_puntero();
				return equitativeLoad();
			}
		}

		puntero = puntero->sgte;
	}

	avanzar_puntero();
	return ret_inst;
}

Instancia leastSpaceUsed(void) {
	t_instancia_node* puntero = instancias.head;
	Instancia instancia = headInstancias(instancias);

	while (puntero != NULL) {
		if (tieneMenosEspacio(puntero->instancia, instancia)
				&& puntero->instancia.disponible) {
			instancia = puntero->instancia;
		}

		puntero = puntero->sgte;
	}

	return instancia;
}

Instancia keyExplicit(char* clave) {
	if (!isalnum(clave[0])) {
		return inst_error;
	}

	char firstChar = tolower(clave[0]);

	t_instancia_node* puntero = instancias.head;

	while (puntero != NULL) {
		if (leCorresponde(puntero->instancia, firstChar)) {
			return puntero->instancia;
		}

		puntero = puntero->sgte;
	}

	return inst_error;
}

bool leCorresponde(Instancia instancia, char caracter) {
	return instancia.keyMin <= caracter && instancia.keyMax >= caracter;
}

int get_packed(char* clave, uint32_t id) {
	if (!existe(clave)) {
		log_warning(logger, "Abortando ESI %i.", id);
		return -3;
	}

	else {
		uint32_t blocker = get_clave_id(clave);
		log_trace(logger, "Blocker %i", blocker);
		log_trace(logger, "Solicitante %i", id);

		if (blocker == -1) {
			log_warning(logger, "Abortando ESI %i.", id);
			return -3;
		}

		if (blocker != id) {
			log_warning(logger, "Abortando ESI %i.", id);
			return -3;
		}

		else if (!esta_bloqueada(clave)) {
			log_warning(logger, "Abortando ESI %i.", id);
			return -3;
		}

		else {
			desbloquear(clave);
			log_info(logger, "STORE %s.", clave);
			return 20;
		}
	}
}

void mostrar_listas() {
	t_clave_node * puntero_bloqueadas = claves_bloqueadas.head;
	loggear("Bloqueadas:");
	while (puntero_bloqueadas != NULL) {
		log_trace(logger, "Clave: %s", puntero_bloqueadas->clave);
		puntero_bloqueadas = puntero_bloqueadas->sgte;
	}
	t_clave_node * puntero_disponibles = claves_disponibles.head;
	loggear("Disponibles:");
	while (puntero_disponibles != NULL) {
		log_trace(logger, "Clave: %s", puntero_disponibles->clave);
		puntero_disponibles = puntero_disponibles->sgte;
	}
	sleep(1);
}

bool tieneLaClave(Instancia unaInstancia, char* clave) {
	t_clave_list claves = unaInstancia.claves;
	t_clave_node* puntero = claves.head;

	while (puntero != NULL) {
		if (mismoString(puntero->clave, clave)) {
			return true;
		}

		puntero = puntero->sgte;
	}

	return false;
}

Instancia getInstanciaStore(char* clave) {
	t_instancia_node* puntero = instancias.head;

	while (puntero != NULL) {
		if (tieneLaClave(puntero->instancia, clave)) {
			log_debug(logger, "%s tiene la clave %s", puntero->instancia.nombre,
					clave);
			return puntero->instancia;
		}

		puntero = puntero->sgte;
	}

	return inst_error;
}

int hacer_store(char* clave) {
	Instancia instancia = getInstanciaStore(clave);
	int sockfd = instancia.sockfd;

	if (mismoString(instancia.nombre, inst_error.nombre)) {
		log_warning(logger, "No se encontró una instancia que posea la clave.");
		return -1;
	}

	if (!ping(instancia)) {
		log_warning(logger,
				"La instancia que posee la clave se encuentra desconectada.");
		desconectar(instancia);
		return -1;
	}

	log_trace(logger, "La instancia %s tiene la clave %s", instancia.nombre,
			clave);

	pthread_mutex_lock(&sem_socket_operaciones_coordi);

	enviar_orden_instancia(0, (void*) (intptr_t) sockfd, 12);

	uint32_t clave_size = (uint32_t) strlen(clave) + 1;

	log_warning(logger, "Clave size %i", clave_size);

	package_int package_size = { .packed = clave_size };

	int status;

	status = send_package_int(package_size, sockfd);
	status = send_string(clave, sockfd);

	log_debug(logger, "Esperando confirmacion...");

	int res = esperar_confirmacion_de_exito((int) sockfd);

	if (res == -5) {
		return -5;
	}

	if (res == -7) {
		return -7;
	}

	pthread_mutex_unlock(&sem_socket_operaciones_coordi);

	if (status == 0) {
		log_warning(logger, "Falló el store.");
		return -1;
	}

	return 1;
}

void desconectar(Instancia instancia) {
	t_instancia_node* puntero = instancias.head;

	while (puntero != NULL) {
		if (mismoString(puntero->instancia.nombre, instancia.nombre)) {
			puntero->instancia.disponible = false;
			log_trace(logger, "%s desconectada.", instancia.nombre);
		}
		puntero = puntero->sgte;
	}

}

void bloquear_ESI(char* clave, uint32_t id) {
	blocked bloqueado = { .id = id };
	strcpy(bloqueado.clave, clave);

	log_debug(logger, "Clave: %s", clave);
	log_debug(logger, "ID: %i", id);

	agregar_blocked(&blocked_ESIs, bloqueado);
}

void liberar_ESI(t_blocked_list* lista, uint32_t id) {
	if (id != -5) {
		eliminar_blocked(lista, id);
	}
}

uint32_t get_clave_id(char* clave) {
	t_clave_node* puntero = claves_bloqueadas.head;

	while (puntero != NULL) {
		if (strcmp(puntero->clave, clave) == 0) {
			return puntero->block_id;
		}

		puntero = puntero->sgte;
	}

	return -1;
}

void conseguirBloqueados(int sockfd) {
	package_int size_package = recibir_packed(sockfd);
	char* clave = recibir_cadena(sockfd, size_package.packed);

	char* bloqueados = getBloqueados(clave);
	loggear(bloqueados);

	char* dup_bloqueados = strdup(bloqueados);

	aviso_con_ID aviso_bloqueados = { .aviso = 71 };

	enviar_aviso(sockfd, aviso_bloqueados);

	enviar_packed(size_package, sockfd);
	enviar_cadena(dup_bloqueados, sockfd);

	free(dup_bloqueados);

	if (flag_free_asignada) {
		free(bloqueados);
		flag_free_asignada = false;
	}
}

void* atender_Planificador(void* un_socket) {
	socket_planificador = (intptr_t) un_socket;

	loggear("Hilo de planificador inicializado correctamente.");

	aviso_con_ID aviso_plani;

	while (1) {
		aviso_plani = recibir_aviso(socket_planificador);

		log_debug(logger, "%i", aviso_plani.aviso);

		if (aviso_plani.aviso == 0) {
			log_info(logger,
					"Fin de Planificador. Cerrando sesión y terminando.");
			exit(42);
			break;
		}

		else if (aviso_plani.aviso == 25) {
			bloquear_clave(socket_planificador);
		}

		else if (aviso_plani.aviso == 27) {
			desbloquear_clave(socket_planificador);
		}

		else if (aviso_plani.aviso == 60) {
			status(socket_planificador);
		}

		else if (aviso_plani.aviso == 62) {
			comunicarDeadlock(socket_planificador);
		}

		else if (aviso_plani.aviso == 70) {
			bloquearSegunClave(socket_planificador);
		}

		else if (aviso_plani.aviso == 71) {
			conseguirBloqueados(socket_planificador);
		}

		else {
			log_warning(logger,
					"Error de mensaje con el planificador. Cierro el socket.");
			close(socket_planificador);
			break;
		}
	}

	seguir_ejecucion = 0;

	return NULL;
}

void bloquearSegunClave(int sockfd) {
	package_int id_package = recibir_packed(sockfd);

	package_int size_package = recibir_packed(sockfd);
	char* string = recibir_cadena(sockfd, size_package.packed);

	bloquear_ESI(string, id_package.packed);

	log_warning(logger, "El ESI %i fue bloqueado tras la clave %s",
			id_package.packed, string);
}

bool esta(char* clave, t_clave_list claves) {
	t_clave_node* puntero = claves.head;

	while (puntero != NULL) {
		if (mismoString(puntero->clave, clave)) {
			return true;
		}

		puntero = puntero->sgte;
	}

	return false;
}

char* getValor(char* recurso) {
	if (!existe(recurso)) {
		return "La clave no existe.";
	}

	t_clave_node* puntero = claves_bloqueadas.head;

	while (puntero != NULL) {
		log_debug(logger, "Clave: %s", puntero->clave);
		log_debug(logger, "Valor: %s", puntero->valor);

		if (mismoString(puntero->clave, recurso)) {
			if (puntero->valor == NULL) {
				return "No tiene valor.";
			}

			return puntero->valor;
		}

		puntero = puntero->sgte;
	}

	return "No tiene valor asignado.";
}

Instancia correspondiente(t_instancia_list lista, char* clave) {
	t_instancia_node* puntero = lista.head;

	while (puntero != NULL) {
		if (leCorresponde(puntero->instancia, clave[0])) {
			return puntero->instancia;
		}

		puntero = puntero->sgte;
	}

	return inst_error;
}

bool tieneMenosEspacio(Instancia unaInstancia, Instancia otraInstancia) {
	return unaInstancia.espacio_usado < otraInstancia.espacio_usado;
}

char* getNombrePotencial(char* recurso) {
	Instancia ret_inst;
	switch (ALGORITMO_DISTRIBUCION) {
	case EL:
		ret_inst = headInstancias(instancias);
		break;
	case KE:
		ret_inst = correspondiente(instancias, recurso);
		break;
	case LSU:
		ret_inst = leastSpaceUsed();
		break;
	default:
		ret_inst = inst_error;
		salir_con_error("Fallo en el algoritmo.", 0);
		break;
	}

	if (mismoString(ret_inst.nombre, inst_error.nombre)) {
		salir_con_error("Falló la asignación potencial de clave", 0);
	}

	return strdup(ret_inst.nombre);
}

char* getInstancia(char* recurso) {
	char posible[255] = "(posible asignada en la proxima) ";

	if (!existe(recurso)) {
		return "La clave no existe.";
	}

	if (!estaAsignada(recurso)) {
		loggear("No está asignada.");
		char* nombre = getNombrePotencial(recurso);

		strcat(posible, nombre);

		char* ret_string = strdup(posible);
		//Esto hace malloc y hay que hacerle free después

		flag_free_asignada = true;
		return ret_string;
	}

	Instancia asignada = elQueLaTiene(recurso);
	return strdup(asignada.nombre);
}

char* getBloqueados(char* recurso) {
	char* dup_recurso = strdup(recurso);

	if (!existe(dup_recurso)) {
		return "La clave no existe.";
		free(dup_recurso);

	}

	t_blocked_node* puntero = blocked_ESIs.head;

	char bloqueados[255] = "";
	int i = 0;

	while (puntero != NULL) {
		if (mismoString(puntero->clave, dup_recurso)) {
			char* num = string_itoa(puntero->id);
			bloqueados[i] = num[0];
			i++;
			bloqueados[i] = ',';
			i++;
		}

		puntero = puntero->sgte;
	}

	char* dup = strdup(bloqueados);
	flag_free_asignada = true;
	free(dup_recurso);
	return dup;
}

void status(int sockfd) {
	package_int string_size = recibir_packed(sockfd);
	char* recurso = recibir_cadena(sockfd, string_size.packed);

	char* valor = getValor(recurso);
	char* instancia = getInstancia(recurso);
	char* bloqueados = getBloqueados(recurso);

	char* dup_valor = strdup(valor);
	char* dup_instancia = strdup(instancia);
	char* dup_bloqueados = strdup(bloqueados);

	log_debug(logger, "Valor: %s", dup_valor);
	log_debug(logger, "Instancia: %s", dup_instancia);
	log_debug(logger, "Bloqueados: %s", dup_bloqueados);

	uint32_t valor_length = (uint32_t) strlen(dup_valor) + 1;
	uint32_t instancia_length = (uint32_t) strlen(dup_instancia) + 1;
	uint32_t bloqueados_length = (uint32_t) strlen(dup_bloqueados) + 1;

	package_int valor_size = { .packed = valor_length };

	package_int instancia_size = { .packed = instancia_length };

	package_int bloqueados_size = { .packed = bloqueados_length };

	aviso_con_ID aviso_status = { .aviso = 61 };

	enviar_aviso(sockfd, aviso_status);

	enviar_packed(valor_size, sockfd);
	enviar_cadena(dup_valor, sockfd);

	sleep(1);
	enviar_packed(instancia_size, sockfd);
	enviar_cadena(dup_instancia, sockfd);

	sleep(1);
	enviar_packed(bloqueados_size, sockfd);
	enviar_cadena(dup_bloqueados, sockfd);

	free(dup_valor);
	free(dup_instancia);
	free(dup_bloqueados);
	if (flag_free_asignada) {
		free(instancia);
		flag_free_asignada = false;
	}
}

void desbloquear_clave(int socket_cliente) {
	aviso_con_ID aviso_ok = { .aviso = 27 };

	package_int size_package = { .packed = -1 };

	aviso_con_ID unlock_ok = { .aviso = 28 };

	enviar_aviso(socket_cliente, aviso_ok);

	size_package = recibir_packed(socket_cliente);
	char* clave = recibir_cadena(socket_cliente, size_package.packed);

	desbloquear(clave);

	unlock_ok.id = dame_desbloqueado(clave, blocked_ESIs);

	if (unlock_ok.id != -5) {
		liberar_ESI(&blocked_ESIs, unlock_ok.id);
	}

	enviar_aviso(socket_cliente, unlock_ok);

}

uint32_t dame_desbloqueado(char* clave, t_blocked_list lista) {
	char* dup_clave = strdup(clave);
	t_blocked_node* puntero = lista.head;

	while (puntero != NULL) {
		if (strcmp(dup_clave, puntero->clave) == 0) {
			free(dup_clave);
			return puntero->id;
		}

		puntero = puntero->sgte;
	}

	free(dup_clave);

	return -5;
}

void desbloquear(char* clave) {
	char* dup_clave = strdup(clave);
	if (!existe(dup_clave)) {
		crear(dup_clave);
		desbloquear(dup_clave);
	}

	else if (existe(dup_clave) && esta_bloqueada(dup_clave)) {
		mostrar_listas();
		eliminar_clave(&claves_bloqueadas, dup_clave);
		agregar_clave(&claves_disponibles, dup_clave, -1);

		log_info(logger, "La clave %s fue desbloqueada.", dup_clave);
	}

	free(dup_clave);
}

void bloquear_clave(int socket_cliente) {
	aviso_con_ID aviso_ok = { .aviso = 25 };

	package_int block_ok = { .packed = 26 };

	package_int size_package = { .packed = -1 };

	enviar_aviso(socket_cliente, aviso_ok);

	size_package = recibir_packed(socket_cliente);
	log_debug(logger, "%i", size_package.packed);
	char* clave = recibir_cadena(socket_cliente, size_package.packed);

	log_debug(logger, "%s", clave);

	bloquear(clave, 0);

	enviar_packed(block_ok, socket_cliente);

}

void bloquear(char* clave, uint32_t id) {
	if (!existe(clave)) {
		crear(clave);
		bloquear(clave, id);
	}

	else if (existe(clave) && !esta_bloqueada(clave)) {
		eliminar_clave(&claves_disponibles, clave);
		agregar_clave(&claves_bloqueadas, clave, id);

		log_info(logger, "La clave %s fue bloqueada por %i (0 indica usuario).",
				clave, id);
	}

}

bool esta_bloqueada(char* clave) {
	t_clave_node* puntero = claves_bloqueadas.head;

	while (puntero != NULL) {
		if (strcmp(puntero->clave, clave) == 0) {
			return true;
		}

		puntero = puntero->sgte;
	}

	return false;
}

bool existe(char* clave) {
	t_clave_node* puntero = claves_bloqueadas.head;

	while (puntero != NULL) {
		if (strcmp(puntero->clave, clave) == 0) {
			return true;
		}

		puntero = puntero->sgte;
	}

	puntero = claves_disponibles.head;

	while (puntero != NULL) {
		if (strcmp(puntero->clave, clave) == 0) {
			return true;
		}

		puntero = puntero->sgte;
	}

	return false;
}

void crear(char* clave) {
	agregar_clave(&claves_disponibles, clave, -1);
}

void* atender_instancia(void* un_socket) {

	int sockfd = (intptr_t) un_socket;

	loggear("Hilo de instancia inicializado correctamente.");

	asignar_entradas(sockfd);

	char* name = get_name(sockfd);

	if (mismoString(string_recv_error, name)) {
		log_warning(logger, "Falló la conexión de la instancia");
		return NULL;
	}

	log_trace(logger, "Nombre: %s", name);

	levantar_instancia(name, sockfd);

	log_warning(logger, "Hilo de instancia %s terminado.", name);

	return NULL;
}

void levantar_instancia(char* name, int sockfd) {
	if (murio(name, sockfd)) {
		revivir(name, sockfd);
		return;
	}

	Instancia instancia = { .sockfd = sockfd, .disponible =
	true, .veces_llamado = 0, .espacio_usado = 0, .id = instancia_id };
	strcpy(instancia.nombre, name);

	loggear("Instancia agregada correctamente");

	pthread_mutex_lock(&sem_instancias);
	agregar_instancia(&instancias, instancia, cantidad_instancias + 1);

	cantidad_instancias++;
	pthread_mutex_unlock(&sem_instancias);

	redistribuir_claves();

}

void redistribuir_claves(void) {
	if (ALGORITMO_DISTRIBUCION == KE) {
		int disponibles = instanciasDisponibles();
		log_debug(logger, "Instancias disponibles: %i", disponibles);

		int contador = 0;

		t_instancia_node* puntero = instancias.head;

		while (puntero != NULL) {
			if (puntero->instancia.disponible) {
				asignarKeyMinMax(&(puntero->instancia), contador, disponibles);

				log_debug(logger, "%s tiene min %c y max %c",
						puntero->instancia.nombre, puntero->instancia.keyMin,
						puntero->instancia.keyMax);
				contador++;
			}

			puntero = puntero->sgte;
		}

	}
}

int getParteEntera(double x) {
	return (int) x;
}

double getParteFraccional(double x) {
	return x - getParteEntera(x);
}

int redondearDivision(double x, double y) {
	double division = x / y;
	int division_as_int = getParteEntera(division);

	if (getParteFraccional(division) >= 0.5) {
		return division_as_int + 1;
	} else {
		return division_as_int;
	}
}

void asignarKeyMinMax(Instancia* instancia, int posicion,
		int totalDeDisponibles) {

	int cantidadDeLetras = strlen(abecedario);
	int asignaciones = redondearDivision((double) cantidadDeLetras,
			(double) totalDeDisponibles);

	int offset = posicion * asignaciones;

	instancia->keyMin = abecedario[offset];

	if (offset + asignaciones >= cantidadDeLetras) {
		instancia->keyMax = 'z';
		return;
	} else {
		instancia->keyMax = abecedario[offset + asignaciones - 1];
	}

	if (posicion == totalDeDisponibles - 1) {
		instancia->keyMax = 'z';
		return;
	}
}

bool murio(char* name, int sockfd) {
	t_instancia_node* puntero = instancias.head;

	while (puntero != NULL) {
		if (mismoString(name, puntero->instancia.nombre)) {
			if (puntero->instancia.disponible) {
				if (!ping(puntero->instancia)) {
					log_warning(logger,
							"Esta instancia se encontraba en el sistema pero se cayó. Reincorporando");
					return true;
				}

				else {
					log_warning(logger,
							"Esta instancia todavía se encuentra en el sistema. Abortando conexión.");
					terminar_conexion(sockfd, false);
					close(sockfd);
					return false;
				}
			}

		}

		puntero = puntero->sgte;
	}

	loggear("Esta instancia no se encontraba en el sistema");
	return false;
}

bool ping(Instancia instancia) {
	pthread_mutex_lock(&sem_socket_operaciones_coordi);

	orden_del_coordinador orden;
	orden.codigo_operacion = 100;
	orden.tamanio_a_enviar = 0;

	uint32_t packageSize = sizeof(orden_del_coordinador);

	orden_del_coordinador* buffer = malloc(packageSize);

	memcpy(buffer, &orden, packageSize);

	log_trace(logger, "Pingeando instancia... SOCKET: %i", instancia.sockfd);

	int envio = send(instancia.sockfd, buffer, packageSize, MSG_NOSIGNAL);

	log_debug(logger, "%i", envio);

	if (envio < 0) {
		log_warning(logger, "Ping fallido. Instancia no disponible: %s",
				strerror(errno));
		instancia.disponible = false;
		free(buffer);

		pthread_mutex_unlock(&sem_socket_operaciones_coordi);

		return false;
	}

	if (!recv_ping(instancia.sockfd)) {
		log_warning(logger,
				"Falló el recibo del ping. Instancia no disponible: %s",
				strerror(errno));
		instancia.disponible = false;
		free(buffer);

		pthread_mutex_unlock(&sem_socket_operaciones_coordi);

		return false;
	}

	pthread_mutex_unlock(&sem_socket_operaciones_coordi);

	loggear("Ping ok, la instancia sigue en pie.");
	free(buffer);

	return true;
}

bool recv_ping(int sockfd) {
	package_int ping = recv_packed_no_exit(sockfd);

	log_debug(logger, "%i", ping.packed);

	if (ping.packed != 100) {
		return false;
	}

	return true;

}

void revivir(char* name, int sockfd) {
	update(name, sockfd);
	t_clave_list claves = get_claves(name);

	if (claves.head == NULL) {
		log_warning(logger, "%s no tenía ninguna clave asignada.", name);
		return;
	}

	enviar_claves(claves, sockfd);
}

void enviar_claves(t_clave_list claves, int sockfd) {
	t_clave_node* puntero = claves.head;

	pthread_mutex_lock(&sem_socket_operaciones_coordi);

	enviar_orden_instancia(0, (void*) (intptr_t) sockfd, 50);

	while (puntero != NULL) {
		int size = strlen(puntero->clave) + 1;
		package_int size_package = { .packed = size };

		log_debug(logger, "Clave: %s", puntero->clave);

		send_packed_no_exit(size_package, sockfd);
		send_string_no_exit(puntero->clave, sockfd);

		package_int ok = recv_packed_no_exit(sockfd);

		esperar_confirmacion_de_exito(sockfd);

		puntero = puntero->sgte;

		if (puntero == NULL) {
			size_package.packed = 60;
			send_packed_no_exit(size_package, sockfd);
		} else {
			size_package.packed = 61;
			send_packed_no_exit(size_package, sockfd);
		}

		if (ok.packed != 51) {
			log_warning(logger, "ERROR EN EL ENVÍO DE MENSAJES: %s",
					strerror(errno));
			close(sockfd);
			pthread_mutex_unlock(&sem_socket_operaciones_coordi);
			return;
		}
	}
	pthread_mutex_unlock(&sem_socket_operaciones_coordi);

	esperar_confirmacion_de_exito(sockfd);
}

t_clave_list get_claves(char* name) {
	t_instancia_node* puntero;
	puntero = instancias.head;
	while (puntero != NULL) {
		if (mismoString(puntero->instancia.nombre, name)) {
			return puntero->instancia.claves;
		}

		puntero = puntero->sgte;
	}

	t_clave_list error_list = { .head = NULL };

	return error_list;
}

void update(char* name, int sockfd) {
	t_instancia_node* puntero = instancias.head;

	while (puntero != NULL) {

		if (mismoString(puntero->instancia.nombre, name)) {
			puntero->instancia.disponible = true;
			log_debug(logger, "Actualizando socket de la instancia...");
			puntero->instancia.sockfd = sockfd;
		}

		puntero = puntero->sgte;
	}

	log_trace(logger, "%s actualizada correctamente.", name);
}

char* get_name(int sockfd) {
	send_orden_no_exit(40, sockfd);

	loggear("Estoy por recibir tamanio nombre");
	package_int name_size = recv_packed_no_exit(sockfd);
	log_warning(logger, "TAMANIO NOMBRE: %i", name_size);
	char* name = recv_string_no_exit(sockfd, name_size.packed);
	log_warning(logger, "NOMBRE: %s", name);

	return name;

}

void abortar_ESI(int sockfd) {
	terminar_conexion(sockfd, false);
	close(sockfd);
}

void log_op(operacion op) {

	switch (op.op_type) {
	case op_GET:
		log_info(log_operaciones, "ESI %i GET %s", op.id, op.clave);
		break;
	case op_SET:
		log_info(log_operaciones, "ESI %i SET %s %s", op.id, op.clave,
				op.valor);
		break;
	case op_STORE:
		log_info(log_operaciones, "ESI %i STORE %s", op.id, op.clave);
		break;
	default:
		log_warning(logger, "Operación inválida.");
		break;
	}
}

void asignar_entradas(int sockfd) {
	log_trace(logger, "Cant entradas: %d, Tamanio_entrada: %d",
			CANTIDAD_ENTRADAS, TAMANIO_ENTRADAS);
	pthread_mutex_lock(&sem_socket_operaciones_coordi);
	enviar_orden_instancia(CANTIDAD_ENTRADAS, (void*) (intptr_t) sockfd,
			TAMANIO_ENTRADAS);

	log_debug(logger, "Esperando confirmacion...");

	esperar_confirmacion_de_exito((int) sockfd);
	pthread_mutex_unlock(&sem_socket_operaciones_coordi);

}

void enviar_orden_instancia(int tamanio_parametros_set, void* un_socket,
		int codigo_de_operacion) {

	orden_del_coordinador orden;
	orden.codigo_operacion = codigo_de_operacion;
	orden.tamanio_a_enviar = tamanio_parametros_set;

	uint32_t tamanio_orden = sizeof(orden_del_coordinador);

	//log_trace(logger, "tamanio a enviar: %d, codigo operacion: %d", orden.tamanio_a_enviar, orden.codigo_operacion);

	orden_del_coordinador * buffer_orden = malloc(tamanio_orden);

	//Serializacion de la orden

	memcpy(buffer_orden, &orden, tamanio_orden);

	loggear("Enviando orden a la instancia...");

	if (send((intptr_t) un_socket, (void*) buffer_orden, tamanio_orden,
	MSG_NOSIGNAL) < 0) {
		log_warning(logger, "Error en el envio de la orden: %s",
				strerror(errno));
		return;
	}

	loggear("Orden enviada!");

	free(buffer_orden);

}

int esperar_confirmacion_de_exito(int un_socket) {

	package_int confirmacion = recv_packed_no_exit(un_socket);

	if (confirmacion.packed == 100) {
		loggear("Comprobacion de PING finalizada con exito");
		return 0;
	} else if (confirmacion.packed == 101) {
		loggear("Pedido de compactacion recibido, mandando a compactar...");
		enviar_instancias_a_compactar();
		esperar_confirmacion_de_exito(un_socket);
		return 0;
	} else if (confirmacion.packed == 110) {
		loggear("Operacion Inicial finalizada con exito");
		return 0;
	} else if (confirmacion.packed == 111) {
		loggear("Operacion SET finalizada con exito");
		return 0;
	} else if (confirmacion.packed == 112) {
		loggear("Operacion STORE finalizada con exito");
		return 0;
	} else if (confirmacion.packed == 115) {
		loggear("Operacion Lectura finalizada con exito");
		return 0;
	} else if (confirmacion.packed == 140) {
		loggear("Nombre asignado con exito");
		return 0;
	} else if (confirmacion.packed == 666) {
		log_error(logger, "Tengo que abortar el ESI");
		return -5;
	} else {
		log_error(logger, "La instancia no pudo finalizar la operacion");
		return -7;
	}

	return -10;
}

void enviar_instancias_a_compactar() {
	t_instancia_node * nodo_aux = instancias.head;

	while (nodo_aux != NULL) {
		log_debug(logger, "Enviando instancia a compactar...");
		pthread_mutex_lock(&sem_socket_operaciones_coordi);
		enviar_orden_instancia(0, (void*) (intptr_t) nodo_aux->instancia.sockfd,
				14);
		pthread_mutex_unlock(&sem_socket_operaciones_coordi);
		nodo_aux = nodo_aux->sgte;
	}
}

void send_orden_no_exit(int op_code, int sockfd) {
	pthread_mutex_lock(&sem_socket_operaciones_coordi);
	enviar_orden_instancia(0, (void*) (intptr_t) sockfd, op_code);

	log_debug(logger, "Esperando confirmacion...");

	esperar_confirmacion_de_exito((int) sockfd);
	pthread_mutex_unlock(&sem_socket_operaciones_coordi);

}

void enviar_valores_set(int tamanio_parametros_set, void * un_socket) {

	buffer_parametros = serializar_valores_set(tamanio_parametros_set,
			&(valor_set));

	loggear("Enviando parametros a la instancia");

	int res = send((intptr_t) un_socket, buffer_parametros,
			tamanio_parametros_set, 0);

	log_trace(logger, "Enviado: %i", res);

	loggear("Parametros enviados!");

}

int instanciasDisponibles() {
	t_instancia_node* puntero = instancias.head;
	int size = 0;

	while (puntero != NULL) {
		if (puntero->instancia.disponible) {
			size++;
		}

		puntero = puntero->sgte;
	}

	return size;
}

char * darLosDeadlock(void) {
	return pasarACadena(estanEnDL(tienenAlgoRetenido(blocked_ESIs)));
}
t_blocked_list tienenAlgoRetenido(t_blocked_list lista) {
	t_blocked_node* puntero = blocked_ESIs.head;
	t_blocked_list retenientes;

	while (puntero != NULL) {
		if (tieneAlgoRetenido(puntero->id)) {
			blocked newBlocked = { .id = puntero->id };
			strcpy(newBlocked.clave, puntero->clave);

			agregar_blocked(&retenientes, newBlocked);
		}

		puntero = puntero->sgte;
	}

	return retenientes;
}
bool tieneAlgoRetenido(uint32_t id) {
	t_clave_node * puntero = claves_bloqueadas.head;

	while (puntero != NULL) {
		if (puntero->block_id == id)
			return true;
		puntero = puntero->sgte;
	}

	return false;
}
t_blocked_list estanEnDL(t_blocked_list lista) {
	t_blocked_node * puntero = lista.head;
	t_blocked_list * deadlock = NULL;

	while (puntero != NULL) {
		if (puedeLlegarA(puntero)) {
			blocked newBlocked = { .id = puntero->id };
			strcpy(newBlocked.clave, puntero->clave);

			agregar_blocked(deadlock, newBlocked);

		}

		puntero = puntero->sgte;
	}

	return *deadlock;
}

bool puedeLlegarA(t_blocked_node * puntero) {
	t_clave_node * aux;
	t_blocked_node * aux2 = NULL;
	aux = duenioDe(puntero->clave);

	if (listaAuxiliar.head->id == aux->block_id) {
		liberar(&listaAuxiliar);
		return true;
	} else {
		if (!estaEn(listaAuxiliar, puntero->id)) {
			aux2->id = aux->block_id;
			strcpy(aux2->clave, aux->clave);
			agregar(listaAuxiliar, *aux2);
			puedeLlegarA(aux2);
		}
	}
	liberar(&listaAuxiliar);
	return false;
}
t_clave_node * duenioDe(char * claveBuscada) {
	t_clave_node * puntero = claves_bloqueadas.head;

	while (puntero != NULL) {
		if (mismoString(puntero->clave, claveBuscada)) {
			return puntero;
		}
		puntero = puntero->sgte;
	}
	return NULL;
}
void liberar(t_blocked_list * lista) {
	while (lista->head != NULL) {
		eliminar_blockeados(lista);
	}
}

bool estaEn(t_blocked_list lista, uint32_t id) {
	t_blocked_node * puntero = lista.head;
	while (puntero != NULL) {
		if (puntero->id == id)
			return true;
		puntero = puntero->sgte;
	}
	return false;
}
void agregar(t_blocked_list lista, t_blocked_node nodo) {
	t_blocked_node * puntero = lista.head;
	while (puntero != NULL) {
		puntero = puntero->sgte;
	}
	puntero->id = nodo.id;
	strcpy(puntero->clave, nodo.clave);
	puntero->sgte = NULL;
}
char * pasarACadena(t_blocked_list lista) {
	t_blocked_node * puntero = lista.head;
	char * cadena = malloc(sizeof(char*));
	char stringAux[20];
	int i;
	while (puntero != NULL) {
		i = (int) puntero->id;
		strcpy(stringAux, string_itoa(i));
		strcat(cadena, stringAux);
		puntero = puntero->sgte;
	}
	return cadena;
}

void comunicarDeadlock(int socket) {
	char * cadena = malloc(sizeof(char*));
	int i = 0;
	package_int paquete;
	strcpy(cadena, darLosDeadlock());
	while (cadena[i] != '\0') {
		i++;
	}
	paquete.packed = (uint32_t) i;
	send_package_int(paquete, socket_planificador);
	enviar_cadena(cadena, socket_planificador);
}

void actualizarEntradas(Instancia instancia, uint32_t cantidad) {
	instancia.espacio_usado = cantidad;
}

/*
 //PARA 3 Entradas de tamanio 8
 void enviar_ordenes_de_prueba(void* un_socket) {

 asignar_parametros_a_enviar_de_prueba();

 int tamanio_parametros_set = 2 * sizeof(uint32_t) + valor_set.tamanio_clave
 + valor_set.tamanio_valor;

 int i;

 for (i = 0; i < 3; i++) {

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 }

 valor_set.clave = "OtraClave";
 valor_set.tamanio_clave = 9;
 valor_set.valor = "PALABRAGRANDE";
 valor_set.tamanio_valor = 13;

 tamanio_parametros_set = 9 + 13 + 2 * 4;

 for (i = 0; i < 4; i++) {

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 }

 valor_set.clave = "OtraClaveDe21Letras00";
 valor_set.tamanio_clave = 21;
 valor_set.valor = "NAPARECE";
 valor_set.tamanio_valor = 8;

 tamanio_parametros_set = 21 + 8 + 2 * 4;

 enviar_orden_instancia(0, un_socket, 15);

 for (i = 0; i < 2; i++) {

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 }

 enviar_orden_instancia(0, un_socket, 15);

 valor_set.clave = "OtraClaveDe21Letras01";
 valor_set.tamanio_clave = 21;
 valor_set.valor = "VALOR01";
 valor_set.tamanio_valor = 7;

 tamanio_parametros_set = 21 + 7 + 2 * 4;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 enviar_orden_instancia(0, un_socket, 15);

 valor_set.clave = "OtraClaveDe21Letras02";
 valor_set.valor = "VALOR02";

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 enviar_orden_instancia(0, un_socket, 15);

 valor_set.clave = "OtraClaveDe21Letras03";
 valor_set.valor = "APARECEGrande";
 valor_set.tamanio_valor = 13;

 tamanio_parametros_set = 21 + 13 + 2 * 4;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 enviar_orden_instancia(0, un_socket, 14);

 enviar_orden_instancia(0, un_socket, 15);

 valor_set.clave = "OtraClaveDe21Letras04";
 valor_set.valor = "VALOR04";
 valor_set.tamanio_valor = 7;

 tamanio_parametros_set = 21 + 7 + 2 * 4;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 enviar_orden_instancia(0, un_socket, 14);

 enviar_orden_instancia(0, un_socket, 15);

 valor_set.clave = "OtraClaveDe21Letras05";
 valor_set.valor = "VALOR05";

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 enviar_orden_instancia(0, un_socket, 15);

 enviar_orden_instancia(0, un_socket, 14);

 enviar_orden_instancia(0, un_socket, 15);

 valor_set.clave = "OtraClaveDe21Letras06";
 valor_set.valor = "VALOR06";

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 enviar_orden_instancia(0, un_socket, 15);

 valor_set.clave = "OtraClaveDe21Letras07";
 valor_set.valor = "VALOR07";

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 enviar_orden_instancia(0, un_socket, 15);

 }

 void asignar_parametros_a_enviar_de_prueba() {

 valor_set.clave = "Clave";
 valor_set.tamanio_clave = strlen(valor_set.clave);
 valor_set.valor = "UnValor";
 valor_set.tamanio_valor = strlen(valor_set.valor);

 }

 //PARA 17 Entradas de tamanio 8 y BSU
 void enviar_ordenes_de_prueba_compactacion(void* un_socket) {

 int tamanio_parametros_set;

 valor_set.clave = "Clave00";
 valor_set.tamanio_clave = 7;
 valor_set.valor = "Valor00";
 valor_set.tamanio_valor = 7;

 tamanio_parametros_set = 7 + 7 + 2 * 4;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);
 enviar_valores_set(tamanio_parametros_set, un_socket);

 valor_set.clave = "Clave01";
 valor_set.valor = "Valor01";

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);
 enviar_valores_set(tamanio_parametros_set, un_socket);

 valor_set.clave = "Clave02";
 valor_set.valor = "Valor02888888888888888888888888";
 valor_set.tamanio_valor = 7 + 8 * 3;

 tamanio_parametros_set += 24;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);
 enviar_valores_set(tamanio_parametros_set, un_socket);

 valor_set.clave = "Clave03";
 valor_set.valor = "Valor03";
 valor_set.tamanio_valor = 7;

 tamanio_parametros_set = 7 + 7 + 2 * 4;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);
 enviar_valores_set(tamanio_parametros_set, un_socket);

 valor_set.clave = "Clave04";
 valor_set.valor = "Valor028888888888888888";
 valor_set.tamanio_valor = 7 + 8 * 2;

 tamanio_parametros_set += 16;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);
 enviar_valores_set(tamanio_parametros_set, un_socket);

 valor_set.clave = "Clave05";
 valor_set.valor = "Valor05";
 valor_set.tamanio_valor = 7;

 tamanio_parametros_set = 7 + 7 + 2 * 4;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);
 enviar_valores_set(tamanio_parametros_set, un_socket);

 valor_set.clave = "Clave06";
 valor_set.valor = "Valor068888888888888888";
 valor_set.tamanio_valor = 7 + 8 * 2;

 tamanio_parametros_set += 16;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);
 enviar_valores_set(tamanio_parametros_set, un_socket);

 valor_set.clave = "Clave07";
 valor_set.valor = "Valor078888888888888888";

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);
 enviar_valores_set(tamanio_parametros_set, un_socket);

 enviar_orden_instancia(0, un_socket, 15);

 enviar_orden_instancia(0, un_socket, 14);

 valor_set.clave = "Clave08";
 valor_set.valor = "Valor088888888888888888888888888888888888888888";
 valor_set.tamanio_valor = 7 + 8 * 5;

 tamanio_parametros_set += 24;

 enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

 enviar_valores_set(tamanio_parametros_set, un_socket);

 enviar_orden_instancia(0, un_socket, 15);

 enviar_orden_instancia(0, un_socket, 14);

 enviar_orden_instancia(0, un_socket, 15);

 }

 */
