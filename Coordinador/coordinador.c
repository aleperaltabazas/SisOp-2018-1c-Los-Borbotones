/*
 * coordinador.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"

int main(int argc, char** argv) {
	iniciar();

	int listening_socket = levantar_servidor(PUERTO_COORDINADOR);
	int socketCliente;

	while (seguir_ejecucion) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				mensajeCoordinador);
	}

	loggear("Cerrando sesion...");

	close(socketCliente);
	close(listening_socket);
	return EXIT_SUCCESS;
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

algoritmo_distribucion dame_algoritmo(char* algoritmo_src){
	algoritmo_distribucion algoritmo_ret;

	if(strcmp(algoritmo_src, "LSU") == 0){
		algoritmo_ret = LSU;
	}

	else if(strcmp(algoritmo_src, "EL") == 0){
		algoritmo_ret = EL;
	}

	else if(strcmp(algoritmo_src, "KE") == 0){
		algoritmo_ret = KE;
	}

	return algoritmo_ret;
}

float dame_retardo(int retardo_int){
	float ret_val = (float) retardo_int;
	ret_val = ret_val/1000;

	return ret_val;
}

int manejar_cliente(int listening_socket, int socketCliente, char* mensaje) {

	loggear("Esperando cliente...");

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	loggear("Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	char package[PACKAGE_SIZE];

	int res = recv(socketCliente, (void*) package, PACKAGE_SIZE, 0);

	if (res <= 0) {
		loggear("Fallo la conexion con el cliente.");
	}

	loggear("Mensaje recibido exitosamente. Identificando cliente...");
	identificar_cliente((char*) package, socketCliente);

	loggear("Enviando mensaje al cliente.");

	send(socketCliente, mensaje, strlen(mensaje) + 1, 0);

	loggear("Mensaje enviado. Cerrando sesion con el cliente actual.");

	return socketCliente;
}

void identificar_cliente(char* mensaje, int socket_cliente) {
	char* mensajePlanificador =
			"My name is Planificador.c and I'm the fastest planifier alive...";
	char* mensajeESI = "A wild ESI has appeared!";
	char* mensajeInstancia = "It's ya boi, instancia!";

	if (strcmp(mensaje, mensajePlanificador) == 0) {
		loggear(mensajePlanificador);
		pthread_create(&hilo_planificador, NULL, atender_Planificador,
				(void*) socket_cliente);
		pthread_detach(hilo_planificador);
	} else if (strcmp(mensaje, mensajeESI) == 0) {
		loggear(mensajeESI);
		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);
		pthread_detach(hilo_ESI);
	} else if (strcmp(mensaje, mensajeInstancia) == 0) {
		loggear(mensajeInstancia);
		pthread_create(&hilo_instancia, NULL, atender_Instancia,
				(void*) socket_cliente);
		pthread_detach(hilo_instancia);
	} else {
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
	aviso_ESI aviso_cliente;
	aviso_ESI aviso_servidor = { .aviso = 1 };

	int packageSize = sizeof(aviso_cliente.aviso) + sizeof(aviso_cliente.id);
	char *message = malloc(packageSize);
	char *package = malloc(packageSize);

	//Me gustaria mas que la aviso_servidor se envie como bool

	int res = recv(socket_cliente, (void*) package, packageSize, 0);

	if (res != 0) {
		loggear("Mensaje recibido del ESI.");
	} else {
		log_error(logger, "Fallo la peticion. Terminando ESI.");

		terminar_conexion(socket_cliente);
	}

	deserializar_aviso(&(aviso_cliente), &(package));

	if (aviso_cliente.aviso == 0) {
		loggear("Fin de ESI.");
		return 0;
	} else if (aviso_cliente.aviso == 1) {
		loggear("Ejecución de ESI.");
	} else {
		loggear("Mensaje erróneo. Abortando ESI.");
		terminar_conexion(socket_cliente);
	}

	serializar_aviso(aviso_servidor, &message);

	send(socket_cliente, message, packageSize, 0);

	free(message);
	free(package);

	return 1;
}

void* atender_Planificador(void* un_socket) {
	int socket_cliente = (int) un_socket;

	loggear("Hilo de planificador inicializado correctamente.");

	aviso_ESI aviso_plani;

	int packageSize = sizeof(aviso_ESI);
	char* message = malloc(packageSize);

	while (1) {
		int res = recv(socket_cliente, (void*) message, packageSize, 0);

		if (res != 0) {
			loggear("Mensaje recibido del planificador.");
		} else {
			salir_con_error("Fallo la recepción de mensaje del planificador.",
					socket_cliente);
		}

		deserializar_aviso(&(aviso_plani), &(message));

		if (aviso_plani.aviso == 0) {
			loggear("Fin de Planificador. Cerrando sesión y terminando.");
			exit(42);
			break;
		}

	}

	seguir_ejecucion = 0;

	return NULL;
}

void* atender_Instancia(void* un_socket) {

	loggear("Hilo de instancia inicializado correctamente.");

	parametros_set valor_set;

	valor_set.tamanio_clave = 5;
	valor_set.clave = "Clave";
	valor_set.tamanio_valor = 7;
	valor_set.valor = "UnValor";

	uint32_t tamanio_parametros_set = 2 * sizeof(uint32_t)
			+ strlen(valor_set.clave) + strlen(valor_set.valor);

	uint32_t tamanio_orden = sizeof(orden_del_coordinador);

	//---------

	orden_del_coordinador orden;
	orden.codigo_operacion = 11;
	orden.tamanio_a_enviar = tamanio_parametros_set;

	log_trace(logger, "tamanio a enviar: %d", orden.tamanio_a_enviar);

	//Quiero mandar dos uint32_t
	orden_del_coordinador * buffer_orden = malloc(
			sizeof(orden_del_coordinador));

	memcpy(buffer_orden, &orden, tamanio_orden);

	//memcpy(buffer_orden, &(orden.codigo_operacion), sizeof(uint32_t));
	//memcpy(buffer_orden + sizeof(uint32_t), &(orden.tamanio_a_enviar), sizeof(uint32_t));

	loggear("Enviando orden a la instancia...");

	if (send((int) un_socket, (void*) buffer_orden,
			sizeof(orden_del_coordinador), 0) < 0) {
		loggear("Error en el envio de la orden");
		return -1;
	}

	loggear("Orden enviada!");

	//Serializacion valor_set

	parametros_set * buffer_parametros = malloc(tamanio_parametros_set);

	int offset = 0;

	memcpy(buffer_parametros, &valor_set.tamanio_clave, sizeof(uint32_t));

	offset += sizeof(uint32_t);

	memcpy(buffer_parametros + offset, &valor_set.clave,
			strlen(valor_set.clave));

	offset += strlen(valor_set.clave);

	memcpy(buffer_parametros + offset, &valor_set.tamanio_valor,
			sizeof(uint32_t));

	offset += sizeof(uint32_t);

	memcpy(buffer_parametros + offset, &valor_set.valor,
			strlen(valor_set.valor));

	loggear("Enviando parametros a la instancia");

	send((int) un_socket, (void*) buffer_parametros, sizeof(valor_set), 0);

	loggear("Parametros enviados!");

	return NULL;
}

