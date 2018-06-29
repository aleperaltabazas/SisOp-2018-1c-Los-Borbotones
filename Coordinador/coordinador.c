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
		pthread_create(&hilo_instancia, NULL, atender_Instancia,
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

			return 20;
		}

		puntero = puntero->sgte;
	}

	return 5;
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
	//MATIIIIIIIIIIIIIIIIII te toca hacer esto
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
	char* clave = recibir_cadena(socket_cliente, size_package.packed);

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

void* atender_Instancia(void* un_socket) {
	int sockfd = (int) un_socket;

	loggear("Hilo de instancia inicializado correctamente.");

	agregameInstancia(sockfd);

	asignar_parametros_a_enviar();

	int tamanio_parametros_set = 2 * sizeof(uint32_t) + valor_set.tamanio_clave
			+ valor_set.tamanio_valor;

	int i;

	for(i=0; i < 3; i++){

	enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

	enviar_valores_set(tamanio_parametros_set, un_socket);

	}

	valor_set.clave = "OtraClave";
	valor_set.tamanio_clave = 9;
	valor_set.valor = "PALABRAGRANDE";
	valor_set.tamanio_valor = 13;

	tamanio_parametros_set = 9 + 13 + 2*4;


	for(i=0; i < 4; i++){

		enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

		enviar_valores_set(tamanio_parametros_set, un_socket);

	}

	valor_set.clave = "OtraClaveSUPERGIGANTE";
	valor_set.tamanio_clave = 21;
	valor_set.valor = "APARECE";
	valor_set.tamanio_valor = 7;

	tamanio_parametros_set = 21 + 7 + 2*4;

	enviar_orden_instancia(0, un_socket, 15);

	for(i=0; i < 4; i++){

		enviar_orden_instancia(tamanio_parametros_set, un_socket, 11);

		enviar_valores_set(tamanio_parametros_set, un_socket);

	}

	enviar_orden_instancia(0, un_socket, 14);

	enviar_orden_instancia(0, un_socket, 15);

	return NULL;
}

void asignar_parametros_a_enviar() {

//Aca estaria la logica de recibir las claves y valores
	valor_set.clave = "Clave";
	valor_set.tamanio_clave = strlen(valor_set.clave);
	valor_set.valor = "UnValor";
	valor_set.tamanio_valor = strlen(valor_set.valor);

}
void enviar_orden_instancia(int tamanio_parametros_set, void* un_socket, int codigo_de_operacion) {

	orden_del_coordinador orden;
	orden.codigo_operacion = codigo_de_operacion;
	orden.tamanio_a_enviar = tamanio_parametros_set;

	uint32_t tamanio_orden = sizeof(orden_del_coordinador);

	log_trace(logger, "tamanio a enviar: %d", orden.tamanio_a_enviar);

	orden_del_coordinador * buffer_orden = malloc(
			sizeof(orden_del_coordinador));

	//Serializacion de la orden

	memcpy(buffer_orden, &orden, tamanio_orden);

	loggear("Enviando orden a la instancia...");

	if (send((int) un_socket, (void*) buffer_orden,
			sizeof(orden_del_coordinador), 0) < 0) {
		loggear("Error en el envio de la orden");
		return;
	}

	loggear("Orden enviada!");

}

void enviar_valores_set(int tamanio_parametros_set, void * un_socket) {

	buffer_parametros = serializar_valores_set(tamanio_parametros_set,
			&(valor_set));

	loggear("Enviando parametros a la instancia");

	send((int) un_socket, buffer_parametros, tamanio_parametros_set, 0);

	loggear("Parametros enviados!");

}

void agregameInstancia(int unSocket) {
	instancia * auxiliar;
	auxiliar = miLista;
	if (find(auxiliar, unSocket) != NULL)
		(auxiliar->disponible) = 1;
	else
		add(miLista, unSocket);
}
void * find(instancia * lista, int unSocket) {
	while (lista != NULL) {
		if (lista->socket == unSocket)
			return lista;
		else
			lista = lista->siguiente;
	}
	return NULL;
}
void add(instancia * instancias, int unSocket) {
	instancia * nodo;
	nodo = malloc(sizeof(instancia));
	nodo->socket = unSocket;
	nodo->vecesLlamado = 0;
	nodo->disponible = 1;
	nodo->espacio_usado = 0;
	nodo->siguiente = NULL;
	if (instancias == NULL) {
		instancias = nodo;
	} else {
		instancia * auxiliar;
		auxiliar = instancias;
		while (auxiliar->siguiente != NULL) {
			auxiliar = auxiliar->siguiente;
		}
		auxiliar->siguiente = nodo;
	}
}
int instanciasDisponibles() {
	int i;
	instancia * aux;
	aux = malloc(sizeof(instancia));
	aux = miLista;
	while (aux != NULL) {
		if (aux->disponible)
			i++;
		aux = aux->siguiente;
	}
	free(aux);
	return i;
}
int equitativeLoad(void) {
	instancia * aux;
	aux = miLista;
	int i = aux->vecesLlamado;
	int retorno = aux->socket;

	while (aux->siguiente != NULL) {
		if (aux->vecesLlamado << i) {
			i = aux->vecesLlamado;
			retorno = aux->socket;
		}
	}
	aux->vecesLlamado++;
	return (retorno);
}
int leastSpaceUsed(void) { //acá me falta que reciba una cantidad de memoria que va a ocupar así se la puedo sumar
	instancia * aux;
	aux = miLista;
	int i = aux->espacio_usado;
	int retorno = aux->socket;
	while (aux->siguiente != NULL) {
		if (aux->espacio_usado << i) {
			i = aux->espacio_usado;
			retorno = aux->socket;
		} else {
			if (aux->espacio_usado == i)
				retorno = desempatar(aux, retorno);
			aux = aux->siguiente;
		}
	}
	return (retorno);
}
int desempatar(instancia * a, int b) {
	instancia * aux;
	aux = miLista;
	while (aux->socket != b) {
		aux = aux->siguiente;
	}
	if (a->vecesLlamado >> aux->vecesLlamado)
		return aux->socket;
	return a->socket;
}
int keyExplicit(char * clave) {
	int cantidadDeLetras = 26 / instanciasDisponibles();
	int instanciaN = (clave[0] - 97) / cantidadDeLetras;
	instancia * aux;
	aux = miLista;
	while (instanciaN > 1) {
		if (aux->disponible)
			instanciaN--;
		aux = aux->siguiente;
	}
//Creo que acá me falta verificar que estemos mandando una instancia que esté disponible
	return aux->socket;
}
