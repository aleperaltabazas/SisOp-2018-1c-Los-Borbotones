/*
 * instancias.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "instancias.h"

int main(int argc, char** argv) {

	iniciar(argv);

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			id_instancia, 0);

	recibir_orden_inicial(socket_coordinador);

	inicializar(cantidad_entradas, tamanio_entrada);

	//Para desconectarla habria que cambiar este valor simplemente
	disponibilidad_de_conexion = 1;

	while (disponibilidad_de_conexion) {
		orden_del_coordinador orden;
		orden = recibir_orden_coordinador(socket_coordinador);
		switch (orden.codigo_operacion) {
		case 11:
			loggear("SET");
			set(orden.tamanio_a_enviar, socket_coordinador);
			break;
		case 12:
			loggear("STORE");
			store(orden.tamanio_a_enviar, socket_coordinador);
			break;
		case 13:
			loggear("Fallo");
			break;
		case 14:
			loggear("Compactar");
			compactacion();
			break;
		case 15:
			loggear("Mostrando lo almacenado...");
			leer_valores_almacenados();
			break;
		default:
			loggear("ERROR");
			break;
		}
	}

	close(socket_coordinador);

	//Deberia vaciar la lista...

	destruir_nodo_entrada(nodo_auxiliar);

	free(almacenamiento_de_valores);

	free(entradas_disponibles);

	return EXIT_SUCCESS;
}

void recibir_orden_inicial(int socket_coordinador) {

	orden_del_coordinador orden_inicial;

	orden_inicial = recibir_orden_coordinador(socket_coordinador);

	tamanio_entrada = orden_inicial.codigo_operacion;

	cantidad_entradas = orden_inicial.tamanio_a_enviar;

	log_trace(logger, "Cantidad entradas: %i, Tamanio de entrada: %i",
			cantidad_entradas, tamanio_entrada);
}

void store(uint32_t tamanio_a_enviar, int socket_coordinador) {

	package_int clave_size = recibir_packed(socket_coordinador);

	char* clave = recibir_cadena(socket_coordinador, clave_size.packed);

	log_trace(logger, "CLAVE RECIBIDA: %s", clave);

	int posicion_de_entrada = posicion_de_entrada_con_clave(clave,
			clave_size.packed);

	entradas_node * entrada_seleccionada = buscar_entrada_en_posicion(
			posicion_de_entrada);

	entrada_seleccionada->una_entrada.tiempo_sin_ser_referenciado =
			reloj_interno;

	reloj_interno++;

	entrada entrada_con_la_clave = entrada_seleccionada->una_entrada;

	int tamanio_valor = entrada_con_la_clave.tamanio_valor;

	char * valor = malloc(tamanio_valor);

	int offset = posicion_de_entrada * tamanio_entrada;

	memcpy(valor, almacenamiento_de_valores + offset, tamanio_valor);

	log_trace(logger, "%s", valor);

	write_file(clave, valor);

	destroy_string(clave);
	destroy_string(valor);
}

void iniciar(char** argv) {
	iniciar_log("Instancias", "A new Instance joins the brawl!");
	loggear("Cargando configuración.");

	cargar_configuracion(argv);
	setup_montaje();
}

FILE* open_file(char* file_name) {
	char* path = malloc(strlen(file_name) + strlen(PUNTO_MONTAJE) + 1);
	strcpy(path, PUNTO_MONTAJE);
	strcat(path, file_name);

	FILE* fd = fopen(path, "w");

	if (fd == NULL) {
		log_error(logger, "Falló la creación del archivo %s.", file_name);
		exit(-1);
	}

	log_info(logger, "Archivo creado exitosamente");

	log_trace(logger, "PUNTO MONTAJE: %s PATH: %s", PUNTO_MONTAJE, path);

	return fd;
}

void write_file(char* file_name, char* text) {
	FILE* fd = open_file(file_name);

	int res = fputs(text, fd);

	if (res < 0) {
		log_error(logger, "Falló la escritura en el archivo");
		exit(-1);
	}

	log_trace(logger, "%s", text);

	loggear("Archivo escrito correctamente");
	fclose(fd);
}

void setup_montaje(void) {
	log_info(logger, "Creando punto de montaje...");

	struct stat sb;

	sleep(1);

	if (stat(PUNTO_MONTAJE, &sb) == 0) {
		log_warning(logger, "Ya existe una carpeta %s.", PUNTO_MONTAJE);
		return;
	}

	int status = mkdir(PUNTO_MONTAJE, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if (status < 0) {
		log_error(logger, "Falló la creación de punto de montaje.");
		exit(-1);
	}

	log_info(logger, "Punto de montaje creado exitósamente.");

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

	PUERTO_COORDINADOR = config_get_string_value(config, "PUERTO_COORDINADOR");
	log_info(logger, "Puerto Coordinador: %s", PUERTO_COORDINADOR);

	IP_COORDINADOR = config_get_string_value(config, "IP_COORDINADOR");
	log_info(logger, "IP Coordinador: %s", IP_COORDINADOR);

	char* algoritmo = config_get_string_value(config, "ALGORITMO_DISTRIBUCION");
	ALGORITMO_REEMPLAZO = dame_algoritmo(algoritmo);
	log_info(logger, "Algoritmo: %s", algoritmo);

	PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");
	strcat(PUNTO_MONTAJE, "\0");
	log_info(logger, "Punto de montaje: %s", PUNTO_MONTAJE);

	NOMBRE = config_get_string_value(config, "NOMBRE");
	log_info(logger, "Nombre: %s", NOMBRE);

	DUMP = config_get_int_value(config, "DUMP");
	log_info(logger, "Intervalo de dump: %i", DUMP);

	loggear("Configuración cargada.");
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

	nodo_auxiliar = (entradas_node*) malloc(sizeof(entradas_node));

	entradas_asignadas.head = NULL;

	puntero_entrada = 0;

	reloj_interno = 0;
}

orden_del_coordinador recibir_orden_coordinador(int socket_coordinador) {

	orden_del_coordinador orden;
	orden_del_coordinador * buffer_orden = malloc(
			sizeof(orden_del_coordinador));

	loggear("Esperando orden del coordinador...");

	if (recv(socket_coordinador, buffer_orden, sizeof(orden_del_coordinador), 0)
			< 0) {
		loggear("Fallo en la recepcion de la orden");
		orden.codigo_operacion = 13;
		orden.tamanio_a_enviar = 0;
		free(buffer_orden);
		return orden;
	}

	loggear("Orden recibida!");

	log_trace(logger, "cod. op: %d, tamanio: %d",
			buffer_orden->codigo_operacion, buffer_orden->tamanio_a_enviar);

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

	int posicion_entrada_clave = posicion_de_entrada_con_clave(parametros.clave,
			parametros.tamanio_clave);

	log_trace(logger, "tamanio_valor %d, tamanio_clave %d",
			parametros.tamanio_valor, parametros.tamanio_clave);

	char * mostrar_clave = malloc(parametros.tamanio_clave + 1);

	memcpy(mostrar_clave, parametros.clave, parametros.tamanio_clave);

	mostrar_clave[parametros.tamanio_clave] = '\0';

	log_trace(logger, "CLAVE RECIBIDA: %s", mostrar_clave);

	free(mostrar_clave);

	if (posicion_entrada_clave >= 0) {
		loggear("La entrada ya existe, actualizando...");
		actualizar_entrada(parametros, posicion_entrada_clave);
	}

	else {
		loggear("La entrada no existe, generando nueva entrada...");
		generar_entrada(parametros);
	}

	free(parametros.clave);
	free(parametros.valor);
}

int posicion_de_entrada_con_clave(char* clave, int tamanio_clave) {

	if (entradas_asignadas.head == NULL) {
		loggear("No hay entradas en la lista");
		return -1;
	}

	nodo_auxiliar = entradas_asignadas.head;

	while (nodo_auxiliar != NULL) {
		entrada posible_entrada = nodo_auxiliar->una_entrada;

		int comparacion_de_claves = comparar_claves(clave, tamanio_clave,
				posible_entrada.clave);
		//comparar_claves(clave, tamanio_clave, posible_entrada.clave);

		if (comparacion_de_claves == 0) {
			return posible_entrada.pos_valor;
		}

		nodo_auxiliar = nodo_auxiliar->siguiente;
	}

	return -1;
}

int comparar_claves(char * clave, int tamanio_clave, char * clave_a_comparar) {
	char * auxiliar_clave = malloc(tamanio_clave + 1);
	memcpy(auxiliar_clave, clave, tamanio_clave);
	auxiliar_clave[tamanio_clave] = '\0';
	int comparacion = strcmp(auxiliar_clave, clave_a_comparar);
	free(auxiliar_clave);
	return comparacion;
}

void actualizar_entrada(parametros_set parametros, int posicion_entrada_clave) {

	entradas_node * puntero_entrada_a_actualizar;
	puntero_entrada_a_actualizar = buscar_entrada_en_posicion(
			posicion_entrada_clave);

	uint32_t nuevo_tamanio_valor = parametros.tamanio_valor;

	puntero_entrada_a_actualizar->una_entrada.tamanio_valor =
			nuevo_tamanio_valor;

	puntero_entrada_a_actualizar->una_entrada.tiempo_sin_ser_referenciado =
			reloj_interno;

	reloj_interno++;

	int entradas_que_ocupa = obtener_entradas_que_ocupa(nuevo_tamanio_valor);

	//Tendria que desactualizar como estaba antes pero Adriano dijo que no habia problema con eso

	actualizar_entradas(puntero_entrada_a_actualizar->una_entrada.pos_valor,
			entradas_que_ocupa);

	almacenar_valor(puntero_entrada_a_actualizar->una_entrada.pos_valor,
			entradas_que_ocupa, parametros.valor);
}

void generar_entrada(parametros_set parametros) {

	char * auxiliar = malloc(parametros.tamanio_valor + 1);

	memcpy(auxiliar, parametros.valor, parametros.tamanio_valor);

	auxiliar[parametros.tamanio_valor] = '\0';

	int tamanio_valor = strlen(auxiliar);

	free(auxiliar);

	int entradas_que_ocupa = obtener_entradas_que_ocupa(tamanio_valor);

	int entrada_seleccionada = verificar_disponibilidad_entradas_contiguas(
			entradas_que_ocupa);

	if (entrada_seleccionada >= 0) {
		almacenar_valor(entrada_seleccionada, entradas_que_ocupa,
				parametros.valor);
		actualizar_entradas(entrada_seleccionada, entradas_que_ocupa);
		crear_entrada(parametros, entrada_seleccionada, tamanio_valor,
				parametros.tamanio_clave);
		return;
	}

	loggear("No puedo almacenar el valor, empleando algoritmo de reemplazo...");

	//Aplicaria el algoritmo de reemplazo y volveria a intentar

	eliminar_entrada_segun_algoritmo();

	generar_entrada(parametros);
}

void eliminar_entrada_segun_algoritmo() {
	switch (ALGORITMO_REEMPLAZO) {
	case CIRC:
		borrar_entrada(obtener_entrada_segun_CIRC());
		log_trace(logger, "CIRC, Puntero: %i", puntero_entrada);
		break;
	case LRU:
		borrar_entrada(obtener_entrada_segun_LRU());
		break;
	case BSU:
		borrar_entrada(obtener_entrada_segun_BSU());
		break;
	default:
		break;
	}
}

entrada obtener_entrada_segun_CIRC() {

	entradas_node* puntero = entradas_asignadas.head;

	while (puntero->una_entrada.pos_valor != puntero_entrada) {

		puntero = puntero->siguiente;

		//Si no encontre ninguna entrada pruebo en el siguiente, porque a lo mejor justo no hay algun valor
		//donde apunta el puntero.
		if (puntero == NULL) {

			//Termine la lista y no lo encontre, tengo que posicionarme al principio
			puntero = entradas_asignadas.head;
			avanzar_puntero_CIRC();
		}

	}

	//Cualquiera de las dos formas seria valida creoooooo
	puntero_entrada = puntero->una_entrada.pos_valor;
	avanzar_puntero_CIRC();

	return puntero->una_entrada;
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
	entradas_node* puntero = entradas_asignadas.head;
	//entradas_node* puntero_auxiliar = malloc(sizeof(entradas_node));
	entradas_node *puntero_auxiliar = entradas_asignadas.head;

	while (puntero_auxiliar != NULL) {

		//Busco el que tenga el mayor tiempo sin ser referenciado, o sea el que tenga el menor tiempo de ultimo acceso
		if (puntero->una_entrada.tiempo_sin_ser_referenciado
				> puntero_auxiliar->una_entrada.tiempo_sin_ser_referenciado) {
			puntero = puntero_auxiliar;
		}

		puntero_auxiliar = puntero_auxiliar->siguiente;

	}

	log_trace(logger, "TIEMPO DE LA ENTRADA SELECCIONADA: %i",
			puntero->una_entrada.tiempo_sin_ser_referenciado);

	return puntero->una_entrada;
}

entrada obtener_entrada_segun_BSU() {
	entradas_node* puntero = entradas_asignadas.head;
	entradas_node *puntero_auxiliar = entradas_asignadas.head->siguiente;

	int tamanio_referencia = puntero->una_entrada.tamanio_valor;
	int tamanio_a_comparar;
	int entradas_que_ocupa_actual = obtener_entradas_que_ocupa(
			tamanio_referencia);
	int entradas_que_ocupa_nuevo;

	while (puntero_auxiliar != NULL) {

		tamanio_a_comparar = puntero_auxiliar->una_entrada.tamanio_valor;
		entradas_que_ocupa_nuevo = obtener_entradas_que_ocupa(
				tamanio_a_comparar);

		//Si empatan tengo que ver segun CIRC
		if (entradas_que_ocupa_nuevo == entradas_que_ocupa_actual) {
			if (el_nuevo_supera_segun_CIRC(puntero, puntero_auxiliar)) {
				puntero = puntero_auxiliar;
				entradas_que_ocupa_actual = entradas_que_ocupa_nuevo;
			}
		}

		//Si ocupa mas entradas lo selecciono de una
		if (entradas_que_ocupa_nuevo > entradas_que_ocupa_actual) {
			puntero = puntero_auxiliar;
			entradas_que_ocupa_actual = entradas_que_ocupa_nuevo;

		}

		puntero_auxiliar = puntero_auxiliar->siguiente;

	}

	puntero_entrada = puntero->una_entrada.pos_valor;
	avanzar_puntero_CIRC();

	return puntero->una_entrada;
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

			log_trace(logger, "Estoy por borrar la entrada: %d",
					puntero->una_entrada.pos_valor);

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
	return 1 + (tamanio_valor - 1) / tamanio_entrada;
}

void crear_entrada(parametros_set parametros, int entrada_seleccionada,
		int tamanio_valor, int tamanio_clave) {
	entrada nueva_entrada;
	nueva_entrada.clave = malloc(tamanio_clave + 1);
	memcpy(nueva_entrada.clave, parametros.clave, tamanio_clave);
	nueva_entrada.clave[tamanio_clave] = '\0';
	nueva_entrada.pos_valor = entrada_seleccionada;
	nueva_entrada.tamanio_valor = tamanio_valor;
	nueva_entrada.tiempo_sin_ser_referenciado = reloj_interno;

	reloj_interno++;

	loggear("Entrada creada, agregando a la lista...");

	agregar_entrada(nueva_entrada);

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

	parametros->clave = malloc(parametros->tamanio_clave);

	status = recv(socketCliente, parametros->clave, parametros->tamanio_clave,
			0);
	if (!status)
		return -1;

	//uint32_t tamanio_valor;
	status = recv(socketCliente, buffer, sizeof(parametros->tamanio_valor), 0);
	memcpy(&(parametros->tamanio_valor), buffer, buffer_size);
	if (!status)
		return -1;

	int entradas_que_ocupa = obtener_entradas_que_ocupa(
			parametros->tamanio_valor);

	parametros->valor = malloc(entradas_que_ocupa * tamanio_entrada);

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

	loggear("Estoy por compactar");

	int total_de_entradas_ocupadas = obtener_cantidad_de_entradas_ocupadas();

	log_trace(logger, "entradas ocupadas: %i", total_de_entradas_ocupadas);

	//Como ya se que esta libre le sumo 1
	int i = primera_entrada_disponible + 1;

	while (i < cantidad_entradas) {

		if (entradas_disponibles[i]) {

			log_trace(logger, "Entrada encontrada: %i", i);

			entradas_node * puntero_entrada_a_desplazar =
					buscar_entrada_en_posicion(i);

			desplazar(puntero_entrada_a_desplazar, primera_entrada_disponible);

			primera_entrada_disponible += obtener_entradas_que_ocupa(
					puntero_entrada_a_desplazar->una_entrada.tamanio_valor);

			// -1 porque despues le hago el i++
			i += obtener_entradas_que_ocupa(
					puntero_entrada_a_desplazar->una_entrada.tamanio_valor) - 1;

			log_trace(logger, "POS ENTRADA NUEVA: %i",
					puntero_entrada_a_desplazar->una_entrada.pos_valor);

		}

		i++;

	}

	actualizar_entradas_disponibles(total_de_entradas_ocupadas);

}

entradas_node * buscar_entrada_en_posicion(int posicion) {

	/*
	 if(entradas_asignadas.head == NULL){
	 //Si se verifico que esta nunca deberia entrar por aca
	 entradas_node * entrada_error;
	 entrada_error -> una_entrada.clave = "ERRORMSG";
	 entrada_error -> una_entrada.pos_valor = -1;
	 entrada_error -> una_entrada.tamanio_valor = -1;
	 return entrada_error;
	 }
	 */

	nodo_auxiliar = entradas_asignadas.head;

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

	//Tendria que ver si se hizo el ultimo i++
	log_trace(logger, "El valor de i es: %d", i);

	for (j = i; j < cantidad_entradas; j++) {
		entradas_disponibles[j] = 0;
	}
}

void agregar_entrada(entrada una_entrada) {
	entradas_node* nodo = crear_nodo_entrada(una_entrada);

	if (entradas_asignadas.head == NULL) {
		entradas_asignadas.head = nodo;
	}

	else {

		entradas_node* puntero = entradas_asignadas.head;

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

	log_trace(logger,
			"Posicion Actual: %i, Nueva posicion: %i, Entradas que ocupa: %i",
			posicion_actual, nueva_posicion, entradas_que_ocupa);

	desplazar_memoria(nueva_posicion, posicion_actual, entradas_que_ocupa);

}

void desplazar_memoria(int posicion_a_desplazarse, int posicion_actual,
		int entradas_del_valor) {

	loggear("Estoy por mover la memoria");

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
			+ tamanio_del_valor_a_leer + 1;

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

void leer_valores_almacenados() {

	loggear("Se almacenaron los siguientes valores: ");

	nodo_auxiliar = entradas_asignadas.head;

	while (nodo_auxiliar != NULL) {

		int entrada_a_leer = nodo_auxiliar->una_entrada.pos_valor;

		log_trace(logger, "En la entrada %i se encuentra: \n%s, ",
				entrada_a_leer, leer_clave_valor(nodo_auxiliar));
		free(auxiliar);

		nodo_auxiliar = nodo_auxiliar->siguiente;
	}

	log_trace(logger, "PUNTERO: %i", puntero_entrada);

	/*
	 int i;
	 for (i = 0; i < cantidad_entradas; i++) {
	 log_trace(logger, "%i", entradas_disponibles[i]);
	 }
	 */

}

/*
 //Lleno con el valor ejemplo
 void caso_de_prueba_1() {
 valor = "EjemploX";
 while (disponibilidad_de_conexion) {
 resultado_almacenamiento = almacenar_valor(entrada_seleccionada, entradas_que_ocupa, valor);
 if (resultado_almacenamiento == EXIT_FAILURE) {
 disponibilidad_de_conexion = 0;
 }
 }
 }

 //Lleno con distintos tipos de valores que ocupan una entrada
 void caso_de_prueba_2() {
 int t = 0;
 while (disponibilidad_de_conexion) {
 //Recibimos el valor
 if (t == 0) {
 valor = "Ejemplo1";
 } else if (t == 1) {
 valor = "Ejemplo2";
 } else if (t == 8) {
 valor = "Algo";
 } else {
 valor = "Default";
 }
 resultado_almacenamiento = almacenar_valor();
 if (resultado_almacenamiento == EXIT_FAILURE) {
 disponibilidad_de_conexion = 0;
 }
 t++;
 }
 }

 //Valor que ocupa dos entradas
 void caso_de_prueba_3() {
 valor = "EjemploEjemplo";
 while (disponibilidad_de_conexion) {
 resultado_almacenamiento = almacenar_valor();
 if (resultado_almacenamiento == EXIT_FAILURE) {
 disponibilidad_de_conexion = 0;
 }
 }
 }

 //Un valor que ocupa todas las entradas
 void caso_de_prueba_4() {
 valor =
 "80caracteeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeees";
 while (disponibilidad_de_conexion) {
 resultado_almacenamiento = almacenar_valor();
 if (resultado_almacenamiento == EXIT_FAILURE) {
 disponibilidad_de_conexion = 0;
 }
 }
 }

 //Lleno con distintos tipos de valores que ocupan una o mas entradas
 void caso_de_prueba_5() {
 int t = 0;
 while (disponibilidad_de_conexion) {
 //Recibimos el valor
 if (t == 0) {
 valor = "20caractereeeeeeeees";
 } else if (t == 1) {
 valor = "15caractereeees";
 } else if (t == 2) {
 valor = "40caractereeeeeeeeeeeeeeeeeeeeeeeeeeeees";
 } else {
 valor = "1";
 }
 resultado_almacenamiento = almacenar_valor();
 if (resultado_almacenamiento == EXIT_FAILURE) {
 disponibilidad_de_conexion = 0;
 }
 t++;
 }
 }
 */
