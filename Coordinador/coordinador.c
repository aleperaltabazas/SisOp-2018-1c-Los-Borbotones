/*
 * coordinador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"

int main(int argc, char** argv) {
	iniciar();

	coordinar();

	loggear("Terminando proceso...");

	return EXIT_SUCCESS;
}

void coordinar(void) {
	int listening_socket = levantar_servidor(PUERTO_COORDINADOR);
	int socketCliente;

	while (seguir_ejecucion) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				id_coordinador);
	}

	close(socketCliente);
	close(listening_socket);
}

void iniciar(void) {
	iniciar_log("Coordinador", "Nace el coordinador...");
	cargar_configuracion();
}

void cargar_configuracion(void) {
	t_config* config = config_create("coordinador.config");

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

	loggear("Esperando cliente...");

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	loggear("Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	package_int id_cliente = { .packed = -1 };

	id_cliente = recibir_packed(socketCliente);

	loggear("Mensaje recibido exitosamente. Identificando cliente...");
	identificar_cliente(id_cliente, socketCliente);

	loggear("Enviando id al cliente.");

	enviar_packed(id, socketCliente);

	return socketCliente;
}

void identificar_cliente(package_int id, int socket_cliente) {
	if (id.packed == 1) {
		loggear(mensajePlanificador);
		pthread_create(&hilo_planificador, NULL, atender_Planificador,
				(void*) socket_cliente);
		pthread_detach(hilo_planificador);
	}

	else if (id.packed == 2) {
		loggear(mensajeESI);
		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);
		pthread_detach(hilo_ESI);
	}

	else if (id.packed == 3) {
		loggear(mensajeInstancia);
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

	if (aviso_cliente.aviso == 0) {
		loggear("Fin de ESI.");
		return 0;
	}

	else if (aviso_cliente.aviso == 1) {
		loggear("Ejecución de ESI.");
	}

	else if (aviso_cliente.aviso == 11) {
		loggear("GET.");
		get(socket_cliente, aviso_cliente.id);
	}

	else if (aviso_cliente.aviso == 12) {
		loggear("SET.");
		set(socket_cliente, aviso_cliente.id);
	}

	else if (aviso_cliente.aviso == 13) {
		loggear("STORE");
		store(socket_cliente, aviso_cliente.id);
	}

	else {
		loggear("Mensaje erróneo. Abortando ESI.");
		terminar_conexion(socket_cliente);
		return 0;
	}

	return 1;
}

void get(int socket_cliente, int id) {
	aviso_con_ID aviso_ok = { .aviso = 10 };

	enviar_aviso(socket_cliente, aviso_ok);

	package_int size_packed = recibir_packed(socket_cliente);
	uint32_t clave_size = size_packed.packed;

	char* clave = recibir_cadena(socket_cliente, clave_size);

	package_int response;

	response.packed = dame_response(clave, id);

	enviar_packed(response, socket_cliente);
}

int dame_response(char* clave, int id) {
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
		loggear("Bloqueando ESI.");
		return 5;
	}
}

void set(int socket_cliente, int id) {
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

	enviar_packed(response, socket_cliente);
}

void store(int socket_cliente, int id) {
	aviso_con_ID aviso_ok = { .aviso = 10 };

	enviar_aviso(socket_cliente, aviso_ok);

	package_int size_packed = recibir_packed(socket_cliente);
	uint32_t clave_size = size_packed.packed;

	char* clave = recibir_cadena(socket_cliente, clave_size);

	package_int response;
	response.packed = get_packed(clave, id);

	if (response.packed != 5) {
		hacer_store(clave);
	}

	enviar_packed(response, socket_cliente);
}

int settear(char* valor, char* clave, int id) {
	t_clave_node* puntero = claves_bloqueadas.head;

	if (!existe(clave)) {
		return 5;
	}

	while (puntero != NULL) {
		if (strcmp(puntero->clave, clave) == 0) {
			if (puntero->block_id != id) {
				loggear("Bloqueando ESI.");
				return 5;
			}

			puntero->valor = valor;

			loggear("Set exitoso.");
			return 20;
		}
	}

	return 5;
}

int get_packed(char* clave, int id) {
	if (!existe(clave)) {
		loggear("Bloqueando ESI.");
		return 5;
	}

	else {
		int blocker = get_clave_id(clave);

		if (blocker == -1) {
			loggear("Bloqueando ESI.");
			return 5;
		}

		if (blocker != id) {
			loggear("Bloqueando ESI.");
			return 5;
		}

		else if (!esta_bloqueada(clave)) {
			loggear("Bloqueando ESI.");
			return 5;
		}

		else {
			desbloquear(clave);
			loggear("Store exitoso.");
			return 20;
		}
	}
}

void hacer_store(char* clave) {
	//MATIIIIIIIIIIIIIIIIII te toca hacer esto
}

int get_clave_id(char* clave) {
	t_clave_node* puntero = claves_bloqueadas.head;

	while (puntero != NULL) {
		if (strcmp(puntero->clave, clave) == 0) {
			return puntero->block_id;
		}
	}

	return -1;
}

void* atender_Planificador(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de planificador inicializado correctamente.");

	aviso_con_ID aviso_plani;

	while (1) {
		aviso_plani = recibir_aviso(socket_cliente);

		if (aviso_plani.aviso == 0) {
			loggear("Fin de Planificador. Cerrando sesión y terminando.");
			exit(42);
			break;
		}

		else if (aviso_plani.aviso == 25) {
			bloquear_clave(socket_cliente);
		}

		else if (aviso_plani.aviso == 27) {
			desbloquear_clave(socket_cliente);
		}

	}

	seguir_ejecucion = 0;

	return NULL;
}

void desbloquear_clave(int socket_cliente) {
	aviso_con_ID aviso_ok = { .aviso = 25 };

	package_int size_package = { .packed = -1 };

	package_int unlock_ok = { .packed = 28 };

	enviar_aviso(socket_cliente, aviso_ok);

	size_package = recibir_packed(socket_cliente);
	char* clave = recibir_cadena(socket_cliente, size_package.packed);

	desbloquear(clave);

	enviar_packed(unlock_ok, socket_cliente);

}

void desbloquear(char* clave) {
	if (!existe(clave)) {
		crear(clave);
		desbloquear(clave);
	}

	else if (existe(clave) && esta_bloqueada(clave)) {
		eliminar_clave(&claves_bloqueadas, clave);
		agregar_clave(&claves_disponibles, clave);

		log_trace(logger, "La clave %s fue desbloqueada.", clave);
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

void bloquear(char* clave, int id) {
	if (!existe(clave)) {
		crear(clave);
		bloquear(clave, id);
	}

	else if (existe(clave) && !esta_bloqueada(clave)) {
		eliminar_clave(&claves_disponibles, clave);
		agregar_clave(&claves_bloqueadas, clave);

		log_trace(logger, "La clave %s fue bloqueada por %i.", clave, id);
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
	agregar_clave(&claves_disponibles, clave);
}

void agregar_clave(t_clave_list* lista, char* clave) {
	t_clave_node* nodo = crear_nodo(clave);

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

t_clave_node* crear_nodo(char* clave) {
	t_clave_node* nodo = (t_clave_node*) malloc(sizeof(t_clave_node));
	nodo->clave = clave;
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

	loggear("Hilo de instancia inicializado correctamente.");

	asignar_parametros_a_enviar();

	int tamanio_parametros_set = 2 * sizeof(uint32_t) + valor_set.tamanio_clave
			+ valor_set.tamanio_valor;

	enviar_orden_instancia(tamanio_parametros_set, un_socket);

	enviar_valores_set(tamanio_parametros_set, un_socket);

	return NULL;

}

void asignar_parametros_a_enviar() {

	//Aca estaria la logica de recibir las claves y valores
	valor_set.clave = "Clave";
	valor_set.tamanio_clave = strlen(valor_set.clave);
	valor_set.valor = "UnValor";
	valor_set.tamanio_valor = strlen(valor_set.valor);

}

void enviar_orden_instancia(int tamanio_parametros_set, void* un_socket) {

	orden_del_coordinador orden;
	orden.codigo_operacion = 11;
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

	log_trace(logger, "UNVALOR: COORDI: %c, %c, %c, %c, %c, %c, %c",
			buffer_parametros[13], buffer_parametros[14], buffer_parametros[15],
			buffer_parametros[16], buffer_parametros[17], buffer_parametros[18],
			buffer_parametros[19]);

	send((int) un_socket, buffer_parametros, tamanio_parametros_set, 0);

	loggear("Parametros enviados!");

}
