/*
 * coordinador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"

int main(int argc, char** argv) {
	iniciar(argv);

	coordinar();

	loggear("Terminando proceso...");

	return EXIT_SUCCESS;
}

void coordinar(void) {
	int listening_socket = levantar_servidor(PUERTO_COORDINADOR, 0);
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

	instancia_id = 0;
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

int manejar_cliente(int listening_socket, int socketCliente, package_int id) {

	log_info(logger, "Esperando cliente...");

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	log_info(logger, "Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	package_int id_cliente = { .packed = -1 };

	id_cliente = recibir_packed(socketCliente);

	log_info(logger, "Mensaje recibido exitosamente. Identificando cliente...");
	identificar_cliente(id_cliente, socketCliente);

	loggear("Enviando id al cliente.");

	enviar_packed(id, socketCliente);

	log_info(logger, "Handshake realizado correctamente.");

	return socketCliente;
}

void identificar_cliente(package_int id, int socket_cliente) {
	if (id.packed == 1) {
		log_info(logger, mensajePlanificador);
		pthread_create(&hilo_planificador, NULL, atender_Planificador,
				(void*) socket_cliente);
		pthread_detach(hilo_planificador);
	}

	else if (id.packed == 2) {
		log_info(logger, mensajeESI);
		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);
		pthread_detach(hilo_ESI);
	}

	else if (id.packed == 3) {
		log_info(logger, mensajeInstancia);
		pthread_create(&hilo_instancia, NULL, atender_instancia,
				(void*) socket_cliente);
		pthread_detach(hilo_instancia);
	}

	else {
		salir_con_error("Cliente desconocido, cerrando conexion.",
				socket_cliente);
	}

	return;
}

void* atender_ESI(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de ESI inicializado correctamente.");

	int status = 1;

	while (status) {
		status = chequear_solicitud(socket_cliente);
	}

	return NULL;
}

int chequear_solicitud(int socket_cliente) {
	aviso_con_ID aviso_cliente = recibir_aviso(socket_cliente);

	log_debug(logger, "%i", aviso_cliente.aviso);

	if (aviso_cliente.aviso == 0) {
		log_info(logger, "Fin de ESI.");
		return 0;
	}

	else if (aviso_cliente.aviso == 1) {
		log_info(logger, "Ejecución de ESI.");
	}

	else if (aviso_cliente.aviso == 11) {
		log_debug(logger, "%i", aviso_cliente.id);
		get(socket_cliente, aviso_cliente.id);
	}

	else if (aviso_cliente.aviso == 12) {
		log_debug(logger, "%i", aviso_cliente.id);
		set(socket_cliente, aviso_cliente.id);
	}

	else if (aviso_cliente.aviso == 13) {
		log_debug(logger, "%i", aviso_cliente.id);
		store(socket_cliente, aviso_cliente.id);
	}

	else {
		log_warning(logger, "Mensaje erróneo. Abortando ESI.");
		terminar_conexion(socket_cliente);
		return 0;
	}

	return 1;
}

void get(int socket_cliente, uint32_t id) {
	aviso_con_ID aviso_ok = { .aviso = 10 };

	enviar_aviso(socket_cliente, aviso_ok);

	package_int size_packed = recibir_packed(socket_cliente);
	uint32_t clave_size = size_packed.packed;

	char* clave = recibir_cadena(socket_cliente, clave_size);

	package_int response;

	response.packed = dame_response(clave, id);

	log_debug(logger, "%i", response.packed);

	sleep(2);

	enviar_packed(response, socket_cliente);
}

int dame_response(char* clave, uint32_t id) {
	if (!existe(clave)) {
		bloquear(clave, id);
		loggear("Get exitoso.");
		return 20;
	}

	else if (existe(clave) && !esta_bloqueada(clave)) {
		bloquear(clave, id);
		loggear("Get exitoso.");
		return 20;
	}

	else {
		log_warning(logger, "Bloqueando ESI %i.", id);
		bloquear_ESI(clave, id);
		return 5;
	}
}

void set(int socket_cliente, uint32_t id) {
	aviso_con_ID aviso_ok = { .aviso = 10 };

	enviar_aviso(socket_cliente, aviso_ok);

	package_int clave_size_packed = recibir_packed(socket_cliente);
	uint32_t clave_size = clave_size_packed.packed;
	char* clave = recibir_cadena(socket_cliente, clave_size);

	package_int valor_size_packed = recibir_packed(socket_cliente);
	uint32_t valor_size = valor_size_packed.packed;
	char* valor = recibir_cadena(socket_cliente, valor_size);

	package_int response;

	response.packed = settear(valor, clave, id);

	log_debug(logger, "%i", response.packed);

	sleep(2);

	enviar_packed(response, socket_cliente);
}

void store(int socket_cliente, uint32_t id) {
	aviso_con_ID aviso_ok = { .aviso = 10 };

	enviar_aviso(socket_cliente, aviso_ok);

	package_int size_packed = recibir_packed(socket_cliente);
	uint32_t clave_size = size_packed.packed;

	char* clave = recibir_cadena(socket_cliente, clave_size);

	package_int response;
	response.packed = get_packed(clave, id);

	enviar_packed(response, socket_cliente);

	if (response.packed != 5) {
		hacer_store(clave);

		if (!esta_vacia(&blocked_ESIs)) {
			aviso_con_ID unlock = { .aviso = 28, .id = dame_desbloqueado(clave,
					blocked_ESIs) };
			liberar_ESI(&blocked_ESIs, unlock.id);
			enviar_aviso(socket_planificador, unlock);

		}

	}

	sleep(2);

	log_debug(logger, "%i", response.packed);

}

int settear(char* valor, char* clave, uint32_t id) {
	t_clave_node* puntero = claves_bloqueadas.head;

	if (!existe(clave)) {
		return 5;
	}

	while (puntero != NULL) {
		if (strcmp(puntero->clave, clave) == 0) {
			if (puntero->block_id != id) {
				log_debug(logger, "%i %i", puntero->block_id, id);

				log_warning(logger, "Bloqueando ESI %i.", id);
				bloquear_ESI(clave, id);
				return 5;
			}

			puntero->valor = valor;

			log_info(logger, "SET %s %s", clave, valor);
			do_set(valor, clave);

			return 20;
		}
		puntero = puntero->sgte;
	}

	return 5;
}

void do_set(char* valor, char* clave) {
	uint32_t valor_size = (uint32_t) strlen(valor);
	uint32_t clave_size = (uint32_t) strlen(clave);

	valor_set.tamanio_clave = clave_size;
	valor_set.tamanio_valor = valor_size;
	valor_set.clave = clave;
	valor_set.valor = valor;

	int sockfd = dame_instancia(clave);

	int tamanio_parametros_set = 2 * sizeof(uint32_t) + valor_set.tamanio_clave
			+ valor_set.tamanio_valor;

	log_trace(logger, "CLAVE: %d VALOR: %d TAMANIO_PARAMETROS: %d", clave_size,
			valor_size, tamanio_parametros_set);

	enviar_orden_instancia(tamanio_parametros_set, (void*) sockfd, 11);
	enviar_valores_set(tamanio_parametros_set, (void*) sockfd);

	enviar_orden_instancia(0, (void*) sockfd, 15);
}

int dame_instancia(char* clave) {
	int ret_sockfd;

	switch (ALGORITMO_DISTRIBUCION) {
	case EL:
		ret_sockfd = equitativeLoad();
		break;

	case LSU:
		ret_sockfd = leastSpaceUsed();
		break;
	case KE:
		ret_sockfd = keyExplicit(clave);
		break;
	default:
		log_warning(logger, "Fallo en el algoritmo");
		break;
	}

	t_instancia_node* ret_node = instancia_head(instancias);
	ret_sockfd = ret_node->socket;

	return ret_sockfd;
}

int equitativeLoad(void) {
	return 0;
}

int leastSpaceUsed(void) {
	return 0;
}

int keyExplicit(char* clave) {
	return 0;
}

int get_packed(char* clave, uint32_t id) {
	if (!existe(clave)) {
		log_warning(logger, "Bloqueando ESI %i.", id);
		bloquear_ESI(clave, id);
		return 5;
	}

	else {
		int blocker = get_clave_id(clave);
		log_trace(logger, "%i", blocker);

		if (blocker == -1) {
			log_warning(logger, "Bloqueando ESI %i.", id);
			bloquear_ESI(clave, id);
			return 5;
		}

		if (blocker != id) {
			log_warning(logger, "Bloqueando ESI %i.", id);
			bloquear_ESI(clave, id);
			return 5;
		}

		else if (!esta_bloqueada(clave)) {
			log_warning(logger, "Bloqueando ESI %i.", id);
			bloquear_ESI(clave, id);
			return 5;
		}

		else {
			desbloquear(clave);
			log_info(logger, "STORE %s.", clave);
			return 20;
		}
	}
}

void hacer_store(char* clave) {
	t_instancia_node* node = instancia_head(instancias);
	int sockfd = node->socket;

	enviar_orden_instancia(0, (void*) sockfd, 12);
	sleep(1);

	uint32_t clave_size = (uint32_t) strlen(clave) + 1;

	package_int package_size = { .packed = clave_size };

	enviar_packed(package_size, sockfd);
	sleep(1);
	enviar_cadena(clave, sockfd);
}

void bloquear_ESI(char* clave, uint32_t id) {
	blocked bloqueado = { .clave = clave, .id = id };

	agregar_blocked(&blocked_ESIs, bloqueado);
}

bool esta_vacia(t_blocked_list* lista) {
	return lista->head == NULL;
}

void liberar_ESI(t_blocked_list* lista, uint32_t id) {
	if (id != -5) {
		eliminar_blocked(lista, id);
	}
}

void destruir_blocked_node(t_blocked_node* nodo) {
	free(nodo);
}

void eliminar_blocked(t_blocked_list* lista, uint32_t id) {
	if (lista->head != NULL) {
		uint32_t head = head_id(*lista);
		if (id == head) {
			t_blocked_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_blocked_node(eliminado);
		} else {
			t_blocked_node* puntero = lista->head;

			while (puntero->id != id) {
				puntero = puntero->sgte;
			}

			t_blocked_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_blocked_node(eliminado);
		}
	}
}

void agregar_blocked(t_blocked_list* lista, blocked bloqueado) {
	t_blocked_node* nodo = crear_blocked_node(bloqueado);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_blocked_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}
}

t_blocked_node* crear_blocked_node(blocked bloqueado) {
	t_blocked_node* nodo = (t_blocked_node*) malloc(sizeof(t_blocked_node));
	nodo->clave = bloqueado.clave;
	nodo->id = bloqueado.id;

	return nodo;
}

uint32_t head_id(t_blocked_list lista) {
	t_blocked_node* head = lista.head;

	return head->id;
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

void* atender_Planificador(void* un_socket) {
	socket_planificador = (int) un_socket;

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

	}

	seguir_ejecucion = 0;

	return NULL;
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
	t_blocked_node* puntero = lista.head;

	while (puntero != NULL) {
		if (strcmp(clave, puntero->clave) == 0) {
			return puntero->id;
		}

		puntero = puntero->sgte;
	}

	return -5;
}

void desbloquear(char* clave) {
	if (!existe(clave)) {
		crear(clave);
		desbloquear(clave);
	}

	else if (existe(clave) && esta_bloqueada(clave)) {
		eliminar_clave(&claves_bloqueadas, clave);
		agregar_clave(&claves_disponibles, clave, -1);

		log_info(logger, "La clave %s fue desbloqueada.", clave);
	}

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

void agregar_clave(t_clave_list* lista, char* clave, uint32_t id) {
	t_clave_node* nodo = crear_nodo(clave, id);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_clave_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}
}

t_clave_node* crear_nodo(char* clave, uint32_t id) {
	t_clave_node* nodo = (t_clave_node*) malloc(sizeof(t_clave_node));
	nodo->clave = clave;
	nodo->block_id = id;
	nodo->sgte = NULL;

	return nodo;
}

void eliminar_clave(t_clave_list* lista, char* clave) {
	if (lista->head != NULL) {
		char* head = first(*lista);
		if (strcmp(clave, head) == 0) {
			t_clave_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo(eliminado);
		} else {
			t_clave_node* puntero = lista->head;

			while (!strcmp(puntero->clave, clave) != 0) {
				puntero = puntero->sgte;
			}

			t_clave_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo(eliminado);
		}
	}
}

void destruir_nodo(t_clave_node* nodo) {
	free(nodo);
}

char* first(t_clave_list lista) {
	char* key = lista.head->clave;

	return key;
}

void* atender_instancia(void* un_socket) {

	int sockfd = (int) un_socket;

	loggear("Hilo de instancia inicializado correctamente.");

	agregar_instancia(&instancias, sockfd);

	instancia_id++;

	asignar_entradas(sockfd);

	loggear("Instancia agregada correctamente");

	enviar_ordenes_de_prueba(un_socket);

	//enviar_ordenes_de_prueba_compactacion(un_socket);

	return NULL;
}

void asignar_entradas(int sockfd) {
	log_trace(logger, "Cant entradas: %d, Tamanio_entrada: %d",
			CANTIDAD_ENTRADAS, TAMANIO_ENTRADAS);
	enviar_orden_instancia(CANTIDAD_ENTRADAS, (void*) sockfd, TAMANIO_ENTRADAS);
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

	if (send((int) un_socket, (void*) buffer_orden, tamanio_orden, 0) < 0) {
		loggear("Error en el envio de la orden");
		return;
	}

	loggear("Orden enviada!");

	free(buffer_orden);

}

void enviar_valores_set(int tamanio_parametros_set, void * un_socket) {

	buffer_parametros = serializar_valores_set(tamanio_parametros_set,
			&(valor_set));

	loggear("Enviando parametros a la instancia");

	send((int) un_socket, buffer_parametros, tamanio_parametros_set, 0);

	loggear("Parametros enviados!");

}

t_instancia_node* crear_instancia_node(int sockfd) {
	t_instancia_node* nodo = (t_instancia_node*) malloc(
			sizeof(t_instancia_node));
	nodo->socket = sockfd;

	return nodo;
}

void destruir_instancia_node(t_instancia_node* nodo) {
	free(nodo);
}

void agregar_instancia(t_instancia_list* lista, int sockfd) {

	t_instancia_node* nodo = crear_instancia_node(sockfd);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_instancia_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}

}

t_instancia_node* instancia_head(t_instancia_list lista) {
	t_instancia_node* instancia = lista.head;

	return instancia;
}

void eliminar_instancia(t_instancia_list* lista, int id) {
	if (lista->head != NULL) {
		t_instancia_node* head = instancia_head(*lista);
		if (id == head->id) {
			t_instancia_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_instancia_node(eliminado);
		} else {
			t_instancia_node* puntero = lista->head;

			while (puntero->id != head->id) {
				puntero = puntero->sgte;
			}

			t_instancia_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_instancia_node(eliminado);
		}
	}
}

int instanciasDisponibles() {
	t_instancia_node* puntero = instancias.head;
	int size = 0;

	while (puntero != NULL) {
		size++;
	}

	return size;
}

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

