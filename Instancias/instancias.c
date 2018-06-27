/*
 * instancias.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "instancias.h"

int main(int argc, char** argv) {

	iniciar();

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			id_instancia, 0);

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
			//case 12: loggear("STORE"); store(orden.tamanio_a_enviar, socket_coordinador); break;
		case 13:
			loggear("Fallo");
			break;
		default:
			loggear("ERROR");
			break;
		}
	}

	//caso_de_prueba_5();

	//leer_valores_almacenados();

	loggear("Cerrando conexion con servidor y terminando.");

	close(socket_coordinador);

	free(almacenamiento_de_valores);

	return EXIT_SUCCESS;
}

void iniciar(void) {
	iniciar_log("Instancias", "A new Instance joins the brawl!");
	loggear("Cargando configuración.");

	cargar_configuracion();
	crear_punto_de_montaje();

	//Aca deberia hacer un recv de la cantidad de entradas y el tamanio por lo que el handshake deberia hacerse antes

	cantidad_entradas = 10;
	tamanio_entrada = 8;

	inicializar(cantidad_entradas, tamanio_entrada);

	log_trace(logger, "Posicion de memoria inicial en main: %d \n",
			*almacenamiento_de_valores);
}

FILE* crear_archivo(char* file_name){
	char* path = strcat(PUNTO_MONTAJE, file_name);

	FILE* fd = fopen(path, "w");

	if(fd == NULL){
		log_error(logger, "Falló la creación del archivo %s.", file_name);
		exit(-1);
	}

	log_info(logger, "Archivo creado exiotsamente");

	return fd;
}

void write_file(FILE* fd, char* valor){
	int res = fputs(valor, fd);

	if(res < 0){
		log_error(logger, "Falló la escritura en el archivo");
		exit(-1);
	}

	loggear("Archivo escrito correctamente");
}

void crear_punto_de_montaje(void){
	log_info(logger, "Creando punto de montaje...");

	struct stat sb;

	sleep(1);

	if(stat(PUNTO_MONTAJE, &sb) == 0){
		log_warning(logger, "Ya existe una carpeta %s.", PUNTO_MONTAJE);
		return;
	}

	int status = mkdir(PUNTO_MONTAJE, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if(status < 0){
		log_error(logger, "Falló la creación de punto de montaje.");
		exit(-1);
	}

	log_info(logger, "Punto de montaje creado exitósamente.");
}

algoritmo_reemplazo dame_algoritmo(char* algoritmo_src){
	algoritmo_reemplazo algoritmo_ret;

	if(strcmp(algoritmo_src, "CIRC") == 0){
		algoritmo_ret = CIRC;
	}

	else if(strcmp(algoritmo_src, "LRU") == 0){
		algoritmo_ret = LRU;
	}

	else if(strcmp(algoritmo_src, "BSU") == 0){
		algoritmo_ret = BSU;
	}

	return algoritmo_ret;
}

void cargar_configuracion(void){
	t_config* config = config_create("instancia.config");

	PUERTO_COORDINADOR = config_get_string_value(config, "PUERTO_COORDINADOR");
	log_info(logger, "Puerto Coordinador: %s", PUERTO_COORDINADOR);

	IP_COORDINADOR = config_get_string_value(config, "IP_COORDINADOR");
	log_info(logger, "IP Coordinador: %s", IP_COORDINADOR);

	char* algoritmo = config_get_string_value(config, "ALGORITMO_DISTRIBUCION");
	ALGORITMO_REEMPLAZO = dame_algoritmo(algoritmo);
	log_info(logger, "Algoritmo: %s", algoritmo);

	PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");
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

	//Inicializo todas las posiciones en 0 (osea que estan libres) y el tama�o de todos los valores en 0
	int i;
	for (i = 0; i < cantidad_entradas; i++) {
		entradas_disponibles[i] = 0;
	}

	list_create(entradas);

}

orden_del_coordinador recibir_orden_coordinador(int socket_coordinador) {

	orden_del_coordinador orden;
	orden_del_coordinador * buffer_orden = malloc(sizeof(orden_del_coordinador));

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

	log_trace(logger, "cod. op: %d", buffer_orden->codigo_operacion);

	log_trace(logger, "tamanio: %d", buffer_orden->tamanio_a_enviar);

	sleep(2);

	orden.codigo_operacion = buffer_orden->codigo_operacion;

	orden.tamanio_a_enviar = buffer_orden->tamanio_a_enviar;

	free(buffer_orden);

	return orden;
}

void set(uint32_t longitud_parametros, int socket_coordinador) {

	parametros_set parametros;
	if(recieve_and_deserialize_set(&(parametros), socket_coordinador) < 0){
		loggear("Fallo en la recepcion de los parametros");
	}

	log_trace(logger, "Clave: %c, %c, %c %c %c", parametros.clave[0], parametros.clave[1], parametros.clave[2], parametros.clave[3], parametros.clave[4]);

	log_trace(logger, "Valor: %c, %c, %c %c %c %c %c", parametros.valor[0], parametros.valor[1], parametros.valor[2], parametros.valor[3], parametros.valor[4], parametros.valor[5], parametros.valor[6]);

	//Veo si ya existe la clave (en cuyo caso trabajo directamente sobre el struct entrada que contenga esa clave)

	//Esta verificacion la hago cuando entienda como usar list_find sin usar bools

	//list_find(entradas, existe_la_clave);

	//Si no existe tengo que crearla, por lo que me fijo si puedo almacenar el valor (veo si entra)

	int tamanio_valor = strlen(parametros.valor);

	int entradas_que_ocupa = obtener_entradas_que_ocupa(tamanio_valor);

	int entrada_seleccionada = verificar_disponibilidad_entradas_contiguas(entradas_que_ocupa);

	//Si puedo almacenar creo un struct entrada con los valores que me dieron y dandole una posicion por la cual acceder

	log_trace(logger, "tamanio_valor: %i entradas que ocupa: %i, entrada_seleccionada: %i", tamanio_valor, entradas_que_ocupa, entrada_seleccionada);

	if(entrada_seleccionada >= 0){
		almacenar_valor(entrada_seleccionada, entradas_que_ocupa, parametros.valor);
		actualizar_entradas(entrada_seleccionada, entradas_que_ocupa);
		crear_entrada(parametros, entrada_seleccionada, tamanio_valor);
		return;
	}

	loggear("No puedo almacenar el valor");
}

int obtener_entradas_que_ocupa(int tamanio_valor){
	return 1 + (tamanio_valor - 1) / tamanio_entrada;
}

void crear_entrada(parametros_set parametros, int entrada_seleccionada, int tamanio_valor){
	entrada nueva_entrada;
	nueva_entrada.clave = parametros.clave;
	nueva_entrada.pos_valor = entrada_seleccionada;
	nueva_entrada.tamanio_valor = tamanio_valor;

	loggear("Entrada creada, agregando a la lista...");

	int tamanio_de_entrada = strlen(parametros.clave) + sizeof(entrada_seleccionada) + sizeof(tamanio_valor);

	char * buffer_entrada = malloc(tamanio_de_entrada);

	memcpy(buffer_entrada, &(nueva_entrada), tamanio_de_entrada);

	log_trace(logger, "En la entrada %d esta guardado el valor %s", nueva_entrada.pos_valor, leer_valor(nueva_entrada));

	//Cada vez que leo libero el buffer auxiliar para leer
	free(auxiliar);

	//list_add(entradas, buffer_entrada);

	//Una vez que lo agregue a la lista tengo que liberarlo?

	free(buffer_entrada);
}

int recieve_and_deserialize_set(parametros_set *parametros, int socketCliente){

	int status;
	int buffer_size;
	char *buffer = malloc(buffer_size = sizeof(uint32_t));

	uint32_t tamanio_clave;
	status = recv(socketCliente, buffer, sizeof(parametros-> tamanio_clave), 0);
	memcpy(&(tamanio_clave), buffer, buffer_size);
	if (!status) return -1;

	log_trace(logger, "Tamanio clave recibido: %d", tamanio_clave);

	parametros -> clave = malloc(tamanio_clave);

	status = recv(socketCliente, parametros -> clave, tamanio_clave, 0);
	if (!status) return -1;

	uint32_t tamanio_valor;
	status = recv(socketCliente, buffer, sizeof(parametros -> tamanio_valor), 0);
	memcpy(&(tamanio_valor), buffer, buffer_size);
	if (!status) return -1;

	parametros -> valor = malloc(tamanio_valor);

	log_trace(logger, "Tamanio valor recibido %d", tamanio_valor);

	status = recv(socketCliente, parametros -> valor, tamanio_valor, 0);
	if (!status) return -1;

	free(buffer);

	int i;
	for (i = 0; i < cantidad_entradas; i++) {
		log_trace(logger, "%i", entradas_disponibles[i]);
	}

	return status;
}

void almacenar_valor(int entrada_seleccionada, int entradas_que_ocupa, char * valor) {

	int offset = entrada_seleccionada * tamanio_entrada;

	loggear("Posicion de memoria seleccionada");
	log_trace(logger, "Ubicacion memoria: %d",
	almacenamiento_de_valores + offset);

	loggear("Copio el valor dentro de esa posicion de memoria");
	memcpy(almacenamiento_de_valores + offset, valor, tamanio_entrada * entradas_que_ocupa);

	loggear("Valor copiado");

}

int verificar_disponibilidad_entradas_contiguas(int entradas_que_ocupa) {
	int entrada = 0;

	while (entrada <= cantidad_entradas - entradas_que_ocupa) {
		if(entradas_disponibles[entrada] == 0){

			int puedo_almacenar = 1;
			int i;

			for(i = 1; i < entradas_que_ocupa; i++){
				if (entradas_disponibles[entrada + i] != 0) {
					puedo_almacenar = 0;
				}
			}

			if(puedo_almacenar){
				log_trace(logger, "Tengo suficientes entradas para almacenar en la entrada: %d", entrada);
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

char * leer_valor(entrada unaEntrada) {
	int tamanio_del_valor_a_leer = unaEntrada.tamanio_valor;

	int posicion = unaEntrada.pos_valor;

	// + 1 por el '\0' que agrego al final para leer
	auxiliar = malloc(tamanio_del_valor_a_leer + 1);

	log_trace(logger, "Estoy por mostrar la entrada: %d que ocupa: %d",
			posicion, tamanio_del_valor_a_leer);

	memcpy(auxiliar, almacenamiento_de_valores + posicion * tamanio_entrada,
			tamanio_del_valor_a_leer);

	//Sobre el final de lo que copie pongo un \0 para que sepa que se termina ahi
	auxiliar[tamanio_del_valor_a_leer] = '\0';

	return auxiliar;
}

void compactacion(){

	int primera_entrada_disponible = obtener_primera_entrada_disponible();

	if(primera_entrada_disponible < 0){
		return;
	}

	loggear("Estoy por compactar");

	//Como ya se que esta libre le sumo 1
	int i = primera_entrada_disponible + 1;

	while(entradas_disponibles[i] < cantidad_entradas){

		if(entradas_disponibles[i]){
			//entrada entrada_a_desplazar = list_find(entradas, entrada_que_esta_en(i));
			entrada entrada_a_desplazar;

			desplazar(entrada_a_desplazar, primera_entrada_disponible);

			primera_entrada_disponible += entrada_a_desplazar.tamanio_valor;

			// -1 porque despues le hago el i++
			i += entrada_a_desplazar.tamanio_valor - 1;

		}

		i++;

	}

	//int total_de_espacio_ocupado = list_sum(entradas);

}

int obtener_primera_entrada_disponible(){

	int i = 0;
	while(i < cantidad_entradas){
		if(entradas_disponibles[i] == 0){
			return i;
		}
		i++;
	}

	loggear("No hay entradas libres, no hay que compactar");

	return -1;
}

void desplazar(entrada una_entrada, int nueva_posicion){

	int posicion_actual = una_entrada.pos_valor;

	//Actualizo las estructuras administrativas
	una_entrada.pos_valor = nueva_posicion;

	//ACTUALIZAR EL VECTOR DE ENTRADAS, aunque se podria hacer al final de compactar si se el tamanio de todo
	//Y seria [1,1,1,1,1,1,1,0,0,0,0]
	//entradas_disponibles[nueva_posicion] = ??

	//Actualizo la memoria
	int entradas_que_ocupa = obtener_entradas_que_ocupa(una_entrada.tamanio_valor);

	desplazar_memoria(posicion_actual, nueva_posicion, entradas_que_ocupa);

}

void desplazar_memoria(int posicion_a_desplazarse, int posicion_actual, int entradas_del_valor){

	//Magia con memcpy y malloc
}

/*
void leer_valores_almacenados() {

	loggear("Se almacenaron los siguientes valores: ");
	int k;
	for (k = 0; k < cantidad_entradas; k++) {
		log_trace(logger,
				"En la entrada %d se encuentra almacenado el valor: %s", k,
				leer_valor(k));
		free(auxiliar);
	}
}

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
