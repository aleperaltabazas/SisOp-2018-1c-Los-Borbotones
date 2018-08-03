/*
 * instancias.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "instancias.h"

int main(int argc, char** argv) {

	iniciar(argv);

	socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			id_instancia, 0);

	recibir_orden_inicial(socket_coordinador);

	confirmar_resultado_de_operacion(110);

	inicializar(cantidad_entradas, tamanio_entrada);

	//Para desconectarla habria que cambiar este valor simplemente
	disponibilidad_de_conexion = 1;

	while (disponibilidad_de_conexion) {
		orden_del_coordinador orden;
		orden = recibir_orden_coordinador(socket_coordinador);
		switch (orden.codigo_operacion) {
		case -1:
			loggear("Orden de Fin");
			close(socket_coordinador);
			disponibilidad_de_conexion = 0;
			break;
		case 11:
			loggear("Operacion SET");
			set(orden.tamanio_a_enviar, socket_coordinador);
			break;
		case 12:
			loggear("Operacion STORE");
			store(orden.tamanio_a_enviar, socket_coordinador);
			break;
		case 13:
			loggear("Fallo en la operacion");
			break;
		case 14:
			loggear("Operacion de COMPACTACION");
			compactacion();
			break;
		case 15:
			loggear("Mostrando lo almacenado...");
			leer_valores_almacenados();
			break;
		case 40:
			loggear("Enviando nombre");
			send_name(socket_coordinador);
			break;
		case 50:
			loggear("Se ve que me caí. Reviviendo...");
			revivir(socket_coordinador);
			break;
		case 100:
			loggear("Envio de Ping para decir que estoy viva");
			confirmar_resultado_de_operacion(100);
			break;
		default:
			log_error(logger, "ERROR: %s", strerror(errno));
			exit(-1);
			break;
		}
	}

	close(socket_coordinador);

	//Deberia vaciar la lista...

	free(almacenamiento_de_valores);

	free(entradas_disponibles);

	return EXIT_SUCCESS;
}

void startSigHandlers(void) {
	signal(SIGINT, sigHandler_sigint);
}

void sigHandler_sigint(int signo) {
	log_warning(logger, "Tiraste un CTRL+C, macho, abortaste el proceso.");
	log_error(logger, strerror(errno));

	exit(-1);
}

void revivir(int sockfd) {
	int hay_mas_claves = 61;

	while (hay_mas_claves == 61) {

		char * clave = recibir_clave(sockfd);
		FILE * archivo_a_leer = open_file(clave, "r", dump_spot);

		if (archivo_a_leer == NULL) {
			confirmar_resultado_de_operacion(152);
			free(clave);
		} else {
			char * valor = leer_valor_de_archivo(archivo_a_leer);

			fclose(archivo_a_leer);

			parametros_set unos_parametros = { .valor = valor, .tamanio_valor =
					strlen(valor) + 1, .clave = clave, .tamanio_clave = strlen(
					clave) + 1 };

			generar_entrada(unos_parametros);

			free(linea_parseada);

			free(clave);
		}

		hay_mas_claves = recibir_packed(sockfd).packed;
	}

	confirmar_resultado_de_operacion(150);
	leer_valores_almacenados();
}

char * recibir_clave(int sockfd) {
	package_int tamanio_clave = recibir_packed(sockfd);
	char *buffer_clave = recibir_cadena(sockfd, tamanio_clave.packed);
	log_debug(logger, "Clave recibida!: %s", buffer_clave);
	confirmar_resultado_de_operacion(151);
	return buffer_clave;
}

char * leer_valor_de_archivo(FILE * archivo_a_leer) {
	size_t len = 0;
	ssize_t read;

	read = getline(&line, &len, archivo_a_leer);
	linea_parseada = malloc(
			obtener_entradas_que_ocupa(read) * tamanio_entrada + 1);
	memcpy(linea_parseada, line, read);

	if (line) {
		free(line);
	}

	linea_parseada[read] = '\0';
	return linea_parseada;
}

void send_name(int socket_coordinador) {
	uint32_t size = (uint32_t) strlen(NOMBRE) + 1;
	package_int size_package = { .packed = size };

	confirmar_resultado_de_operacion(140);
	enviar_packed(size_package, socket_coordinador);
	enviar_cadena(NOMBRE, socket_coordinador);

}

void recibir_orden_inicial(int socket_coordinador) {

	orden_del_coordinador orden_inicial;

	orden_inicial = recibir_orden_coordinador(socket_coordinador);

	tamanio_entrada = orden_inicial.codigo_operacion;

	cantidad_entradas = orden_inicial.tamanio_a_enviar;

	log_trace(logger, "Cantidad entradas: %i, Tamanio de entrada: %i",
			cantidad_entradas, tamanio_entrada);
}

void store(uint32_t tamanio_a_recibir, int socket_coordinador) {

	char* clave = recibir_cadena(socket_coordinador, tamanio_a_recibir);

	log_debug(logger, "Clave recibida: %s", clave);

	int posicion_de_entrada = posicion_de_entrada_con_clave(clave);

	entradas_node * entrada_seleccionada = buscar_entrada_en_posicion(
			posicion_de_entrada);

	if (entrada_seleccionada == NULL) {
		log_warning(logger, "La clave para STORE no se encuentra disponible");
		confirmar_resultado_de_operacion(666);
		return;
	}

	entrada_seleccionada->una_entrada.tiempo_sin_ser_referenciado =
			reloj_interno;

	reloj_interno++;

	entrada entrada_con_la_clave = entrada_seleccionada->una_entrada;

	int tamanio_valor = entrada_con_la_clave.tamanio_valor;

	char * valor = malloc(tamanio_valor + 1);

	int offset = posicion_de_entrada * tamanio_entrada;

	memcpy(valor, almacenamiento_de_valores + offset, tamanio_valor);

	valor[tamanio_valor] = '\0';

	write_file(clave, valor, PUNTO_MONTAJE);

	free(valor);

	free(clave);

	confirmar_resultado_de_operacion(112);

}

void iniciar(char** argv) {
	iniciar_log("Instancias", "A new Instance joins the brawl!");
	loggear("Cargando configuración.");
	cargar_configuracion(argv);
	startSigHandlers();

	setup_montaje();
	init_dump_thread();
	iniciar_semaforos();
}

void iniciar_semaforos(void) {
	pthread_mutex_init(&sem_entradas, NULL);
}

void init_dump_thread(void) {
	pthread_t dump_thread;
	//strcpy(dump_spot, "/home/alesaurio/dump/");
	strcpy(dump_spot, "/home/utnso/dump/");

	crear_directorio(dump_spot);

	//descomenten este y comenten el mio
	//--Alesaurio-bot

	strcat(dump_spot, NOMBRE);
	strcat(dump_spot, "/");

	pthread_create(&dump_thread, NULL, dump, (void*) dump_spot);
	pthread_detach(dump_thread);
}

FILE* open_file(char* file_name, char* mode, char* directory) {
	char* path = malloc(strlen(file_name) + strlen(directory) + 1);
	strcpy(path, directory);
	strcat(path, file_name);

	FILE* fd = fopen(path, mode);

	if (fd == NULL) {
		log_error(logger, "Falló la apertura del archivo %s.", file_name);
		free(path);
		return NULL;
	}

	free(path);

	return fd;
}

void write_file(char* file_name, char* text, char* directory) {

	FILE* fd = open_file(file_name, "w", directory);

	int res = fputs(text, fd);

	if (res < 0) {
		log_error(logger, "Falló la escritura en el archivo");
		exit(-1);
	}

	fclose(fd);

}

void cerrar_directorio(char* directorio) {
	int i = 0;

	while (directorio[i] != '\0') {
		i++;
	}

	directorio[i] = '/';
	cerrar_cadena(directorio);
}

void* dump(void* buffer) {
	char* dump_path = (char*) buffer;
	crear_directorio(dump_path);

	while (1) {
		sleep(DUMP);
		log_info(logger, "Iniciando DUMP");
		leer_valores_almacenados();

		pthread_mutex_lock(&sem_entradas);
		entradas_node * nodo_aux = entradas_asignadas.head;

		while (nodo_aux != NULL) {
			char* valor_a_dumpear = leer_valor(nodo_aux->una_entrada.pos_valor,
					nodo_aux->una_entrada.tamanio_valor);

			char* clave_a_dumpear = nodo_aux->una_entrada.clave;

			write_file(clave_a_dumpear, valor_a_dumpear, dump_path);

			free(valor_a_dumpear);

			nodo_aux = nodo_aux->siguiente;
		}

		pthread_mutex_unlock(&sem_entradas);
	}

	return NULL;
}

void setup_montaje(void) {
	log_info(logger, "Creando punto de montaje...");

	crear_directorio(PUNTO_MONTAJE);

	log_info(logger, "Punto de montaje creado exitosamente.");

}

void crear_directorio(char* nombre) {
	struct stat sb;

	if (stat(nombre, &sb) == 0) {
		log_warning(logger, "Ya existe una carpeta %s.", nombre);
		return;
	}

	int status = mkdir(nombre, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if (status < 0) {
		log_error(logger, "Falló la creación del directorio: %s", nombre);
		log_error(logger, strerror(errno));
		exit_gracefully(-1);
	}

	log_info(logger, "Directorio %s creado con éxito.", nombre);
}

algoritmo_reemplazo dame_algoritmo(char* algoritmo_src) {
	algoritmo_reemplazo algoritmo_ret;

	if (strcmp(algoritmo_src, "CIRC") == 0) {
		algoritmo_ret = CIRC;
	}

	else if (strcmp(algoritmo_src, "LRU") == 0) {
		algoritmo_ret = LRU;
	}

	else if (strcmp(algoritmo_src, "BSU") == 0) {
		algoritmo_ret = BSU;
	}

	return algoritmo_ret;
}

void cargar_configuracion(char** argv) {
	t_config* config = config_create(argv[1]);

	char* puerto_coordi = config_get_string_value(config, "PUERTO_COORDINADOR");
	PUERTO_COORDINADOR = transfer(puerto_coordi, strlen(puerto_coordi) + 1);
	log_info(logger, "Puerto Coordinador: %s", PUERTO_COORDINADOR);

	char* ip_coordi = config_get_string_value(config, "IP_COORDINADOR");
	IP_COORDINADOR = transfer(ip_coordi, strlen(ip_coordi) + 1);
	log_info(logger, "IP Coordinador: %s", IP_COORDINADOR);

	char* algoritmo = config_get_string_value(config, "ALGORITMO_DISTRIBUCION");
	ALGORITMO_REEMPLAZO = dame_algoritmo(algoritmo);
	log_info(logger, "Algoritmo: %s", algoritmo);

	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");
	PUNTO_MONTAJE = transfer(punto_montaje, strlen(punto_montaje) + 1);
	cerrar_cadena(PUNTO_MONTAJE);
	log_info(logger, "Punto de montaje: %s", PUNTO_MONTAJE);

	char* nombre = config_get_string_value(config, "NOMBRE");
	NOMBRE = transfer(nombre, strlen(nombre) + 1);
	cerrar_cadena(NOMBRE);
	log_info(logger, "Nombre: %s", NOMBRE);

	DUMP = config_get_int_value(config, "DUMP");
	log_info(logger, "Intervalo de dump: %i", DUMP);

	loggear("Configuración cargada.");
	config_destroy(config);
}

void inicializar(int cantidad_entradas, int tamanio_entrada) {

	//Creo una matriz de memoria para almacenar los valores que vengan
	//Aunque en realidad es una fila pero lo pienso como matriz
	almacenamiento_de_valores = malloc(tamanio_entrada * cantidad_entradas);

	entradas_disponibles = malloc(sizeof(int) * cantidad_entradas);

	//Inicializo todas las es en 0 (osea que estan libres) y el tamanio de todos los valores en 0
	int i;
	for (i = 0; i < cantidad_entradas; i++) {
		entradas_disponibles[i] = 0;
	}

	//nodo_auxiliar = (entradas_node*) malloc(sizeof(entradas_node));

	entradas_asignadas.head = NULL;

	puntero_entrada = 0;

	reloj_interno = 0;
}

orden_del_coordinador recibir_orden_coordinador(int socket_coordinador) {

	orden_del_coordinador orden;
	orden_del_coordinador * buffer_orden = malloc(sizeof(orden_del_coordinador));

	log_info(logger, "Esperando orden del coordinador...");

	int res = recv(socket_coordinador, buffer_orden,
			sizeof(orden_del_coordinador), 0);

	if (res < 0) {
		loggear("Fallo en la recepcion de la orden");
		orden.codigo_operacion = 13;
		orden.tamanio_a_enviar = 0;
		free(buffer_orden);
		return orden;
	}

	log_trace(logger, "cod. op: %d, tamanio: %d", buffer_orden->codigo_operacion, buffer_orden->tamanio_a_enviar);

	orden.codigo_operacion = buffer_orden->codigo_operacion;

	orden.tamanio_a_enviar = buffer_orden->tamanio_a_enviar;

	free(buffer_orden);

	return orden;
}

void set(uint32_t longitud_parametros, int socket_coordinador) {

	parametros_set parametros;

	if (recieve_and_deserialize_set(&(parametros), socket_coordinador) < 0) {
		loggear("Fallo en la recepcion de los parametros");
	}

	//Veo si ya existe la clave (en cuyo caso trabajo directamente sobre el struct entrada que contenga esa clave)

	int posicion_entrada_clave = posicion_de_entrada_con_clave(
			parametros.clave);

	log_debug(logger, "Tamanio valor: %d, Tamanio clave: %d",
			parametros.tamanio_valor, parametros.tamanio_clave);

	if (posicion_entrada_clave >= 0) {
		log_info(logger, "La entrada ya existe, actualizando...");
		actualizar_entrada(parametros, posicion_entrada_clave);
	}

	else {
		log_info(logger, "La entrada no existe, generando nueva entrada...");
		generar_entrada(parametros);
	}

	free(parametros.clave);
	free(parametros.valor);
}

int posicion_de_entrada_con_clave(char* clave) {

	if (entradas_asignadas.head == NULL) {
		log_debug(logger, "No hay entradas en la lista");
		return -1;
	}

	entradas_node * nodo_auxiliar = entradas_asignadas.head;

	while (nodo_auxiliar != NULL) {
		entrada posible_entrada = nodo_auxiliar->una_entrada;

		int comparacion_de_claves = strcmp(clave, posible_entrada.clave);

		if (comparacion_de_claves == 0) {
			return posible_entrada.pos_valor;
		}

		nodo_auxiliar = nodo_auxiliar->siguiente;
	}

	return -1;
}

void actualizar_entrada(parametros_set parametros, int posicion_entrada_clave) {

	entradas_node * puntero_entrada_a_actualizar;
	puntero_entrada_a_actualizar = buscar_entrada_en_posicion(
			posicion_entrada_clave);

	//No considero el barra 0
	uint32_t nuevo_tamanio_valor = parametros.tamanio_valor - 1;

	desactualizar_entradas(
			puntero_entrada_a_actualizar->una_entrada.tamanio_valor,
			posicion_entrada_clave);

	puntero_entrada_a_actualizar->una_entrada.tamanio_valor =
			nuevo_tamanio_valor;

	puntero_entrada_a_actualizar->una_entrada.tiempo_sin_ser_referenciado =
			reloj_interno;

	reloj_interno++;

	int entradas_que_ocupa = obtener_entradas_que_ocupa(nuevo_tamanio_valor);

	actualizar_entradas(puntero_entrada_a_actualizar->una_entrada.pos_valor,
			entradas_que_ocupa);

	almacenar_valor(puntero_entrada_a_actualizar->una_entrada.pos_valor,
			entradas_que_ocupa, parametros.valor);

	confirmar_resultado_de_operacion(111);

	package_int cantidad_entradas_ocupadas;

	cantidad_entradas_ocupadas.packed =
			(uint32_t) obtener_cantidad_de_entradas_ocupadas();

	log_debug(logger, "Enviando cantidad de entradas ocupadas: %i",
			cantidad_entradas_ocupadas.packed);

	enviar_packed(cantidad_entradas_ocupadas, socket_coordinador);

}

void desactualizar_entradas(uint32_t tamanio_valor, int posicion_entrada_clave) {
	int entradas_que_ocupa = obtener_entradas_que_ocupa(tamanio_valor);
	int i;

	for (i = 0; i < entradas_que_ocupa; i++) {
		entradas_disponibles[posicion_entrada_clave + i] = 0;
	}
}

void generar_entrada(parametros_set parametros) {

	//-1 Porque en los parametros viene con el \0
	int entradas_que_ocupa = obtener_entradas_que_ocupa(
			parametros.tamanio_valor - 1);

	int entrada_seleccionada = verificar_disponibilidad_entradas_contiguas(
			entradas_que_ocupa);

	if (entrada_seleccionada >= 0) {
		log_debug(logger, "Valor a generar: %s", parametros.valor);
		almacenar_valor(entrada_seleccionada, entradas_que_ocupa, parametros.valor);
		actualizar_entradas(entrada_seleccionada, entradas_que_ocupa);
		crear_entrada(parametros, entrada_seleccionada);

		confirmar_resultado_de_operacion(111);

		package_int cantidad_entradas_ocupadas;

		cantidad_entradas_ocupadas.packed =
				(uint32_t) obtener_cantidad_de_entradas_ocupadas();

		log_debug(logger, "Enviando cantidad de entradas ocupadas: %i",
				cantidad_entradas_ocupadas.packed);

		enviar_packed(cantidad_entradas_ocupadas, socket_coordinador);

		return;
	}

	if (puedo_almacenar_si_compacto(entradas_que_ocupa)) {
		loggear("Puedo almacenar si compacto...");
		confirmar_resultado_de_operacion(101);
		return;
	}

	loggear("No puedo almacenar el valor, empleando algoritmo de reemplazo...");

	//Aplicaria el algoritmo de reemplazo y volveria a intentar

	eliminar_entrada_segun_algoritmo();

	generar_entrada(parametros);
}

int puedo_almacenar_si_compacto(int cantidad_entradas_solicitadas) {
	int cantidad_entradas_libres = cantidad_entradas
			- obtener_cantidad_de_entradas_ocupadas();

	if (cantidad_entradas_libres >= cantidad_entradas_solicitadas) {
		return 1;
	}

	return 0;
}

void eliminar_entrada_segun_algoritmo() {
	switch (ALGORITMO_REEMPLAZO) {
	case CIRC:
		borrar_entrada(obtener_entrada_segun_CIRC());
		log_info(logger, "Reemplazo CIRC finalizado! Nueva posicion del puntero: %i", puntero_entrada);
		break;
	case LRU:
		borrar_entrada(obtener_entrada_segun_LRU());
		log_info(logger, "Reemplazo LRU finalizado! Nueva posicion del puntero: %i", puntero_entrada);
		break;
	case BSU:
		borrar_entrada(obtener_entrada_segun_BSU());
		log_info(logger, "Reemplazo BSU finalizado! Nueva posicion del puntero: %i", puntero_entrada);
		break;
	default:
		break;
	}
}

entrada obtener_entrada_segun_CIRC() {

	//Obtener entradas atomicas y usar esa lista
	t_entrada_list entradas_atomicas = obtener_entradas_atomicas();

	entradas_node * puntero = entradas_atomicas.head;

	while (puntero->una_entrada.pos_valor != puntero_entrada) {

		puntero = puntero->siguiente;

		//Si no encontre ninguna entrada pruebo en el siguiente, porque a lo mejor justo no hay algun valor
		//donde apunta el puntero.

		if (puntero == NULL) {
			//Termine la lista y no lo encontre, tengo que posicionarme al principio
			puntero = entradas_atomicas.head;
			avanzar_puntero_CIRC();
		}

	}

	//Cualquiera de las dos formas seria valida creeeeeo
	puntero_entrada = puntero->una_entrada.pos_valor;
	avanzar_puntero_CIRC();

	entrada entrada_a_eliminar = asignar_entrada(puntero);

	liberar_entradas_atomicas(entradas_atomicas);

	return entrada_a_eliminar;
}

int el_nuevo_supera_segun_CIRC(entradas_node * puntero_actual,
		entradas_node * puntero_nuevo) {

	int posicion_entrada_actual = puntero_actual->una_entrada.pos_valor;
	int posicion_entrada_nueva = puntero_nuevo->una_entrada.pos_valor;
	int referencia_puntero = puntero_entrada;

	while (1) {

		if (posicion_entrada_actual == referencia_puntero) {
			return 0;
		}

		if (posicion_entrada_nueva == referencia_puntero) {
			return 1;
		}

		referencia_puntero++;
		referencia_puntero = referencia_puntero % cantidad_entradas;

	}
}

void avanzar_puntero_CIRC() {

	puntero_entrada++;

	//Si me pase de la cantidad de entradas reseteo el puntero haciendolo circular
	if (puntero_entrada >= cantidad_entradas) {
		puntero_entrada = 0;
	}

}

entrada obtener_entrada_segun_LRU() {

	//Obtener entradas atomicas y usar esa lista
	t_entrada_list entradas_atomicas = obtener_entradas_atomicas();

	entradas_node * puntero = entradas_atomicas.head;
	entradas_node * puntero_auxiliar = entradas_atomicas.head;

	//El primero de la lista de filtrados
	int tiempo_del_elegido = puntero->una_entrada.tiempo_sin_ser_referenciado;

	while (puntero_auxiliar != NULL) {

		int tiempo_entrada_a_comparar =
				puntero_auxiliar->una_entrada.tiempo_sin_ser_referenciado;

		//Busco el que tenga el mayor tiempo sin ser referenciado, o sea el que tenga el menor tiempo de ultimo acceso
		if (tiempo_del_elegido > tiempo_entrada_a_comparar) {
			puntero = puntero_auxiliar;
			tiempo_del_elegido = tiempo_entrada_a_comparar;
		}

		puntero_auxiliar = puntero_auxiliar->siguiente;

	}

	log_debug(logger, "Tiempo de la entrada seleccionada: %i",
			puntero->una_entrada.tiempo_sin_ser_referenciado);

	entrada entrada_a_eliminar = asignar_entrada(puntero);

	liberar_entradas_atomicas(entradas_atomicas);

	return entrada_a_eliminar;
}

entrada asignar_entrada(entradas_node * puntero) {

	entrada entrada_a_eliminar;

	entrada_a_eliminar.clave = puntero->una_entrada.clave;
	entrada_a_eliminar.pos_valor = puntero->una_entrada.pos_valor;
	entrada_a_eliminar.tamanio_valor = puntero->una_entrada.tamanio_valor;
	entrada_a_eliminar.tiempo_sin_ser_referenciado =
			puntero->una_entrada.tiempo_sin_ser_referenciado;

	return entrada_a_eliminar;

}

void liberar_entradas_atomicas(t_entrada_list entradas_atomicas) {
	entradas_node * puntero = entradas_atomicas.head;
	entradas_node * aux = puntero->siguiente;

	while (puntero != NULL) {
		free(puntero);
		puntero = aux;
		if (aux != NULL) {
			aux = puntero->siguiente;
		}
	}

}

t_entrada_list obtener_entradas_atomicas() {

	t_entrada_list entradas_atomicas;
	entradas_atomicas.head = NULL;

	entradas_node * puntero = entradas_asignadas.head;

	while (puntero != NULL) {

		entrada una_entrada = puntero->una_entrada;

		if (es_entrada_atomica(una_entrada)) {
			agregar_entrada(una_entrada, &entradas_atomicas);
		}

		puntero = puntero->siguiente;

	}

	if(entradas_atomicas.head == NULL){
		log_warning(logger, "Que problema! No hay entradas para borrar. Borro la primera por default");
		agregar_entrada(first(), &entradas_atomicas);
	}

	return entradas_atomicas;
}

bool es_entrada_atomica(entrada una_entrada) {

	int entradas_que_ocupa = obtener_entradas_que_ocupa(
			una_entrada.tamanio_valor);

	return entradas_que_ocupa == 1;
}

entrada obtener_entrada_segun_BSU() {
	//Obtener entradas atomicas y usar esa lista
	t_entrada_list entradas_atomicas = obtener_entradas_atomicas();

	entradas_node * puntero = entradas_atomicas.head;
	entradas_node * puntero_auxiliar = entradas_atomicas.head;

	int tamanio_referencia = puntero->una_entrada.tamanio_valor;
	int tamanio_a_comparar;

	while (puntero_auxiliar != NULL) {

		tamanio_a_comparar = puntero_auxiliar->una_entrada.tamanio_valor;

		//Si empatan tengo que ver segun CIRC
		if (tamanio_referencia == tamanio_a_comparar) {
			if (el_nuevo_supera_segun_CIRC(puntero, puntero_auxiliar)) {
				puntero = puntero_auxiliar;
			}
		}

		//Si ocupa mas espacio lo selecciono de una
		if (tamanio_a_comparar > tamanio_referencia) {
			puntero = puntero_auxiliar;
			tamanio_referencia = tamanio_a_comparar;

		}

		puntero_auxiliar = puntero_auxiliar->siguiente;

	}

	puntero_entrada = puntero->una_entrada.pos_valor;
	avanzar_puntero_CIRC();

	entrada entrada_a_eliminar = asignar_entrada(puntero);

	log_debug(logger, "Tamanio del valor de la entrada seleccionada: %i", entrada_a_eliminar.tamanio_valor);

	liberar_entradas_atomicas(entradas_atomicas);

	return entrada_a_eliminar;
}

void borrar_entrada(entrada entrada_a_eliminar) {
	if (entradas_asignadas.head != NULL) {

		entrada head = first();

		if (entrada_a_eliminar.pos_valor == head.pos_valor) {
			entradas_node* eliminado = entradas_asignadas.head;
			entradas_asignadas.head = entradas_asignadas.head->siguiente;
			liberar_entradas_disponibles(entrada_a_eliminar.pos_valor,
					entrada_a_eliminar.tamanio_valor);
			destruir_nodo_entrada(eliminado);
		}

		else {

			entradas_node* puntero = entradas_asignadas.head;
			entradas_node * aux;

			while (puntero->una_entrada.pos_valor
					!= entrada_a_eliminar.pos_valor) {
				aux = puntero;
				puntero = puntero->siguiente;
			}

			if (puntero->siguiente != NULL) {
				aux->siguiente = puntero->siguiente;
				liberar_entradas_disponibles(entrada_a_eliminar.pos_valor,
						entrada_a_eliminar.tamanio_valor);
				destruir_nodo_entrada(puntero);
			}

			else {
				loggear("Estoy borrando justo la ultima entrada de la lista");
				liberar_entradas_disponibles(entrada_a_eliminar.pos_valor,
						entrada_a_eliminar.tamanio_valor);
				aux->siguiente = NULL;
				destruir_nodo_entrada(puntero);
			}
		}
	}

}

void liberar_entradas_disponibles(int posicion, int tamanio_valor) {

	int entradas_a_liberar = obtener_entradas_que_ocupa(tamanio_valor);

	int i;

	for (i = 0; i < entradas_a_liberar; i++) {
		entradas_disponibles[posicion + i] = 0;
	}
}

entrada first() {
	entrada primer_entrada = entradas_asignadas.head->una_entrada;

	return primer_entrada;
}

int obtener_entradas_que_ocupa(int tamanio_valor) {
	int entradas_que_ocupa = 1 + (tamanio_valor - 1) / tamanio_entrada;
	return entradas_que_ocupa;
}

void crear_entrada(parametros_set parametros, int entrada_seleccionada) {
	entrada nueva_entrada;

	nueva_entrada.clave = malloc(parametros.tamanio_clave);

	//La clave se guarda con \0
	memcpy(nueva_entrada.clave, parametros.clave, parametros.tamanio_clave);

	nueva_entrada.pos_valor = entrada_seleccionada;
	nueva_entrada.tamanio_valor = parametros.tamanio_valor - 1;
	nueva_entrada.tiempo_sin_ser_referenciado = reloj_interno;

	reloj_interno++;

	loggear("Entrada creada, agregando a la lista...");

	agregar_entrada(nueva_entrada, &entradas_asignadas);

}

int recieve_and_deserialize_set(parametros_set *parametros, int socketCliente) {

	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));

	//uint32_t tamanio_clave;
	status = recv(socketCliente, buffer, sizeof(parametros->tamanio_clave), 0);
	memcpy(&(parametros->tamanio_clave), buffer, buffer_size);
	if (!status)
		return -1;

	parametros->clave = malloc(parametros->tamanio_clave + 1);

	status = recv(socketCliente, parametros->clave, parametros->tamanio_clave,
			0);
	if (!status)
		return -1;

	//uint32_t tamanio_valor;
	status = recv(socketCliente, buffer, sizeof(parametros->tamanio_valor), 0);
	memcpy(&(parametros->tamanio_valor), buffer, buffer_size);
	if (!status)
		return -1;

	int tamanio_maximo_en_la_entrada = obtener_entradas_que_ocupa(
			parametros->tamanio_valor) * tamanio_entrada;

	parametros->valor = malloc(tamanio_maximo_en_la_entrada + 1);

	status = recv(socketCliente, parametros->valor, parametros->tamanio_valor,
			0);
	if (!status)
		return -1;

	free(buffer);

	return status;
}

void almacenar_valor(int entrada_seleccionada, int entradas_que_ocupa,
		char * valor) {

	int offset = entrada_seleccionada * tamanio_entrada;

	memcpy(almacenamiento_de_valores + offset, valor,
			tamanio_entrada * entradas_que_ocupa);

	loggear("Valor copiado");

}

int verificar_disponibilidad_entradas_contiguas(int entradas_que_ocupa) {
	int entrada = 0;

	while (entrada <= cantidad_entradas - entradas_que_ocupa) {
		if (entradas_disponibles[entrada] == 0) {

			int puedo_almacenar = 1;
			int i;

			for (i = 1; i < entradas_que_ocupa; i++) {
				if (entradas_disponibles[entrada + i] != 0) {
					puedo_almacenar = 0;
				}
			}

			if (puedo_almacenar) {
				log_trace(logger,
						"Tengo suficientes entradas para almacenar en la entrada: %d",
						entrada);
				return entrada;
			}

		}
		entrada++;
	}

	return -1;
}

void actualizar_entradas(int pos_entrada, int entradas_que_ocupa) {

	entradas_disponibles[pos_entrada] = 1;

	//Para marcar a las que fueron ocupadas por el valor con mas de una entrada
	int siguiente = pos_entrada + 1;
	int limite = pos_entrada + entradas_que_ocupa;

	while (siguiente < limite) {
		entradas_disponibles[siguiente] = 1;
		siguiente++;
	}
}

void compactacion() {

	int primera_entrada_disponible = obtener_primera_entrada_disponible();

	if (primera_entrada_disponible < 0) {
		return;
	}

	int total_de_entradas_ocupadas = obtener_cantidad_de_entradas_ocupadas();

	log_debug(logger, "Entradas ocupadas: %i", total_de_entradas_ocupadas);

	//Como ya se que esta libre le sumo 1
	int i = primera_entrada_disponible + 1;

	while (i < cantidad_entradas) {

		if (entradas_disponibles[i]) {

			log_info(logger, "Entrada encontrada para desplazar: %i", i);

			entradas_node * puntero_entrada_a_desplazar =
					buscar_entrada_en_posicion(i);

			desplazar(puntero_entrada_a_desplazar, primera_entrada_disponible);

			primera_entrada_disponible += obtener_entradas_que_ocupa(
					puntero_entrada_a_desplazar->una_entrada.tamanio_valor);

			// -1 porque despues le hago el i++
			i += obtener_entradas_que_ocupa(
					puntero_entrada_a_desplazar->una_entrada.tamanio_valor) - 1;

		}

		i++;

	}

	actualizar_entradas_disponibles(total_de_entradas_ocupadas);

	confirmar_resultado_de_operacion(114);

}

entradas_node * buscar_entrada_en_posicion(int posicion) {

	entradas_node * nodo_auxiliar = entradas_asignadas.head;

	while (nodo_auxiliar != NULL) {

		if (nodo_auxiliar->una_entrada.pos_valor == posicion) {
			return nodo_auxiliar;
		}

		nodo_auxiliar = nodo_auxiliar->siguiente;
	}

	loggear("Error en encontrar la entrada");

	return nodo_auxiliar;
}

entradas_node* crear_nodo_entrada(entrada una_entrada) {
	entradas_node * nodo = (entradas_node*) malloc(sizeof(entradas_node));
	nodo->una_entrada = una_entrada;
	nodo->siguiente = NULL;
	return nodo;
}

int obtener_cantidad_de_entradas_ocupadas() {

	int contador_entradas_ocupadas = 0;
	int i;

	for (i = 0; i < cantidad_entradas; i++) {
		if (entradas_disponibles[i] == 1) {
			contador_entradas_ocupadas++;
		}
	}

	return contador_entradas_ocupadas;
}

void actualizar_entradas_disponibles(int entradas_ocupadas) {

	int i;
	int j;

	for (i = 0; i < entradas_ocupadas; i++) {
		entradas_disponibles[i] = 1;
	}

	for (j = i; j < cantidad_entradas; j++) {
		entradas_disponibles[j] = 0;
	}
}

void agregar_entrada(entrada una_entrada, t_entrada_list * lista_de_entradas) {
	entradas_node* nodo = crear_nodo_entrada(una_entrada);

	if (lista_de_entradas->head == NULL) {
		lista_de_entradas->head = nodo;
	}

	else {

		entradas_node* puntero = lista_de_entradas->head;

		while (puntero->siguiente != NULL) {

			puntero = puntero->siguiente;

		}

		puntero->siguiente = nodo;

	}

	return;
}

void destruir_nodo_entrada(entradas_node* nodo) {
	free(nodo->una_entrada.clave);
	free(nodo);
}

int obtener_primera_entrada_disponible() {

	int i = 0;
	while (i < cantidad_entradas) {
		if (entradas_disponibles[i] == 0) {
			return i;
		}
		i++;
	}

	loggear("No hay entradas libres, no hay que compactar");

	return -1;
}

void desplazar(entradas_node * puntero_entrada, int nueva_posicion) {

	int posicion_actual = puntero_entrada->una_entrada.pos_valor;

	//Actualizo las estructuras administrativas
	puntero_entrada->una_entrada.pos_valor = nueva_posicion;

	//Actualizo la memoria
	int entradas_que_ocupa = obtener_entradas_que_ocupa(
			puntero_entrada->una_entrada.tamanio_valor);

	log_info(logger,
			"Posicion actual: %i, Nueva posicion: %i, Entradas que ocupa: %i",
			posicion_actual, nueva_posicion, entradas_que_ocupa);

	desplazar_memoria(nueva_posicion, posicion_actual, entradas_que_ocupa);

}

void desplazar_memoria(int posicion_a_desplazarse, int posicion_actual,
		int entradas_del_valor) {

	memcpy(almacenamiento_de_valores + posicion_a_desplazarse * tamanio_entrada,
			almacenamiento_de_valores + posicion_actual * tamanio_entrada,
			entradas_del_valor * tamanio_entrada);

	return;
}

char * leer_clave_valor(entradas_node * puntero) {

	//Le resto el '\0' de la clave de la entrada
	int tamanio_de_la_clave_a_leer = strlen(puntero->una_entrada.clave);

	int tamanio_del_valor_a_leer = puntero->una_entrada.tamanio_valor;

	int posicion = puntero->una_entrada.pos_valor;

	//+ 1 por el ':' + 1 por el '\0' que agrego al final para leer
	int tamanio_total = tamanio_de_la_clave_a_leer + 1
			+ tamanio_del_valor_a_leer;

	auxiliar = malloc(tamanio_total + 1);

	memcpy(auxiliar, puntero->una_entrada.clave, tamanio_de_la_clave_a_leer);

	auxiliar[tamanio_de_la_clave_a_leer] = ':';

	memcpy(auxiliar + tamanio_de_la_clave_a_leer + 1,
			almacenamiento_de_valores + posicion * tamanio_entrada,
			tamanio_del_valor_a_leer);

	//Sobre el final de lo que copie pongo un \0 para que sepa que se termina ahi
	auxiliar[tamanio_total] = '\0';

	return auxiliar;
}

char * leer_valor(int posicion_entrada, int tamanio_valor) {

	auxiliar = malloc(tamanio_valor + 1);

	memcpy(auxiliar,
			almacenamiento_de_valores + posicion_entrada * tamanio_entrada,
			tamanio_valor);

	auxiliar[tamanio_valor] = '\0';

	return auxiliar;
}

void leer_valores_almacenados() {

	loggear("Se almacenaron los siguientes valores: ");

	entradas_node * nodo_auxiliar = entradas_asignadas.head;

	while (nodo_auxiliar != NULL) {

		int entrada_a_leer = nodo_auxiliar->una_entrada.pos_valor;

		char * clave_valor = leer_clave_valor(nodo_auxiliar);

		log_trace(logger, "En la entrada %i se encuentra: \n%s, ",
				entrada_a_leer, clave_valor);

		free(auxiliar);

		nodo_auxiliar = nodo_auxiliar->siguiente;
	}

	log_trace(logger, "Posicion del puntero actual: %i", puntero_entrada);

	int i;

	for (i = 0; i < cantidad_entradas; i++) {
		log_debug(logger, "%i", entradas_disponibles[i]);
	}



}

void confirmar_resultado_de_operacion(int codigo_exito_operacion) {

	if (codigo_exito_operacion == 100) {
		log_info(logger, "Envio de Ping finalizado con exito!");
	} else if (codigo_exito_operacion == 51) {
		log_info(logger, "Clave restaurada con exito!");
	} else if (codigo_exito_operacion == 101) {
		log_info(logger, "Envio de solicitud de compactacion");
	} else if (codigo_exito_operacion == 110) {
		log_info(logger, "Entradas y tamanio inicializadas con exito!");
	} else if (codigo_exito_operacion == 111) {
		log_info(logger, "SET finalizado con exito!");
	} else if (codigo_exito_operacion == 112) {
		log_info(logger, "STORE finalizado con exito!");
	} else if (codigo_exito_operacion == 114) {
		log_info(logger, "Compactacion finalizada!");
	} else if (codigo_exito_operacion == 140) {
		log_info(logger, "Envio de nombre finalizado!");
	} else if (codigo_exito_operacion == 150) {
		log_info(logger, "Instancia lista para volver a trabajar!");
	} else if (codigo_exito_operacion == 151) {
		//log_debug(logger, "Clave recibida con exito!");
	} else if (codigo_exito_operacion == 152) {
		log_warning(logger, "La clave no fue dumpeada :(");
	} else if (codigo_exito_operacion == 666) {
		log_warning(logger, "PIDIENDO ABORTO DEL ESI");
	}

	package_int ping = { .packed = codigo_exito_operacion };

	enviar_packed(ping, socket_coordinador);
}
